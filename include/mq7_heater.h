// include/mq7_heater.h
#pragma once
#include <stdbool.h>

void mq7_heater_init(void);
void mq7_heater_set(bool on);
bool mq7_heater_get(void);

