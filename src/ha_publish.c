// src/ha_publish.c
#include "ha_publish.h"
#include <stdio.h>

void ha_publish_init(void) {
    // TODO: init Wi-Fi / MQTT / HTTP
}

static const char* st2s(aq_state_t s) {
    switch (s) {
        case AQ_OK: return "OK";
        case AQ_WARNING: return "WARNING";
        case AQ_DANGER: return "DANGER";
        default: return "UNK";
    }
}

void ha_publish_send(const telem_t *t) {
    // Serial “publisher” (works immediately; swap later with real HomeAssistant publish)
    printf("{\"mq135_raw\":%u,\"mq135_filt\":%u,\"mq135_state\":\"%s\","
           "\"mq7_raw\":%u,\"mq7_filt\":%u,\"mq7_state\":\"%s\","
           "\"heater\":%s}\n",
           t->mq135_raw, t->mq135_filt, st2s(t->mq135_state),
           t->mq7_raw,   t->mq7_filt,   st2s(t->mq7_state),
           t->mq7_heater_on ? "true" : "false");
}

