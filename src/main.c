#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#include "oled.h"
#include "aht20.h"
#include "mq135.h"
#include "mq7.h"
#include "mqtt.h"

#define WIFI_SSID "MEE"
#define WIFI_PASS "M27E23E19"

#define I2C_PORT         i2c0
#define ADC_MQ135_PIN    26
#define ADC_MQ7_PIN      27
#define MQ7_PWM_PIN      15

void log_status(const char *msg, int line) {
    printf("%s\n", msg);
    oled_write_str((char*)msg, line, 0);
}

int main() {
    stdio_init_all();
    sleep_ms(2000); 

    // 1. INIZIALIZZAZIONE HARDWARE BASE
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C); 
    gpio_set_function(5, GPIO_FUNC_I2C); 
    adc_init();
    oled_init();
    oled_clear();

    log_status("=== SYSTEM START ===", 0);

    // 2. CONNESSIONE WI-FI OBBLIGATORIA
    if (cyw43_arch_init()) {
        log_status("WIFI INIT ERROR", 1);
        while(true) tight_loop_contents();
    }
    cyw43_arch_enable_sta_mode();
    
    log_status("WIFI: CONNECTING...", 1);
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 15000) != 0) {
        log_status("WIFI RETRYING...", 2);
        sleep_ms(2000);
    }
    
    struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
    char ip_msg[20];
    sprintf(ip_msg, "IP: %s", ip4addr_ntoa(netif_ip4_addr(n)));
    log_status(ip_msg, 2);

    // 3. CONNESSIONE MQTT OBBLIGATORIA
    log_status("MQTT: CONNECTING...", 3);
    mqtt_manager_init();
    
    // Attendiamo finché non è connesso al broker Mosquitto
    while (!mqtt_is_connected_to_broker()) {
        // La callback mqtt_connection_cb aggiornerà lo stato
        cyw43_arch_poll(); // Necessario per processare i pacchetti LwIP
        sleep_ms(500);
    }
    log_status("MQTT: CONNECTED", 4);
    sleep_ms(2000);
    oled_clear();

    // 4. INIZIALIZZAZIONE E WARM-UP SENSORI
    log_status("SENSORS WARM-UP", 0);
    aht20_init();
    mq135_init(ADC_MQ135_PIN);
    mq7_init(ADC_MQ7_PIN, MQ7_PWM_PIN);

    // Ciclo di warm-up (es. 10 secondi per stabilizzare l'elettronica)
    for (int i = 10; i > 0; i--) {
        char countdown[20];
        sprintf(countdown, "STABILIZING: %ds", i);
        oled_write_str(countdown, 2, 0);
        
        // Fase MQ-7 High per pulizia veloce durante warm-up
        mq7_set_heater(MQ7_STATE_HIGH); 
        sleep_ms(1000);
    }
    oled_clear();

    // 5. CALIBRAZIONE R0 (Con sensori caldi)
    log_status("CALIBRATING R0...", 0);
    float temp_init, hum_init;
    aht20_read(&temp_init, &hum_init);
    
    float v135_cal = mq135_get_voltage();
    float r0_135 = mq135_calibrate_r0(v135_cal, temp_init, hum_init);
    float r0_7 = 0.0f; 
    
    log_status("CALIBRATION OK", 1);
    sleep_ms(2000);
    oled_clear();

    uint32_t last_mq7_switch = to_ms_since_boot(get_absolute_time());
    mq7_state_t current_mq7_state = MQ7_STATE_HIGH;

    // --- LOOP PRINCIPALE ---
    while (true) {
        // Cruciale per mantenere attiva la connessione MQTT/Wi-Fi
        cyw43_arch_poll(); 

        uint32_t now = to_ms_since_boot(get_absolute_time());
        float temp, hum;
        aht20_read(&temp, &hum);

        // --- GESTIONE MQ-135 ---
        float v_135 = mq135_get_voltage();
        float nh3_ppm = mq135_get_ppm(v_135, r0_135, temp, hum);

        // --- GESTIONE MQ-7 (Ciclo Dual Stage) ---
        float co_ppm = 0.0f;
        if (current_mq7_state == MQ7_STATE_HIGH) {
            mq7_set_heater(MQ7_STATE_HIGH);
            if (now - last_mq7_switch >= 60000) {
                current_mq7_state = MQ7_STATE_LOW;
                last_mq7_switch = now;
            }
        } else {
            mq7_set_heater(MQ7_STATE_LOW);
            float v_7 = mq7_get_voltage();
            if (r0_7 == 0) r0_7 = mq7_calibrate_r0(v_7, temp, hum); 
            co_ppm = mq7_get_ppm(v_7, r0_7, temp, hum);

            if (now - last_mq7_switch >= 90000) {
                current_mq7_state = MQ7_STATE_HIGH;
                last_mq7_switch = now;
            }
        }

        // --- OLED & SERIAL ---
        char line_buf[32];
        sprintf(line_buf, "T:%.1fC H:%.0f%%", temp, hum);
        oled_write_str(line_buf, 0, 0);
        sprintf(line_buf, "NH3: %.2f PPM", nh3_ppm);
        oled_write_str(line_buf, 2, 0);
        
        if (current_mq7_state == MQ7_STATE_LOW) {
            sprintf(line_buf, "CO:  %.2f PPM", co_ppm);
        } else {
            sprintf(line_buf, "CO:  HEATING...");
        }
        oled_write_str(line_buf, 4, 0);

        // --- MQTT PUBLISH ---
        if (mqtt_is_connected_to_broker()) {
            mqtt_publish_data(nh3_ppm, co_ppm, temp, hum, (int)current_mq7_state);
        } else {
            // Se cade la connessione, prova a reinizializzare senza bloccare il loop
            mqtt_manager_init(); 
        }

        sleep_ms(2000);
    }

    return 0;
}