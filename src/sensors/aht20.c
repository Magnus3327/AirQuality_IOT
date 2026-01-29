#include "aht20.h"

#include "i2c_ll.h"

#include "hardware/structs/i2c.h"
#include "pico/stdlib.h"

#define AHT20_CMD_INIT      0xBE
#define AHT20_CMD_MEASURE   0xAC
#define AHT20_CMD_SOFTRESET 0xBA

static i2c_hw_t *const aht20_i2c = i2c0_hw;

void aht20_init(void) {
    i2c_ll_init(aht20_i2c, I2C0_SDA_GPIO, I2C0_SCL_GPIO, I2C_BAUD_HZ);

    uint8_t cmd = AHT20_CMD_SOFTRESET;
    i2c_ll_write(aht20_i2c, AHT20_I2C_ADDR, &cmd, 1);
    sleep_ms(20);

    uint8_t init_cmd[3] = {AHT20_CMD_INIT, 0x08, 0x00};
    i2c_ll_write(aht20_i2c, AHT20_I2C_ADDR, init_cmd, 3);
    sleep_ms(20);
}

bool aht20_read(env_data_t *out) {
    uint8_t cmd[3] = {AHT20_CMD_MEASURE, 0x33, 0x00};
    if (!i2c_ll_write(aht20_i2c, AHT20_I2C_ADDR, cmd, 3)) return false;
    sleep_ms(80);

    uint8_t buf[6] = {0};
    if (!i2c_ll_read(aht20_i2c, AHT20_I2C_ADDR, buf, 6)) return false;

    uint32_t raw_h = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | ((buf[3] >> 4) & 0x0F);
    uint32_t raw_t = (((uint32_t)buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5];

    out->rh_pct = (raw_h * 100.0f) / 1048576.0f;
    out->temp_c = ((raw_t * 200.0f) / 1048576.0f) - 50.0f;
    return true;
}
