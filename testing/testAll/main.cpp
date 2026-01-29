#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

// --- CONFIGURAZIONE PIN E INDIRIZZI ---
#define I2C_PORT i2c0
#define ADDR_AHT 0x38
#define ADDR_OLED 0x3C
#define PIN_SDA 4
#define PIN_SCL 5
#define MQ135_ADC 0
#define MQ7_ADC 1
#define MQ7_PWM_PIN 15

// Costanti MQ7
const float RL = 10.0;
float R0_MQ7 = 4.0;
const float m_mq7 = -1.53;
const float b_mq7 = 1.72;

// --- DRIVER OLED MINIMALE (SSD1306) ---
const uint8_t font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x7C, 0x12, 0x11, 0x12, 0x7C, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x44, 0x44, 0x44, 0x00  // :
};

void oled_cmd(uint8_t cmd) {
    uint8_t b[] = {0x00, cmd};
    i2c_write_blocking(I2C_PORT, ADDR_OLED, b, 2, false);
}

void oled_init() {
    uint8_t cmds[] = {0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
    for(int i=0; i<sizeof(cmds); i++) oled_cmd(cmds[i]);
}

void oled_set_cursor(uint8_t line, uint8_t col) {
    oled_cmd(0xB0 + line);
    oled_cmd(0x00 + (col & 0x0F));
    oled_cmd(0x10 + (col >> 4));
}

void oled_write_char(char c) {
    uint8_t b[6] = {0x40};
    int idx = 0;
    if(c >= '0' && c <= '9') idx = (c - '0' + 1) * 5;
    else if(c >= 'A' && c <= 'Z') idx = (c - 'A' + 11) * 5;
    else if(c == ' ') idx = 0;
    else if(c == ':') idx = 38 * 5;
    else if(c == '!') idx = 37 * 5;
    
    for(int i=0; i<5; i++) b[i+1] = font[idx + i];
    i2c_write_blocking(I2C_PORT, ADDR_OLED, b, 6, false);
}

void oled_print(uint8_t line, const char* str) {
    oled_set_cursor(line, 0);
    while(*str) oled_write_char(*str++);
}

void oled_clear() {
    for(int i=0; i<8; i++) {
        oled_set_cursor(i, 0);
        uint8_t b[129] = {0x40};
        i2c_write_blocking(I2C_PORT, ADDR_OLED, b, 129, false);
    }
}

// --- LOGICA SENSORI ---

float get_v_stable(int ch) {
    adc_select_input(ch);
    uint32_t s = 0;
    for(int i=0; i<5000; i++) s += adc_read();
    return ((float)s / 5000.0f) * (3.3f / 4095.0f) * 1.5f;
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA); gpio_pull_up(PIN_SCL);
    
    adc_init();
    adc_gpio_init(26); adc_gpio_init(27);
    gpio_set_function(MQ7_PWM_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(MQ7_PWM_PIN);
    pwm_set_wrap(slice, 65535);
    pwm_set_enabled(slice, true);

    sleep_ms(2000);
    oled_init();
    oled_clear();
    oled_print(0, "CALIBRAZIONE");
    oled_print(2, "ATTENDERE 2 MIN");

    // Fase Calibrazione
    pwm_set_gpio_level(MQ7_PWM_PIN, 65535); // 5V
    sleep_ms(60000);
    pwm_set_gpio_level(MQ7_PWM_PIN, 18350); // 1.4V
    sleep_ms(50000); // Stabilizzazione
    float v_cal = get_v_stable(MQ7_ADC);
    R0_MQ7 = ((5.0f / v_cal) - 1.0f) * RL;
    oled_clear();

    while(true) {
        pwm_set_gpio_level(MQ7_PWM_PIN, 18350);
        for(int i=0; i<90; i++) {
            // AHT20
            uint8_t m_c[] = {0xAC, 0x33, 0x00};
            i2c_write_blocking(I2C_PORT, ADDR_AHT, m_c, 3, false);
            sleep_ms(80);
            uint8_t data[6];
            i2c_read_blocking(I2C_PORT, ADDR_AHT, data, 6, false);
            float hum = (float)(((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4)) * 100 / 1048576.0;
            float temp = (float)(((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5]) * 200 / 1048576.0 - 50;

            // MQ
            float v135 = get_v_stable(0);
            float v7 = get_v_stable(1);
            float ratio = (((5.0f / v7) - 1.0f) * RL) / R0_MQ7;
            float ppm = pow(10, (log10(ratio) + b_mq7) / fabs(m_mq7));

            // Display
            char buf[20];
            oled_print(0, "ARIA: ");
            oled_print(0, (v135 < 1.0 ? "BUONA" : (v135 < 2.0 ? "DISCRETA" : "PESSIMA")));
            
            sprintf(buf, "TEMP: %0.1f C", temp); oled_print(2, buf);
            sprintf(buf, "HUM:  %0.1f %%", hum);  oled_print(4, buf);
            sprintf(buf, "CO:   %0.1f PPM", ppm); oled_print(6, buf);

            sleep_ms(920);
        }
        // Pulizia ciclica
        oled_clear();
        oled_print(2, "PULIZIA SENSORE");
        pwm_set_gpio_level(MQ7_PWM_PIN, 65535);
        sleep_ms(60000);
    }
}