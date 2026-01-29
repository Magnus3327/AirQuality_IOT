#include "ha_publish.h"
#include "net_config.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

// --- internal state ---
static mqtt_client_t *g_client = NULL;
static volatile bool g_mqtt_connected = false;

static const char* st2s(aq_state_t s) {
    switch (s) {
        case AQ_OK: return "OK";
        case AQ_WARNING: return "WARNING";
        case AQ_DANGER: return "DANGER";
        default: return "UNK";
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    (void)client; (void)arg;
    g_mqtt_connected = (status == MQTT_CONNECT_ACCEPTED);

    if (g_mqtt_connected) {
        printf("[MQTT] Connected\n");
        // publish "online"
        const char *msg = "online";
        mqtt_publish(g_client, MQTT_TOPIC_STATUS, msg, strlen(msg), 1, 1, NULL, NULL);
    } else {
        printf("[MQTT] Disconnected (status=%d)\n", (int)status);
    }
}

static bool wifi_connect(void) {
    if (cyw43_arch_init()) {
        printf("[WiFi] cyw43 init failed\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    printf("[WiFi] Connecting to %s...\n", WIFI_SSID);
    int r = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000
    );
    if (r != 0) {
        printf("[WiFi] connect failed: %d\n", r);
        return false;
    }
    printf("[WiFi] Connected\n");
    return true;
}

static bool mqtt_connect(void) {
    g_client = mqtt_client_new();
    if (!g_client) {
        printf("[MQTT] mqtt_client_new failed\n");
        return false;
    }

    ip_addr_t broker_addr;
    if (!ipaddr_aton(MQTT_BROKER_IP, &broker_addr)) {
        printf("[MQTT] invalid broker ip\n");
        return false;
    }

    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = MQTT_CLIENT_ID;
    ci.keep_alive = 30;

    printf("[MQTT] Connecting to %s:%d...\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);
    err_t err = mqtt_client_connect(g_client, &broker_addr, MQTT_BROKER_PORT,
                                   mqtt_connection_cb, NULL, &ci);
    if (err != ERR_OK) {
        printf("[MQTT] connect error: %d\n", (int)err);
        return false;
    }
    return true;
}

bool ha_publish_init(void) {
    g_mqtt_connected = false;
    if (!wifi_connect()) return false;
    if (!mqtt_connect()) return false;
    return true;
}

bool ha_publish_is_ready(void) {
    return g_mqtt_connected;
}

static void pub_cb(void *arg, err_t result) {
    (void)arg;
    if (result != ERR_OK) {
        printf("[MQTT] publish failed: %d\n", (int)result);
    }
}

bool ha_publish_send(const telem_t *t) {
    if (!g_client || !g_mqtt_connected) return false;

    char payload[256];
    // JSON compatto (HA lo parse facilmente)
    int n = snprintf(payload, sizeof(payload),
        "{\"mq135_raw\":%u,\"mq135_filt\":%u,\"mq135_state\":\"%s\","
        "\"mq7_raw\":%u,\"mq7_filt\":%u,\"mq7_state\":\"%s\","
        "\"heater\":%s,"
        "\"temp_c\":%.2f,\"hum_rh\":%.2f,\"press_hpa\":%.2f}",
        t->mq135_raw, t->mq135_filt, st2s(t->mq135_state),
        t->mq7_raw,   t->mq7_filt,   st2s(t->mq7_state),
        t->mq7_heater_on ? "true" : "false",
        t->env.temperature_c, t->env.humidity_rh, t->env.pressure_hpa
    );
    if (n <= 0 || n >= (int)sizeof(payload)) return false;

    err_t err = mqtt_publish(g_client, MQTT_TOPIC_TELEM,
                            payload, (u16_t)strlen(payload),
                            0, 0, pub_cb, NULL);
    return (err == ERR_OK);
}

