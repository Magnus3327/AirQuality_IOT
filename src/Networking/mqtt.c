#include "mqtt.h"
#include "lwip/apps/mqtt.h"

static mqtt_client_t *client;

void mqtt_init(ip_addr_t *server_addr) {
    client = mqtt_client_new();
    struct mqtt_connect_client_info_t ci = { .client_id = "PicoW_Air", .keep_alive = 60 };
    mqtt_client_connect(client, server_addr, 1883, NULL, NULL, &ci);
}

void mqtt_send_data(const char* topic, const char* payload) {
    if (mqtt_client_is_connected(client)) {
        mqtt_publish(client, topic, payload, strlen(payload), 0, 0, NULL, NULL);
    }
}