#ifndef I2C_LL_H
#define I2C_LL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hardware/structs/i2c.h"

void i2c_ll_init(i2c_hw_t *i2c, uint32_t sda_gpio, uint32_t scl_gpio, uint32_t baud_hz);
bool i2c_ll_write(i2c_hw_t *i2c, uint8_t addr, const uint8_t *data, size_t len);
bool i2c_ll_read(i2c_hw_t *i2c, uint8_t addr, uint8_t *data, size_t len);
bool i2c_ll_write_read(i2c_hw_t *i2c, uint8_t addr,
                       const uint8_t *wdata, size_t wlen,
                       uint8_t *rdata, size_t rlen);

#endif
