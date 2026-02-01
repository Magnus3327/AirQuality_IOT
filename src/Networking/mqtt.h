#ifndef MQTT_H
#define MQTT_H

#include "lwip/apps/mqtt.h"

// Parametri Broker EMQX
#define MQTT_BROKER_IP   "192.168.1.4"
#define MQTT_BROKER_PORT 1883
#define MQTT_USER        "pico"
#define MQTT_PASS        "1234567890!x"
#define MQTT_CLIENT_ID   "PicoW_AirStation"

// Topic MQTT
#define MQTT_TOPIC       "casa/aria/sensori"

// Prototipi funzioni
void mqtt_manager_init(void);
void mqtt_publish_data(float nh3, float co, float t, float h, int state);
bool mqtt_is_connected_to_broker(void);

#endif