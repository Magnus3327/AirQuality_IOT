#include "calibration_store.h"

#include <string.h>

#include "hardware/flash.h"
#include "hardware/regs/addressmap.h"
#include "hardware/sync.h"

#define CAL_STORE_MAGIC   0xA1C0F00Du
#define CAL_STORE_VERSION 1u

typedef struct {
    uint32_t magic;
    uint32_t version;
    float mq7_ro;
    float mq135_ro;
    uint32_t crc;
} cal_store_t;

static uint32_t cal_crc(const cal_store_t *s) {
    const uint32_t *w = (const uint32_t *)s;
    uint32_t crc = 0;
    for (size_t i = 0; i < (sizeof(cal_store_t) / sizeof(uint32_t)) - 1; ++i) {
        crc ^= w[i];
    }
    return crc;
}

static const uint32_t flash_target_offset =
    (uint32_t)(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);

static const uint8_t *flash_target_ptr(void) {
    return (const uint8_t *)(XIP_BASE + flash_target_offset);
}

bool calibration_store_load(float *mq7_ro, float *mq135_ro) {
    const cal_store_t *s = (const cal_store_t *)flash_target_ptr();
    if (s->magic != CAL_STORE_MAGIC) return false;
    if (s->version != CAL_STORE_VERSION) return false;
    if (s->crc != cal_crc(s)) return false;
    if (mq7_ro) *mq7_ro = s->mq7_ro;
    if (mq135_ro) *mq135_ro = s->mq135_ro;
    return true;
}

bool calibration_store_save(float mq7_ro, float mq135_ro) {
    cal_store_t s;
    memset(&s, 0, sizeof(s));
    s.magic = CAL_STORE_MAGIC;
    s.version = CAL_STORE_VERSION;
    s.mq7_ro = mq7_ro;
    s.mq135_ro = mq135_ro;
    s.crc = cal_crc(&s);

    uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0xFF, sizeof(page));
    memcpy(page, &s, sizeof(s));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(flash_target_offset, FLASH_SECTOR_SIZE);
    flash_range_program(flash_target_offset, page, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    return true;
}
