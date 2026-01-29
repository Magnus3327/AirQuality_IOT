// include/aq_filter.h
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t sum;
    uint16_t buf[16];  // MQ_FILTER_N max 16 in config; keep simple
    uint8_t  idx;
    uint8_t  count;
} movavg_t;

void movavg_init(movavg_t *f);
uint16_t movavg_push(movavg_t *f, uint16_t x);

typedef enum {
    AQ_OK = 0,
    AQ_WARNING,
    AQ_DANGER
} aq_state_t;

aq_state_t eval_with_hyst(uint16_t v, aq_state_t prev,
                          uint16_t th_warn, uint16_t th_dang, uint16_t hyst);

