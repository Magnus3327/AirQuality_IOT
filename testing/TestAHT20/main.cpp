#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define ADDR_AHT 0x38
#define PIN_SDA 4
#define PIN_SCL 5

int main() {
    stdio_init_all();

    // Inizializzazione I2C a 100kHz
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    
    // Abilitiamo i pull-up interni
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);

    // Aspettiamo che la seriale si colleghi
    sleep_ms(3000);
    printf("\n--- Test AHT20 (Solo Temperatura e Umidita) ---\n");

    // Inizializzazione sensore: invio comando di trigger iniziale
    uint8_t init_cmd[] = {0xBE, 0x08, 0x00};
    i2c_write_blocking(I2C_PORT, ADDR_AHT, init_cmd, 3, false);
    sleep_ms(20);

    while (true) {
        // 1. Invia il comando di misurazione
        uint8_t measure_cmd[] = {0xAC, 0x33, 0x00};
        i2c_write_blocking(I2C_PORT, ADDR_AHT, measure_cmd, 3, false);
        
        // 2. Attendi la fine della misurazione (fondamentale)
        sleep_ms(80);
        
        // 3. Leggi i 6 byte di dati
        uint8_t data[6];
        int ret = i2c_read_blocking(I2C_PORT, ADDR_AHT, data, 6, false);

        if (ret < 0) {
            printf("Errore di lettura I2C!\n");
        } else {
            // Verifica bit di stato (bit 7 deve essere 0 = pronto)
            if ((data[0] & 0x80) == 0) {
                // Conversione Umidità (20 bit)
                uint32_t raw_hum = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
                float humidity = (float)raw_hum * 100 / 1048576.0;

                // Conversione Temperatura (20 bit)
                uint32_t raw_temp = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
                float temperature = (float)raw_temp * 200 / 1048576.0 - 50;

                printf("AHT20 >> Temperatura: %.2f C | Umidita: %.1f %%\n", temperature, humidity);
            } else {
                printf("Sensore occupato...\n");
            }
        }

        sleep_ms(1500);
    }
}