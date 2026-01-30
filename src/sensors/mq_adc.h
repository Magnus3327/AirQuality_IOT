#ifndef MQ_ADC_H
#define MQ_ADC_H

#include <stdint.h>

void mq_adc_init(void);
uint16_t mq_adc_read_raw(uint8_t channel);

float mq_calc_rs_ohms(uint16_t adc_raw, float rl_ohms, float vref_v, float vcc_v);
float mq_calc_ratio(float rs_ohms, float ro_ohms);
float mq_calc_ppm(float ratio, float a, float b);

#endif
