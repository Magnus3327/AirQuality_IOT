#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define HEATER_PWM_PIN 15
#define MQ7_ADC_PIN 27
#define MQ7_ADC_INPUT 1

// Costanti per il calcolo PPM (da datasheet MQ7 per CO)
const float RL = 10.0;      // Resistenza di carico sul modulo (tipicamente 10kOhm)
float R0 = 4.0;             // Valore di default, verrà aggiornato dalla calibrazione
const float m = -1.53;      // Coefficiente angolare della curva logaritmica
const float b = 1.72;       // Intercetta della curva

// Funzione di campionamento massivo per stabilizzare senza condensatore
float get_voltage_stable() {
    uint32_t sum = 0;
    const int samples = 5000; // 5000 letture per mediare il PWM
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
    }
    // Conversione: ADC -> Voltage al pin -> Voltage al sensore (partitore 1.5x)
    return ((float)sum / samples) * (3.3f / 4095.0f) * 1.5f;
}

void set_heater(float voltage) {
    uint16_t level = (uint16_t)((voltage / 5.0f) * 65535);
    pwm_set_gpio_level(HEATER_PWM_PIN, level);
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(MQ7_ADC_PIN);
    adc_select_input(MQ7_ADC_INPUT);

    gpio_set_function(HEATER_PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(HEATER_PWM_PIN);
    pwm_set_wrap(slice_num, 65535);
    pwm_set_enabled(slice_num, true);

    printf("\n--- MQ7 CALIBRATION START (Inizia in Aria Pulita) ---\n");

    while (true) {
        // 1. FASE HIGH (Pulizia - 60s)
        set_heater(5.0f);
        for (int i = 60; i > 0; i--) {
            printf("Pulizia riscaldatore: %d s\r", i);
            sleep_ms(1000);
        }

        // 2. FASE LOW (Lettura e Calibrazione - 90s)
        set_heater(1.4f);
        printf("\nFase di lettura... attendere stabilizzazione\n");
        
        for (int i = 90; i > 0; i--) {
            float v_out = get_voltage_stable();
            
            // Calcolo Rs (Resistenza sensore)
            // Rs = ((Vcc / Vout) - 1) * RL
            // Usiamo 5.0V come Vcc teorica
            float rs = ((5.0f / v_out) - 1.0f) * RL;

            if (i > 10) {
                printf("Stabilizzazione: %d s | V_out: %.2fV\r", i, v_out);
            } else {
                // Calibrazione automatica negli ultimi secondi (assumendo aria pulita)
                // In aria pulita Rs/R0 è circa 1.0, quindi R0 = Rs
                if (i == 10) R0 = rs; 
                
                // Calcolo PPM: ratio = Rs/R0 -> ppm = 10^((log10(ratio) - b) / m)
                float ratio = rs / R0;
                float ppm = pow(10, (log10(ratio) + b) / fabs(m)); 

                printf("[!] PPM CO: %.2f | Rs/R0: %.2f | V: %.2fV\n", ppm, ratio, v_out);
            }
            sleep_ms(1000);
        }
    }
}