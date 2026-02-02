#ifndef MQTT_H
#define MQTT_H

#include "lwip/apps/mqtt.h"

/** Broker Configuration */
#define MQTT_BROKER_IP      "192.168.1.4"
#define MQTT_BROKER_PORT    1883
#define MQTT_USER           NULL
#define MQTT_PASS           NULL
#define MQTT_CLIENT_ID      "PicoW_AirStation"

/** MQTT Topics */
#define MQTT_TOPIC_DATA     "casa/aria/sensori"

/**
 * @brief Initialize the MQTT client and attempt connection to the broker.
 */
void mqtt_manager_init(void);

/**
 * @brief Publish sensor data in JSON format.
 * @param nh3 NH3 concentration in PPM
 * @param co CO concentration in PPM
 * @param t Temperature in Celsius
 * @param h Humidity in %RH
 * @param state MQ-7 heater state
 */
void mqtt_publish_data(float nh3, float co, float t, float h, int state);

/**
 * @brief Check if the MQTT client is currently connected.
 * @return true if connected, false otherwise
 */
bool mqtt_is_connected_to_broker(void);

#endif