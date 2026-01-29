#ifndef MQ_ADC_H
#define MQ_ADC_H

#include <stdint.h>

void mq_adc_init(void);
uint16_t mq_adc_read_raw(uint8_t channel);

#endif
