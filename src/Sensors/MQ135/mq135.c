#include "mq135.h"
#include "hardware/adc.h"
#include <math.h>

/** Constants for MQ-135 Sensitivity Curve (NH3 Reference) */
#define MQ135_VC            5.0f    
#define MQ135_RL            930.0f  /**  Load Resistor Value (User measured A0-GND) */
#define MQ135_A_GAS         110.47f /** Scaling factor for the power function */
#define MQ135_B_GAS         -2.86f  /** Exponent (slope) for the power function */
#define CLEAN_AIR_RATIO     3.6f    /** Characteristic Rs/R0 ratio in clean air */

void mq135_init(int adc_pin) {
    adc_gpio_init(adc_pin);
}

float mq135_get_voltage(void) {
    uint32_t sum = 0;
    const int samples = 64;
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
    }
    /** * ADC 12-bit (0-3.3V) with 1.5x external voltage divider compensation 
     * allows reading the 0-5V sensor range safely.
     */
    return (((float)sum / samples) * 3.3f / 4095.0f) * 1.5f;
}

float mq135_get_rs(float v_out) {
    if (v_out <= 0.1f) return 1000000.0f; /** High resistance for low voltage */
    return ((MQ135_VC * MQ135_RL) / v_out) - MQ135_RL;
}

/**
 * @brief Calculate correction factor based on Temp/Hum datasheet curves.
 */
static float mq135_get_correction_factor(float t, float h) {
    return 1.45f - 0.015f * t - 0.0015f * h;
}

float mq135_calibrate_r0(float v_out, float t, float h) {
    float rs_air = mq135_get_rs(v_out);
    float correction = mq135_get_correction_factor(t, h);
    return (rs_air / CLEAN_AIR_RATIO) * correction;
}

float mq135_get_ppm(float v_out, float r0, float t, float h) {
    if (r0 <= 0 || v_out <= 0.1f) return 0.0f;
    
    float rs = mq135_get_rs(v_out);
    float correction = mq135_get_correction_factor(t, h);
    float ratio = (rs / r0) * correction;
    
    /** Calculate concentration using the power law model */
    float ppm = MQ135_A_GAS * pow(ratio, MQ135_B_GAS);
    
    /** Filter noise: readings below 10 PPM are treated as clean air */
    return (ppm < 10.0f) ? 0.0f : ppm;
}