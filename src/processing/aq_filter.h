#ifndef AQ_FILTER_H
#define AQ_FILTER_H

#include <stdint.h>

#include "app_config.h"

typedef struct {
    uint16_t buf[MOVAVG_LEN];
    uint32_t sum;
    uint8_t idx;
    uint8_t filled;
} movavg_t;

void movavg_init(movavg_t *f);
uint16_t movavg_push(movavg_t *f, uint16_t sample);

aq_state_t eval_with_hyst(uint16_t value, aq_state_t prev,
                          uint16_t warn_th, uint16_t dang_th, uint16_t hyst);

#endif
