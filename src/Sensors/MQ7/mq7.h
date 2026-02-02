#ifndef MQ7_H
#define MQ7_H

#include <stdint.h>

// Stati del riscaldatore
typedef enum {
    MQ7_STATE_HIGH, // 5V - Pulizia (60s)
    MQ7_STATE_LOW   // 1.5V - Misura (90s)
} mq7_state_t;

// Inizializzazione (ADC + PWM)
void mq7_init(int adc_pin, int pwm_pin);

// Imposta la potenza del riscaldatore via PWM
void mq7_set_heater(mq7_state_t state);

// Lettura tensione (corretta per partitore 1.5x)
float mq7_get_voltage(void);

// Calcolo Rs
float mq7_get_rs(float v_out);

// Calibrazione R0 (da fare a fine fase LOW in aria pulita)
float mq7_calibrate_r0(float v_out, float temperature_c, float humidity_rh);

// Calcolo PPM Monossido di Carbonio
float mq7_get_ppm(float v_out, float r0, float temperature_c, float humidity_rh);

#endif