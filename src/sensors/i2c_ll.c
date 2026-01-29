#include "i2c_ll.h"

#include "app_config.h"

#include "hardware/gpio.h"
#include "hardware/regs/i2c.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/padsbank0.h"
#include "hardware/regs/resets.h"
#include "hardware/structs/io_bank0.h"
#include "hardware/structs/padsbank0.h"
#include "hardware/structs/resets.h"
#include "hardware/structs/sio.h"

#ifndef I2C_IC_CON_SPEED_LSB
#define I2C_IC_CON_SPEED_LSB 1u
#endif
#ifndef I2C_IC_CON_SPEED_VALUE_FAST
#define I2C_IC_CON_SPEED_VALUE_FAST 2u
#endif

static void i2c_gpio_init(uint32_t gpio) {
    io_bank0_hw->io[gpio].ctrl =
        (GPIO_FUNC_I2C << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);
    padsbank0_hw->io[gpio] =
        PADS_BANK0_GPIO0_IE_BITS |
        PADS_BANK0_GPIO0_PUE_BITS;
}

void i2c_ll_init(i2c_hw_t *i2c, uint32_t sda_gpio, uint32_t scl_gpio, uint32_t baud_hz) {
    // Reset I2C block
    if (i2c == i2c0_hw) {
        resets_hw->reset |= RESETS_RESET_I2C0_BITS;
        resets_hw->reset &= ~RESETS_RESET_I2C0_BITS;
        while ((resets_hw->reset_done & RESETS_RESET_I2C0_BITS) == 0u) {}
    } else {
        resets_hw->reset |= RESETS_RESET_I2C1_BITS;
        resets_hw->reset &= ~RESETS_RESET_I2C1_BITS;
        while ((resets_hw->reset_done & RESETS_RESET_I2C1_BITS) == 0u) {}
    }

    i2c_gpio_init(sda_gpio);
    i2c_gpio_init(scl_gpio);

    i2c->enable = 0u;

    // Fast mode
    i2c->ic_con = I2C_IC_CON_MASTER_MODE_BITS |
                  (I2C_IC_CON_SPEED_VALUE_FAST << I2C_IC_CON_SPEED_LSB) |
                  I2C_IC_CON_IC_SLAVE_DISABLE_BITS;

    // Simple SCL timing (approx)
    uint32_t clk = SYS_CLK_HZ;
    uint32_t hcnt = (clk / (baud_hz * 2u));
    uint32_t lcnt = hcnt;
    if (hcnt < 8u) hcnt = 8u;
    if (lcnt < 8u) lcnt = 8u;

    i2c->ic_fs_scl_hcnt = hcnt;
    i2c->ic_fs_scl_lcnt = lcnt;
    i2c->ic_rx_tl = 0u;
    i2c->ic_tx_tl = 0u;

    i2c->enable = I2C_IC_ENABLE_ENABLE_BITS;
}

static bool i2c_wait_tx_empty(i2c_hw_t *i2c) {
    for (uint32_t i = 0; i < 500000u; ++i) {
        if (i2c->ic_status & I2C_IC_STATUS_TFE_BITS) return true;
    }
    return false;
}

static bool i2c_wait_rx_ready(i2c_hw_t *i2c) {
    for (uint32_t i = 0; i < 500000u; ++i) {
        if (i2c->ic_status & I2C_IC_STATUS_RFNE_BITS) return true;
    }
    return false;
}

bool i2c_ll_write(i2c_hw_t *i2c, uint8_t addr, const uint8_t *data, size_t len) {
    i2c->ic_tar = addr;
    for (size_t i = 0; i < len; ++i) {
        i2c->ic_data_cmd = data[i];
    }
    return i2c_wait_tx_empty(i2c);
}

bool i2c_ll_read(i2c_hw_t *i2c, uint8_t addr, uint8_t *data, size_t len) {
    i2c->ic_tar = addr;
    for (size_t i = 0; i < len; ++i) {
        i2c->ic_data_cmd = I2C_IC_DATA_CMD_CMD_BITS;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!i2c_wait_rx_ready(i2c)) return false;
        data[i] = (uint8_t)i2c->ic_data_cmd;
    }
    return true;
}

bool i2c_ll_write_read(i2c_hw_t *i2c, uint8_t addr,
                       const uint8_t *wdata, size_t wlen,
                       uint8_t *rdata, size_t rlen) {
    if (!i2c_ll_write(i2c, addr, wdata, wlen)) return false;
    return i2c_ll_read(i2c, addr, rdata, rlen);
}
