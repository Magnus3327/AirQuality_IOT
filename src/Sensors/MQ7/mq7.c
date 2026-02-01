#include "mq7.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"  // Necessario per mq7_init e mq7_set_heater
#include "hardware/gpio.h"
#include <math.h>

/* ================= PARAMETRI DATASHEET MQ-7 ================= 
 * Gas: Monossido di Carbonio (CO)
 * B: log10(0.6) / log10(3) = -0.465
 * A: 100
 * ============================================================ */

#define MQ7_VC      5.0f    
#define MQ7_RL      830.0f  
#define MQ7_A_CO    100.0f
#define MQ7_B_CO    -0.465f

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
        pwm_set_gpio_level(_pwm_pin, 255); // 100% duty cycle (5V)
    } else {
        pwm_set_gpio_level(_pwm_pin, 77);  // ~30% duty cycle (1.5V)
    }
}

float mq7_get_voltage(void) {
    uint32_t sum = 0;
    const int samples = 64;
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
    }
    float avg_raw = (float)sum / (float)samples;
    return ((avg_raw * 3.3f) / 4095.0f) * 1.5f;
}

float mq7_get_rs(float v_out) {
    if (v_out <= 0.1f) return 1000000.0f;
    return ((MQ7_VC * MQ7_RL) / v_out) - MQ7_RL;
}

// AGGIORNATO: Firma corretta con temp e hum
float mq7_calibrate_r0(float v_out, float temperature_c, float humidity_rh) {
    float rs_air = mq7_get_rs(v_out);
    // Nota: temperature_c e humidity_rh possono essere usati qui per compensazione
    return rs_air / 5.0f;
}

// AGGIORNATO: Firma corretta con temp e hum
float mq7_get_ppm(float v_out, float r0, float temperature_c, float humidity_rh) {
    if (r0 <= 0 || v_out <= 0.1f) return 0.0f;
    
    float rs = mq7_get_rs(v_out);
    float ratio = rs / r0;
    
    // Formula: PPM = A * (Ratio)^B
    return MQ7_A_CO * pow(ratio, MQ7_B_CO);
}