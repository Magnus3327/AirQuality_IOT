// include/i2c_sensors.h
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float temperature_c;
    float humidity_rh;
    float pressure_hpa;
    bool  ok_aht20;
    bool  ok_bmp280;
} env_data_t;

void i2c_sensors_init(void);
void i2c_scan_bus(void);

// Return true if at least one sensor ok
bool i2c_read_environment(env_data_t *out);

