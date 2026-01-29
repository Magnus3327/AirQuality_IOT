// src/mq7_heater.c
#include "mq7_heater.h"
#include "app_config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

static volatile bool s_on = false;

void mq7_heater_init(void) {
    gpio_init(PIN_MQ7_HEATER_EN);
    gpio_set_dir(PIN_MQ7_HEATER_EN, GPIO_OUT);
    gpio_put(PIN_MQ7_HEATER_EN, 0);
    s_on = false;
}

void mq7_heater_set(bool on) {
    gpio_put(PIN_MQ7_HEATER_EN, on ? 1 : 0);
    s_on = on;
}

bool mq7_heater_get(void) { return s_on; }

