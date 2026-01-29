#include "aq_filter.h"

void movavg_init(movavg_t *f) {
    f->sum = 0;
    f->idx = 0;
    f->filled = 0;
    for (uint8_t i = 0; i < MOVAVG_LEN; ++i) {
        f->buf[i] = 0;
    }
}

uint16_t movavg_push(movavg_t *f, uint16_t sample) {
    f->sum -= f->buf[f->idx];
    f->buf[f->idx] = sample;
    f->sum += sample;
    f->idx = (uint8_t)((f->idx + 1u) % MOVAVG_LEN);
    if (f->filled < MOVAVG_LEN) {
        f->filled++;
    }
    return (uint16_t)(f->sum / (f->filled ? f->filled : 1u));
}

aq_state_t eval_with_hyst(uint16_t value, aq_state_t prev,
                          uint16_t warn_th, uint16_t dang_th, uint16_t hyst) {
    switch (prev) {
    case AQ_DANG:
        if (value + hyst < dang_th) {
            return (value + hyst < warn_th) ? AQ_OK : AQ_WARN;
        }
        return AQ_DANG;
    case AQ_WARN:
        if (value + hyst < warn_th) return AQ_OK;
        if (value > dang_th + hyst) return AQ_DANG;
        return AQ_WARN;
    case AQ_OK:
    default:
        if (value > dang_th) return AQ_DANG;
        if (value > warn_th) return AQ_WARN;
        return AQ_OK;
    }
}
