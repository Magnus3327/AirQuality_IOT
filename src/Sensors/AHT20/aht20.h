#ifndef AHT20_H
#define AHT20_H

#include "pico/stdlib.h"

void aht20_init();
void aht20_read(float *t, float *h);

#endif