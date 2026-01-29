#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>

#include "app_config.h"

bool mqtt_client_init(void);
bool mqtt_client_is_ready(void);
bool mqtt_client_publish(const telem_t *t);

#endif
