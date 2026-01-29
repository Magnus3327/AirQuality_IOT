#ifndef AHT20_H
#define AHT20_H

#include <stdbool.h>

#include "app_config.h"

void aht20_init(void);
bool aht20_read(env_data_t *out);

#endif
