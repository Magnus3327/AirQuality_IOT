#include "aht20.h"
#include "hardware/i2c.h"

#define ADDR_AHT20 0x38

void aht20_init() {
    sleep_ms(40); // Tempo di avvio post-power on
    uint8_t status;
    
    // Controlla se il sensore è calibrato
    uint8_t cmd_status = 0x71;
    i2c_write_blocking(i2c0, ADDR_AHT20, &cmd_status, 1, true);
    i2c_read_blocking(i2c0, ADDR_AHT20, &status, 1, false);

    // Se il bit 3 dello stato è 0, serve calibrazione
    if (!(status & 0x08)) {
        uint8_t cmd_cal[] = {0xBE, 0x08, 0x00};
        i2c_write_blocking(i2c0, ADDR_AHT20, cmd_cal, 3, false);
        sleep_ms(10);
    }
}

void aht20_read(float *t, float *h) {
    // Comando di trigger misura
    uint8_t cmd_meas[] = {0xAC, 0x33, 0x00};
    i2c_write_blocking(i2c0, ADDR_AHT20, cmd_meas, 3, false);
    
    // Attesa fine conversione (80ms come da datasheet)
    sleep_ms(80);

    uint8_t data[6];
    i2c_read_blocking(i2c0, ADDR_AHT20, data, 6, false);

    // Trasformazione dati grezzi (20 bit per valore)
    uint32_t hum_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    // Formule di conversione
    *h = (float)hum_raw * 100.0f / 1048576.0f;
    *t = (float)temp_raw * 200.0f / 1048576.0f - 50.0f;
}