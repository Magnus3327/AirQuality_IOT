#ifndef MQTT_H
#define MQTT_H

#include "lwip/ip_addr.h"

void mqtt_init(ip_addr_t *server_addr);
void mqtt_send_data(const char* topic, const char* payload);

#endif