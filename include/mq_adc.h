// include/mq_adc.h
#pragma once
#include <stdint.h>

void mq_adc_init(void);
uint16_t mq_adc_read_raw(uint8_t adc_channel);

