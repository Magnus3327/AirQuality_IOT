#ifndef AHT20_H
#define AHT20_H

#include "pico/stdlib.h"

/**
 * @brief Initialize AHT20 sensor and perform calibration if needed.
 */
void aht20_init(void);

/**
 * @brief Read temperature and humidity data.
 * @param t Pointer to float for temperature output
 * @param h Pointer to float for humidity output
 */
void aht20_read(float *t, float *h);

#endif