#pragma once
#include <stdbool.h>
#include "aq_filter.h"
#include "i2c_sensors.h"

typedef struct {
    uint16_t mq135_raw, mq135_filt;
    uint16_t mq7_raw,   mq7_filt;
    aq_state_t mq135_state;
    aq_state_t mq7_state;
    env_data_t env;
    bool mq7_heater_on;
} telem_t;

bool ha_publish_init(void);      // connect Wi-Fi + MQTT
bool ha_publish_is_ready(void);  // connected?
bool ha_publish_send(const telem_t *t);

