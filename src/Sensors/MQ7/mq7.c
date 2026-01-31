#include "mq7.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <math.h>

/* ================= CONFIGURAZIONE MQ-7 ================= */
#define MQ7_VC              5.0f
#define MQ7_RL              830.0f  // Il tuo valore misurato
#define MQ7_PWM_PIN         15      // Pin base transistor

// Coefficienti curva CO (Monossido di Carbonio)
// Basati su: PPM = A * (Rs/R0)^B
static const float MQ7_CO_A = 99.042f;
static const float MQ7_CO_B = -1.518f;

void mq7_init(int adc_pin, int pwm_pin) {
    adc_gpio_init(adc_pin);
    
    // Setup PWM per l'heater
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
}

void mq7_set_heater(mq7_state_t state) {
    uint slice_num = pwm_gpio_to_slice_num(MQ7_PWM_PIN);
    if (state == MQ7_STATE_HIGH) {
        // 5.0V -> 100% Duty Cycle
        pwm_set_gpio_level(MQ7_PWM_PIN, 255);
    } else {
        // 1.5V -> (1.5 / 5.0) * 255 = ~76 (30% Duty Cycle)
        pwm_set_gpio_level(MQ7_PWM_PIN, 76);
    }
}

float mq7_get_voltage(void) {
    uint16_t raw = adc_read();
    float v_adc = (raw * 3.3f) / 4095.0f;
    return v_adc * 1.5f; // Compensazione partitore 1k/2k
}

float mq7_get_rs(float v_out) {
    if (v_out < 0.1f) return 1e6f;
    return ((MQ7_VC * MQ7_RL) / v_out) - MQ7_RL;
}

static float get_correction_factor(float t, float rh) {
    // L'MQ-7 è molto sensibile a T/RH, usiamo il modello standard
    return (-0.003f * t + 1.06f) + (rh <= 65.0f ? 0.0008f * (65.0f - rh) : -0.0015f * (rh - 65.0f));
}

float mq7_calibrate_r0(float v_out, float temperature_c, float humidity_rh) {
    float rs_air = mq7_get_rs(v_out);
    float factor = get_correction_factor(temperature_c, humidity_rh);
    // In aria pulita il rapporto Rs/R0 per CO è circa 26.0-28.0 (dipende dal datasheet)
    // Ma per semplicità molti calcolano R0 in modo che Ratio = 1 in aria pulita
    return rs_air / factor; 
}

float mq7_get_ppm(float v_out, float r0, float t, float rh) {
    if (r0 <= 0) return 0.0f;
    float rs = mq7_get_rs(v_out);
    float factor = get_correction_factor(t, rh);
    float ratio = (rs / factor) / r0;
    
    return MQ7_CO_A * pow(ratio, MQ7_CO_B);
}