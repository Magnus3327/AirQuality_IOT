#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

int main() {
    stdio_init_all();
    
    // Inizializzazione ADC
    adc_init();
    adc_gpio_init(26); // GP26 è ADC0
    adc_select_input(0);

    // Fattore di conversione: 
    // 1. ADC a 12-bit -> 3.3V / 4095
    // 2. Partitore 2/3 -> Moltiplichiamo per 1.5 per tornare ai 5V teorici
    const float conversion_factor = 3.3f / (1 << 12);
    const float divider_ratio = 1.5f; 

    while (true) {
        // Leggi il valore grezzo
        uint16_t raw = adc_read();
        
        // Calcola la tensione letta sul pin (max 3.3V)
        float voltage_at_pin = raw * conversion_factor;
        
        // Calcola la tensione originale all'uscita del sensore (max 5V)
        float sensor_voltage = voltage_at_pin * divider_ratio;

        printf("Raw: %d | Pin Voltage: %.2f V | Sensor Output: %.2f V\n", 
                raw, voltage_at_pin, sensor_voltage);

        sleep_ms(1000);
    }

    return 0;
}