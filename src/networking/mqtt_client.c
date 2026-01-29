#include "mqtt_client.h"

#include <stdio.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

static mqtt_client_t *g_client;
static bool g_ready = false;

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    (void)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        g_ready = true;
    } else {
        g_ready = false;
        mqtt_disconnect(client);
    }
}

static void mqtt_publish_discovery(void) {
    if (!g_ready || g_client == NULL) return;

    const char *dev_class_temp = "temperature";
    const char *dev_class_hum = "humidity";

    char topic[96];
    char payload[256];

    cyw43_arch_lwip_begin();

    snprintf(topic, sizeof(topic), "%s/temp/config", MQTT_BASE_TOPIC);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"Air Temp\",\"state_topic\":\"%s\",\"unit_of_measurement\":\"C\","
             "\"device_class\":\"%s\",\"value_template\":\"{{ value_json.temp_c }}\"}",
             MQTT_STATE_TOPIC, dev_class_temp);
    mqtt_publish(g_client, topic, payload, strlen(payload), 1, 1, NULL, NULL);

    snprintf(topic, sizeof(topic), "%s/hum/config", MQTT_BASE_TOPIC);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"Air Hum\",\"state_topic\":\"%s\",\"unit_of_measurement\":\"%%\","
             "\"device_class\":\"%s\",\"value_template\":\"{{ value_json.rh_pct }}\"}",
             MQTT_STATE_TOPIC, dev_class_hum);
    mqtt_publish(g_client, topic, payload, strlen(payload), 1, 1, NULL, NULL);

    snprintf(topic, sizeof(topic), "%s/mq135/config", MQTT_BASE_TOPIC);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"MQ135\",\"state_topic\":\"%s\",\"value_template\":\"{{ value_json.mq135 }}\"}",
             MQTT_STATE_TOPIC);
    mqtt_publish(g_client, topic, payload, strlen(payload), 1, 1, NULL, NULL);

    snprintf(topic, sizeof(topic), "%s/mq7/config", MQTT_BASE_TOPIC);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"MQ7\",\"state_topic\":\"%s\",\"value_template\":\"{{ value_json.mq7 }}\"}",
             MQTT_STATE_TOPIC);
    mqtt_publish(g_client, topic, payload, strlen(payload), 1, 1, NULL, NULL);

    cyw43_arch_lwip_end();
}

bool mqtt_client_init(void) {
    if (cyw43_arch_init()) return false;

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        return false;
    }

    g_client = mqtt_client_new();
    if (!g_client) return false;

    ip_addr_t broker_addr;
    IP4_ADDR(&broker_addr, MQTT_BROKER_IP0, MQTT_BROKER_IP1, MQTT_BROKER_IP2, MQTT_BROKER_IP3);

    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = MQTT_CLIENT_ID;

    g_ready = false;
    cyw43_arch_lwip_begin();
    mqtt_connect(g_client, &broker_addr, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
    cyw43_arch_lwip_end();

    // Give time for connection
    for (int i = 0; i < 100; ++i) {
        if (g_ready) break;
        sleep_ms(50);
    }

    mqtt_publish_discovery();
    return g_ready;
}

bool mqtt_client_is_ready(void) {
    return g_ready;
}

bool mqtt_client_publish(const telem_t *t) {
    if (!g_ready || g_client == NULL) return false;

    char payload[256];
    int n = snprintf(payload, sizeof(payload),
                     "{\"temp_c\":%.2f,\"rh_pct\":%.2f,"
                     "\"mq135\":%u,\"mq7\":%u,"
                     "\"mq135_state\":%u,\"mq7_state\":%u,"
                     "\"heater\":%u}",
                     t->env.temp_c, t->env.rh_pct,
                     t->mq135_filt, t->mq7_filt,
                     (unsigned)t->mq135_state, (unsigned)t->mq7_state,
                     (unsigned)t->mq7_heater_on);
    if (n <= 0) return false;

    cyw43_arch_lwip_begin();
    err_t err = mqtt_publish(g_client, MQTT_STATE_TOPIC, payload, (uint16_t)strlen(payload),
                             1, 0, NULL, NULL);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}
