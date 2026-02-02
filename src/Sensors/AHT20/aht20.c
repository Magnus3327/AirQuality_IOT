#include "aht20.h"
#include "hardware/i2c.h"

#define ADDR_AHT20 0x38

void aht20_init(void) {
    sleep_ms(40); // Wait for sensor power-on
    uint8_t status;
    
    // Check calibration status
    uint8_t cmd_status = 0x71;
    i2c_write_blocking(i2c0, ADDR_AHT20, &cmd_status, 1, true);
    i2c_read_blocking(i2c0, ADDR_AHT20, &status, 1, false);

    // If bit 3 is 0, trigger calibration
    if (!(status & 0x08)) {
        uint8_t cmd_cal[] = {0xBE, 0x08, 0x00};
        i2c_write_blocking(i2c0, ADDR_AHT20, cmd_cal, 3, false);
        sleep_ms(10);
    }
}

void aht20_read(float *t, float *h) {
    // Trigger measurement
    uint8_t cmd_meas[] = {0xAC, 0x33, 0x00};
    i2c_write_blocking(i2c0, ADDR_AHT20, cmd_meas, 3, false);
    
    // Wait for conversion (80ms per datasheet)
    sleep_ms(80);
    
    uint8_t data[6];
    i2c_read_blocking(i2c0, ADDR_AHT20, data, 6, false);

    // Data parsing according to AHT20 datasheet
    uint32_t hum_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *h = (float)hum_raw * 100.0f / 1048576.0f;
    *t = (float)temp_raw * 200.0f / 1048576.0f - 50.0f;
}