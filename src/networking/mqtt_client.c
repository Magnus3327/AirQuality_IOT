#include "mqtt_client.h"

#include <stdio.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

static mqtt_client_t *g_client;
static bool g_ready = false;
static void (*g_cmd_cb)(const char *payload, uint16_t len) = NULL;
static char g_in_topic[96];

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    (void)arg;
    (void)tot_len;
    strncpy(g_in_topic, topic, sizeof(g_in_topic) - 1u);
    g_in_topic[sizeof(g_in_topic) - 1u] = '\0';
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    (void)arg;
    if (flags & MQTT_DATA_FLAG_LAST) {
        if (strcmp(g_in_topic, MQTT_CMD_TOPIC) == 0 && g_cmd_cb) {
            char buf[64];
            u16_t n = (len < (sizeof(buf) - 1u)) ? len : (sizeof(buf) - 1u);
            memcpy(buf, data, n);
            buf[n] = '\0';
            g_cmd_cb(buf, n);
        }
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    (void)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        g_ready = true;
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
        mqtt_subscribe(client, MQTT_CMD_TOPIC, 1, NULL, NULL);
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
             "{\"name\":\"MQ135\",\"state_topic\":\"%s\",\"value_template\":\"{{ value_json.mq135_ppm }}\","
             "\"unit_of_measurement\":\"ppm\"}",
             MQTT_STATE_TOPIC);
    mqtt_publish(g_client, topic, payload, strlen(payload), 1, 1, NULL, NULL);

    snprintf(topic, sizeof(topic), "%s/mq7/config", MQTT_BASE_TOPIC);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"MQ7 CO\",\"state_topic\":\"%s\",\"value_template\":\"{{ value_json.mq7_ppm }}\","
             "\"unit_of_measurement\":\"ppm\"}",
             MQTT_STATE_TOPIC);
    mqtt_publish(g_client, topic, payload, strlen(payload), 1, 1, NULL, NULL);

    snprintf(topic, sizeof(topic), "homeassistant/button/air_quality/calibrate/config");
    snprintf(payload, sizeof(payload),
             "{\"name\":\"Air Calibrate\",\"command_topic\":\"%s\",\"payload_press\":\"CALIBRATE\"}",
             MQTT_CMD_TOPIC);
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

void mqtt_client_set_cmd_callback(void (*cb)(const char *payload, uint16_t len)) {
    g_cmd_cb = cb;
}

bool mqtt_client_publish(const telem_t *t) {
    if (!g_ready || g_client == NULL) return false;

    char payload[256];
    int n = snprintf(payload, sizeof(payload),
                     "{\"temp_c\":%.2f,\"rh_pct\":%.2f,"
                     "\"mq135_ppm\":%.2f,\"mq7_ppm\":%.2f,"
                     "\"mq135_raw\":%u,\"mq7_raw\":%u,"
                     "\"mq135_state\":%u,\"mq7_state\":%u,"
                     "\"heater\":%u}",
                     t->env.temp_c, t->env.rh_pct,
                     t->mq135_ppm, t->mq7_ppm,
                     t->mq135_raw, t->mq7_raw,
                     (unsigned)t->mq135_state, (unsigned)t->mq7_state,
                     (unsigned)t->mq7_heater_on);
    if (n <= 0) return false;

    cyw43_arch_lwip_begin();
    err_t err = mqtt_publish(g_client, MQTT_STATE_TOPIC, payload, (uint16_t)strlen(payload),
                             1, 0, NULL, NULL);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}
