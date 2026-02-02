#include "mq135.h"
#include "hardware/adc.h"
#include <math.h>

#define MQ135_VC            5.0f    
#define MQ135_RL            1000.0f // Valore misurato da te
#define MQ135_A_GAS         110.47f 
#define MQ135_B_GAS         -2.86f  

void mq135_init(int adc_pin) {
    adc_gpio_init(adc_pin);
}

float mq135_get_voltage(void) {
    uint32_t sum = 0;
    const int samples = 64;
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
    }
    return (( (float)sum / samples ) * 3.3f / 4095.0f) * 1.5f;
}

float mq135_get_rs(float v_out) {
    if (v_out <= 0.1f) return 1000000.0f;
    return ((MQ135_VC * MQ135_RL) / v_out) - MQ135_RL;
}

// Compensazione ambientale per MQ-135
float mq135_get_correction_factor(float t, float h) {
    // Modello semplificato: l'umidità influisce pesantemente sulla conduttività dell'ossido di stagno
    return 1.45f - 0.015f * t - 0.0015f * h;
}

float mq135_calibrate_r0(float v_out, float t, float h) {
    float rs_air = mq135_get_rs(v_out);
    float correction = mq135_get_correction_factor(t, h);
    // In aria pulita Rs/R0 è circa 3.6
    return (rs_air / 3.6f) * correction;
}

float mq135_get_ppm(float v_out, float r0, float t, float h) {
    if (r0 <= 0 || v_out <= 0.1f) return 0.0f;
    
    float rs = mq135_get_rs(v_out);
    float correction = mq135_get_correction_factor(t, h);
    float ratio = (rs / r0) * correction;
    
    float ppm = MQ135_A_GAS * pow(ratio, MQ135_B_GAS);
    return (ppm < 10.0f) ? 0.0f : ppm; // Sotto i 10ppm siamo nel rumore
}