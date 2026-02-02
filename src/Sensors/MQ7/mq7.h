#ifndef MQ7_H
#define MQ7_H

#include <stdint.h>

/** Heater operating states */
typedef enum {
    MQ7_STATE_HIGH, /** 5.0V - Cleaning phase */
    MQ7_STATE_LOW   /** 1.5V - Measurement phase */
} mq7_state_t;

/**
 * @brief Initialize MQ-7 (ADC for reading, PWM for heater)
 */
void mq7_init(int adc_pin, int pwm_pin);

/**
 * @brief Set heater voltage via PWM
 */
void mq7_set_heater(mq7_state_t state);

/**
 * @brief Get sensor voltage (compensated for 1.5x divider)
 */
float mq7_get_voltage(void);

/**
 * @brief Calculate sensor resistance Rs
 */
float mq7_get_rs(float v_out);

/**
 * @brief Calibrate R0 in clean air during LOW phase
 */
float mq7_calibrate_r0(float v_out, float t, float h);

/**
 * @brief Calculate CO concentration in PPM
 */
float mq7_get_ppm(float v_out, float r0, float t, float h);

#endif