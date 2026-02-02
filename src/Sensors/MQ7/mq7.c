#include "mq7.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <math.h>

/** Constants for MQ-7 Curve (CO Reference) */
#define MQ7_VC              5.0f    
#define MQ7_RL              980.0f  /**  Load Resistor Value (User measured A0-GND) */
#define MQ7_A_CO            100.0f
#define MQ7_B_CO            -0.465f
#define CLEAN_AIR_RATIO     27.0f   /** Ratio Rs/R0 in clean air (Datasheet) */
#define MQ7_PPM_OFFSET      20.0f   /** Offset to zero the clean air baseline */

static int _pwm_pin;

void mq7_init(int adc_pin, int pwm_pin) {
    adc_gpio_init(adc_pin);
    _pwm_pin = pwm_pin;
    
    gpio_set_function(_pwm_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(_pwm_pin);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
}

void mq7_set_heater(mq7_state_t state) {
    uint slice_num = pwm_gpio_to_slice_num(_pwm_pin);
    if (state == MQ7_STATE_HIGH) {
        pwm_set_gpio_level(_pwm_pin, 255); // 5.0V
    } else {
        pwm_set_gpio_level(_pwm_pin, 77);  // ~1.5V
    }
}

float mq7_get_voltage(void) {
    uint32_t sum = 0;
    const int samples = 64;
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
    }
    // ADC 12-bit (0-3.3V) with 1.5x voltage divider compensation
    return (((float)sum / samples) * 3.3f / 4095.0f) * 1.5f;
}

float mq7_get_rs(float v_out) {
    if (v_out <= 0.1f) return 1000000.0f;
    return ((MQ7_VC * MQ7_RL) / v_out) - MQ7_RL;
}

static float mq7_get_correction_factor(float t, float h) {
    return 1.42f - 0.0132f * t - 0.0012f * h;
}

float mq7_calibrate_r0(float v_out, float t, float h) {
    float rs_air = mq7_get_rs(v_out);
    float correction = mq7_get_correction_factor(t, h);
    return (rs_air / CLEAN_AIR_RATIO) * correction;
}

float mq7_get_ppm(float v_out, float r0, float t, float h) {
    if (r0 <= 0 || v_out <= 0.1f) return 0.0f;
    
    float rs = mq7_get_rs(v_out);
    float correction = mq7_get_correction_factor(t, h);
    float ratio = (rs / r0) * correction;
    
    float raw_ppm = MQ7_A_CO * pow(ratio, MQ7_B_CO);
    
    // Apply -20 PPM offset and clamp to 0
    float final_ppm = raw_ppm - MQ7_PPM_OFFSET;
    return (final_ppm < 0.0f) ? 0.0f : final_ppm;
}