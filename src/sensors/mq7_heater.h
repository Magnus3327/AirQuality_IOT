#ifndef MQ7_HEATER_H
#define MQ7_HEATER_H

#include <stdbool.h>

void mq7_heater_init(void);
void mq7_heater_set(bool on);
bool mq7_heater_get(void);

#endif
