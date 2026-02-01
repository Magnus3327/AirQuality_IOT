#include "mqtt.h"
#include <string.h>
#include <stdio.h>
#include "pico/cyw43_arch.h"

static mqtt_client_t *mqtt_client;
static bool connected = false;

// Callback: Stato della connessione al broker
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT: Connesso al broker EMQX!\n");
        connected = true;
    } else {
        printf("MQTT: Connessione fallita (status %d), riprovo...\n", status);
        connected = false;
    }
}

void mqtt_manager_init(void) {
    mqtt_client = mqtt_client_new();
    if (mqtt_client == NULL) return;

    ip_addr_t broker_addr;
    if (!ipaddr_aton(MQTT_BROKER_IP, &broker_addr)) {
        printf("MQTT: Errore indirizzo IP Broker\n");
        return;
    }

    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_CLIENT_ID;
    ci.client_user = MQTT_USER;
    ci.client_pass = MQTT_PASS;
    ci.keep_alive = 60;

    // Tentativo di connessione
    mqtt_client_connect(mqtt_client, &broker_addr, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
}

void mqtt_publish_data(float nh3, float co, float t, float h, int state) {
    if (!connected || mqtt_client == NULL) return;

    char json_payload[150];
    snprintf(json_payload, sizeof(json_payload), 
             "{\"nh3\":%.2f,\"co\":%.2f,\"temp\":%.1f,\"hum\":%.1f,\"state\":%d}", 
             nh3, co, t, h, state);

    err_t err = mqtt_publish(mqtt_client, MQTT_TOPIC, json_payload, strlen(json_payload), 0, 0, NULL, NULL);
    
    if (err != ERR_OK) {
        printf("MQTT: Errore pubblicazione (%d)\n", err);
    }
}

bool mqtt_is_connected_to_broker(void) {
    return connected;
}