// src/mq_adc.c
#include "mq_adc.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

void mq_adc_init(void) {
    adc_init();
    // GPIO26..28 are ADC0..2; init happens with adc_gpio_init
    adc_gpio_init(26); // ADC0 GP26
    adc_gpio_init(27); // ADC1 GP27
}

uint16_t mq_adc_read_raw(uint8_t adc_channel) {
    adc_select_input(adc_channel);       // 0->GP26, 1->GP27
    // Optional: small settle time if mux switching annoys you
    sleep_us(5);
    return adc_read();                   // 12-bit (0..4095)
}

