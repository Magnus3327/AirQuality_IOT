#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "oled.h"
#include "aht20.h"
#include "mq135.h"
#include "mq7.h"

/* ================= CONFIGURAZIONE ================= */
#define I2C_PORT         i2c0
#define ADC_135_CH       0   // GP26
#define ADC_7_CH         1   // GP27
#define MQ7_PWM_PIN      15

/* --- FUNZIONE MEDIA --- */
// Legge il canale ADC specificato N volte e restituisce la tensione media
float get_voltage_averaged(uint8_t channel, int samples) {
    adc_select_input(channel);
    uint32_t sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
        sleep_us(50); // Piccolo delay per stabilità tra campioni
    }
    float avg_raw = (float)sum / samples;
    float v_adc = (avg_raw * 3.3f) / 4095.0f;
    return v_adc * 1.5f; // Compensazione partitore 1k/2k
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    /* --- INIT HARDWARE --- */
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    adc_init();
    oled_init();
    aht20_init();
    mq135_init(26);             
    mq7_init(27, MQ7_PWM_PIN);  

    /* --- WARM-UP --- */
    oled_clear();
    oled_write_str("AVVIO SISTEMA", 0, 0);
    for (int i = 5; i > 0; i--) {
        char b[16]; sprintf(b, "Warm-up: %ds", i);
        oled_write_str(b, 2, 0);
        sleep_ms(1000);
    }

    /* --- CALIBRAZIONE --- */
    float temp, hum;
    aht20_read(&temp, &hum);
    float v_init_135 = get_voltage_averaged(ADC_135_CH, 100); // Media su 100 campioni
    float r0_135 = mq135_calibrate_r0(v_init_135, temp, hum);
    float r0_7 = 0.0f;

    uint32_t last_mq7_switch = to_ms_since_boot(get_absolute_time());
    mq7_state_t current_mq7_state = MQ7_STATE_HIGH;
    
    oled_clear();

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        // 1. Lettura AHT20
        aht20_read(&temp, &hum);

        // 2. Lettura MQ-135 con MEDIA (molto più stabile!)
        float v_135 = get_voltage_averaged(ADC_135_CH, 64);
        float ratio_135 = mq135_get_rs(v_135) / r0_135;

        // 3. Gestione MQ-7
        float v_7 = 0.0f;
        if (current_mq7_state == MQ7_STATE_HIGH) {
            mq7_set_heater(MQ7_STATE_HIGH);
            if (now - last_mq7_switch >= 60000) {
                current_mq7_state = MQ7_STATE_LOW;
                last_mq7_switch = now;
            }
        } else {
            mq7_set_heater(MQ7_STATE_LOW);
            // Media anche per MQ-7
            v_7 = get_voltage_averaged(ADC_7_CH, 64);
            
            if (now - last_mq7_switch >= 90000) {
                if (r0_7 == 0) r0_7 = mq7_calibrate_r0(v_7, temp, hum);
                current_mq7_state = MQ7_STATE_HIGH;
                last_mq7_switch = now;
            }
        }

        /* ===== DISPLAY & SERIAL ===== */
        char buf[32];
        sprintf(buf, "T:%.1fC H:%.0f%%   ", temp, hum);
        oled_write_str(buf, 0, 0);
        sprintf(buf, "MQ135: %.3fV  ", v_135);
        oled_write_str(buf, 2, 0);

        if (current_mq7_state == MQ7_STATE_HIGH) {
            oled_write_str("MQ7: CLEANING...", 4, 0);
        } else {
            sprintf(buf, "MQ7: MEAS %.3fV", v_7);
            oled_write_str(buf, 4, 0);
        }

        // Stampa DEBUG pulita
        printf("DEBUG|T:%.1f|H:%.1f|V135:%.3f|V7:%.3f|S:%d\n", 
                temp, hum, v_135, v_7, current_mq7_state);

        sleep_ms(2000); 
    }
}