#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#include "Display/oled.h"
#include "Sensors/AHT20/aht20.h"
#include "Sensors/MQ135/mq135.h"
#include "Sensors/MQ7/mq7.h"
#include "Networking/mqtt.h"

/** Networking Credentials */
#define WIFI_SSID "MEE"
#define WIFI_PASS "M27E23E19"

/** Hardware Mapping */
#define I2C_PORT         i2c0
#define ADC_MQ135_PIN    26
#define ADC_MQ7_PIN      27
#define MQ7_PWM_PIN      15

/** Timing Constants (ms) */
#define MQ7_CLEAN_TIME   60000  /** 60s at 5.0V */
#define MQ7_READ_TIME    90000  /** 90s at 1.5V */
#define LOOP_DELAY       2000   /** Main loop refresh rate */

/** Global State */
static float last_co_ppm = 0.0f;

/**
 * @brief Helper to log system status to Serial and OLED.
 */
void log_status(const char *msg, int line) {
    printf("[SYSTEM] %s\n", msg);
    oled_write_str(msg, line, 0);
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Wait for serial monitor attach

    /** 1. BASIC HARDWARE INITIALIZATION */
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C); 
    gpio_set_function(5, GPIO_FUNC_I2C); 
    adc_init();
    oled_init();
    oled_clear();

    log_status("=== SYSTEM START ===", 0);

    /** 2. PICO W NETWORK INITIALIZATION */
    if (cyw43_arch_init()) {
        log_status("CYW43 INIT ERROR", 1);
        while(true) tight_loop_contents(); // Critical failure
    }
    
    cyw43_arch_enable_sta_mode();
    log_status("WIFI: CONNECTING...", 1);
    
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 20000) != 0) {
        log_status("WIFI: CONN FAILED", 1);
        log_status("HALTING SYSTEM", 2);
        while(true) tight_loop_contents(); // Stop if no network
    }
    
    struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
    char ip_msg[32];
    sprintf(ip_msg, "IP:%s", ip4addr_ntoa(netif_ip4_addr(n)));
    log_status(ip_msg, 2);

    /** 3. MQTT INITIALIZATION */
    log_status("MQTT: CONNECTING...", 3);
    mqtt_manager_init();
    
    // Wait for MQTT connection before starting sensor logic
    uint32_t mqtt_timeout = 0;
    while (!mqtt_is_connected_to_broker() && mqtt_timeout < 100) {
        cyw43_arch_poll();
        sleep_ms(100);
        mqtt_timeout++;
    }

    if (!mqtt_is_connected_to_broker()) {
        log_status("MQTT: CONN FAILED", 3);
        log_status("HALTING SYSTEM", 4);
        while(true) tight_loop_contents(); // Stop if no MQTT
    }
    
    log_status("NETWORKING READY", 4);
    sleep_ms(2000);
    oled_clear();

    /** 4. SENSORS INITIALIZATION & STABILIZATION */
    log_status("SENSORS WARM-UP", 0);
    aht20_init();
    mq135_init(ADC_MQ135_PIN);
    mq7_init(ADC_MQ7_PIN, MQ7_PWM_PIN);

    // Initial MQ-7 heating cycle
    mq7_set_heater(MQ7_STATE_HIGH);
    
    float temp_init, hum_init;
    aht20_read(&temp_init, &hum_init);
    
    // R0 Calibration for MQ-135 (Ammonia/VOC)
    float r0_135 = mq135_calibrate_r0(mq135_get_voltage(), temp_init, hum_init);
    float r0_7 = 0.0f; // To be calibrated during the first LOW cycle

    log_status("CALIBRATION OK", 1);
    sleep_ms(2000);
    oled_clear();

    uint32_t last_state_change = to_ms_since_boot(get_absolute_time());
    mq7_state_t mq7_current_state = MQ7_STATE_HIGH;

    /** 5. MAIN LOOP */
    while (true) {
        // Essential for background networking on Pico W
        cyw43_arch_poll(); 

        uint32_t now = to_ms_since_boot(get_absolute_time());
        float current_temp, current_hum;
        aht20_read(&current_temp, &current_hum);

        // --- MQ-135 Measurement ---
        float nh3_ppm = mq135_get_ppm(mq135_get_voltage(), r0_135, current_temp, current_hum);

        // --- MQ-7 State Machine & Measurement ---
        float current_co_ppm = 0.0f;

        if (mq7_current_state == MQ7_STATE_HIGH) {
            // FASE HIGH (5V - Cleaning)
            mq7_set_heater(MQ7_STATE_HIGH);
            current_co_ppm = last_co_ppm; // Send last known value during heating

            if (now - last_state_change >= MQ7_CLEAN_TIME) {
                mq7_current_state = MQ7_STATE_LOW;
                last_state_change = now;
                printf("[MQ7] Switching to MEASUREMENT phase (1.5V)\n");
            }
        } else {
            // FASE LOW (1.5V - Reading)
            mq7_set_heater(MQ7_STATE_LOW);
            
            float v7 = mq7_get_voltage();
            if (r0_7 == 0.0f) {
                r0_7 = mq7_calibrate_r0(v7, current_temp, current_hum);
            }
            
            current_co_ppm = mq7_get_ppm(v7, r0_7, current_temp, current_hum);
            last_co_ppm = current_co_ppm; // Update last valid reading

            if (now - last_state_change >= MQ7_READ_TIME) {
                mq7_current_state = MQ7_STATE_HIGH;
                last_state_change = now;
                printf("[MQ7] Switching to CLEANING phase (5.0V)\n");
            }
        }

        /** 6. OLED & DATA OUTPUT */
        char display_buf[32];
        
        // Line 0: Environment
        sprintf(display_buf, "T:%.1fC H:%.0f%%", current_temp, current_hum);
        oled_write_str(display_buf, 0, 0);

        // Line 2: Ammonia
        sprintf(display_buf, "NH3: %.2f PPM", nh3_ppm);
        oled_write_str(display_buf, 2, 0);

        // Line 4: Carbon Monoxide status
        if (mq7_current_state == MQ7_STATE_HIGH) {
            sprintf(display_buf, "CO:  HEATING...");
        } else {
            sprintf(display_buf, "CO:  %.2f PPM", current_co_ppm);
        }
        oled_write_str(display_buf, 4, 0);

        /** 7. MQTT PUBLICATION */
        if (mqtt_is_connected_to_broker()) {
            mqtt_publish_data(nh3_ppm, current_co_ppm, current_temp, current_hum, (int)mq7_current_state);
        } else {
            // Attempt to reconnect if lost
            mqtt_manager_init();
        }

        // Debug Serial Output
        printf("DATA | NH3:%.2f | CO:%.2f | T:%.1f | H:%.1f | MQ7_S:%d\n", 
               nh3_ppm, current_co_ppm, current_temp, current_hum, (int)mq7_current_state);

        sleep_ms(LOOP_DELAY);
    }

    return 0;
}