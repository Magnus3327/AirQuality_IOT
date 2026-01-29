// src/i2c_sensors.c
#include "i2c_sensors.h"
#include "app_config.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

#define I2C_PORT i2c0

void i2c_sensors_init(void) {
    i2c_init(I2C_PORT, 100 * 1000); // 100 kHz
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
}

void i2c_scan_bus(void) {
    printf("I2C scan:\n");
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        uint8_t dummy = 0;
        int ret = i2c_write_blocking(I2C_PORT, addr, &dummy, 1, true);
        if (ret >= 0) {
            printf("  - Found device at 0x%02X\n", addr);
        }
        // Ignore NACK errors
    }
}

bool i2c_read_environment(env_data_t *out) {
    // TODO: implement real AHT20 + BMP280 drivers
    // For now: return false so you remember to implement it.
    out->temperature_c = 0.0f;
    out->humidity_rh = 0.0f;
    out->pressure_hpa = 0.0f;
    out->ok_aht20 = false;
    out->ok_bmp280 = false;
    return false;
}

