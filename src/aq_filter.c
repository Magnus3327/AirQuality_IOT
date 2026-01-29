// src/aq_filter.c
#include "aq_filter.h"

void movavg_init(movavg_t *f) {
    f->sum = 0;
    f->idx = 0;
    f->count = 0;
    for (int i = 0; i < 16; i++) f->buf[i] = 0;
}

uint16_t movavg_push(movavg_t *f, uint16_t x) {
    if (f->count < 16) {
        f->buf[f->idx] = x;
        f->sum += x;
        f->count++;
        f->idx = (f->idx + 1) & 0x0F;
        return (uint16_t)(f->sum / f->count);
    }

    // Full window
    uint16_t old = f->buf[f->idx];
    f->buf[f->idx] = x;
    f->sum = f->sum - old + x;
    f->idx = (f->idx + 1) & 0x0F;
    return (uint16_t)(f->sum / 16u);
}

aq_state_t eval_with_hyst(uint16_t v, aq_state_t prev,
                          uint16_t th_warn, uint16_t th_dang, uint16_t hyst) {
    // Simple hysteresis state machine
    switch (prev) {
        case AQ_OK:
            if (v >= th_dang) return AQ_DANGER;
            if (v >= th_warn) return AQ_WARNING;
            return AQ_OK;

        case AQ_WARNING:
            if (v >= th_dang) return AQ_DANGER;
            if (v + hyst < th_warn) return AQ_OK;
            return AQ_WARNING;

        case AQ_DANGER:
            if (v + hyst < th_dang) return AQ_WARNING;
            return AQ_DANGER;

        default:
            return AQ_OK;
    }
}

