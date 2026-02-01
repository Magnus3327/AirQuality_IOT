#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

// Inclusioni moduli del progetto
#include "oled.h"
#include "aht20.h"
#include "mq135.h"
#include "mq7.h"
#include "mqtt.h"

// --- CONFIGURAZIONE RETE ---
#define WIFI_SSID "pico"
#define WIFI_PASS "12345678"

// --- CONFIGURAZIONE PIN ---
#define I2C_PORT         i2c0
#define ADC_MQ135_PIN    26
#define ADC_MQ7_PIN      27
#define MQ7_PWM_PIN      15

// Funzione di utilità per stampare sia su Seriale che su OLED
void log_status(const char *msg, int line) {
    printf("%s\n", msg);
    oled_write_str((char*)msg, line, 0);
}

int main() {
    // Inizializzazione Standard I/O (USB)
    stdio_init_all();
    sleep_ms(2000); // Attesa per apertura terminale seriale

    // 1. INIZIALIZZAZIONE HARDWARE
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C); // SDA
    gpio_set_function(5, GPIO_FUNC_I2C); // SCL
    adc_init();
    
    oled_init();
    oled_clear();
    log_status("SYSTEM STARTUP", 0);

    // 2. INIZIALIZZAZIONE SENSORI
    aht20_init();
    mq135_init(ADC_MQ135_PIN);
    mq7_init(ADC_MQ7_PIN, MQ7_PWM_PIN);
    log_status("SENSORS READY", 1);

    // 3. CONNESSIONE WI-FI
    if (cyw43_arch_init()) {
        log_status("WIFI INIT ERR", 2);
        return -1;
    }
    cyw43_arch_enable_sta_mode();
    
    log_status("CONNECTING WIFI...", 2);
    log_status("CONNECTING WIFI...", 2);
    
    // Tentativo di connessione
    int res = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000);
    
    if (res != 0) {
        // Stampiamo il codice di errore numerico per capire il motivo
        printf("WIFI CONNECT FAIL. Code: %d\n", res); 
        log_status("WIFI ERR", 2);
        
        // Se res è -1: probabilmente password errata o timeout
        // Se res è -2: SSID non trovato (il router è troppo lontano o a 5GHz)
    } else {
        log_status("WIFI CONNECTED", 2);
        printf("Connessione stabilita!\n");
        
        char ip_msg[20];
        struct netif *n = &cyw43_state.netif[CYW43_ITF_STA];
        sprintf(ip_msg, "IP:%s", ip4addr_ntoa(netif_ip4_addr(n)));
        log_status(ip_msg, 3);
        
        // 4. INIZIALIZZAZIONE MQTT
        mqtt_manager_init();
        log_status("MQTT CONNECTING", 4);
    }
    sleep_ms(3000);
    oled_clear();

    // 5. CALIBRAZIONE R0 (In aria pulita all'avvio)
    log_status("CALIBRATING...", 0);
    float v135_cal = mq135_get_voltage();
    float r0_135 = mq135_calibrate_r0(v135_cal);
    float r0_7 = 0.0f; // Verrà calibrato al primo ciclo LOW dell'MQ-7
    log_status("CALIB DONE", 1);
    sleep_ms(2000);
    oled_clear();

    // Variabili per la gestione del ciclo MQ-7
    uint32_t last_mq7_switch = to_ms_since_boot(get_absolute_time());
    mq7_state_t current_mq7_state = MQ7_STATE_HIGH; // Inizia con fase 5V (Pulizia)

    // --- LOOP PRINCIPALE ---
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        float temp, hum;
        
        // Lettura AHT20
        aht20_read(&temp, &hum);

        // --- GESTIONE MQ-135 (NH3) ---
        float v_135 = mq135_get_voltage();
        float nh3_ppm = mq135_get_ppm(v_135, r0_135);

        // --- GESTIONE MQ-7 (CO) ---
        float co_ppm = 0.0f;
        if (current_mq7_state == MQ7_STATE_HIGH) {
            mq7_set_heater(MQ7_STATE_HIGH); // Fase 5V
            if (now - last_mq7_switch >= 60000) {
                current_mq7_state = MQ7_STATE_LOW;
                last_mq7_switch = now;
            }
        } else {
            mq7_set_heater(MQ7_STATE_LOW); // Fase 1.5V
            float v_7 = mq7_get_voltage();
            
            // CORREZIONE 1: Passiamo temp e hum per la calibrazione
            if (r0_7 == 0) {
                r0_7 = mq7_calibrate_r0(v_7, temp, hum); 
            }
            
            // CORREZIONE 2: Passiamo temp e hum per il calcolo dei PPM
            co_ppm = mq7_get_ppm(v_7, r0_7, temp, hum);

            if (now - last_mq7_switch >= 90000) {
                current_mq7_state = MQ7_STATE_HIGH;
                last_mq7_switch = now;
            }
        }

        // --- AGGIORNAMENTO DISPLAY OLED ---
        char line_buf[32];
        sprintf(line_buf, "T:%.1fC H:%.0f%%", temp, hum);
        oled_write_str(line_buf, 0, 0);

        sprintf(line_buf, "NH3: %.2f PPM", nh3_ppm);
        oled_write_str(line_buf, 2, 0);

        if (current_mq7_state == MQ7_STATE_LOW) {
            sprintf(line_buf, "CO:  %.2f PPM", co_ppm);
        } else {
            sprintf(line_buf, "CO:  WARMING...");
        }
        oled_write_str(line_buf, 4, 0);

        // --- TRASMISSIONE DATI MQTT ---
        // Invia i dati al broker EMQX per Home Assistant
        mqtt_publish_data(nh3_ppm, co_ppm, temp, hum, (int)current_mq7_state);

        // Debug Seriale (per monitoraggio PC)
        printf("DATA | NH3:%.2f | CO:%.2f | T:%.1f | H:%.1f | MQ7_S:%d | MQTT:%d\n", 
                nh3_ppm, co_ppm, temp, hum, current_mq7_state, mqtt_is_connected_to_broker());

        sleep_ms(2000); // Frequenza di campionamento
    }

    return 0;
}