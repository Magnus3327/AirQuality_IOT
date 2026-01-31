#include "mq135.h"
#include "hardware/adc.h"
#include <math.h>

/* ================= CONFIGURAZIONE HARDWARE ================= */

// Tensione di loop del sensore MQ
#define MQ135_VC              5.0f

// Resistenza di carico (METTI IL VALORE REALE CHE USI)
#define MQ135_RL              738.0f   // 1kΩ per il mio sensore

// Rapporto Rs/R0 in aria pulita (datasheet MQ135)
#define RATIO_CLEAN_AIR       1.0f

// Limiti fisici per evitare divergenze
#define RATIO_MIN             0.1f
#define RATIO_MAX             10.0f

// Limite IAQ per Home Assistant
#define IAQ_MAX               500.0f


/* ================= CORREZIONE T / RH ================= */

// Modello empirico semplice (sufficiente per MQ)
static float get_correction_factor(float t, float rh) {
    return (-0.0035f * t + 1.0714f)
         + (rh <= 60.0f ? 0.001f * (60.0f - rh)
                        : -0.002f * (rh - 60.0f));
}


/* ================= API PUBBLICA ================= */

void mq135_init(int adc_pin) {
    adc_gpio_init(adc_pin);     
}

float mq135_get_voltage(void) {
    uint16_t raw = adc_read();

    // ADC RP2040: 12 bit, 0–3.3V
    float v_adc = (raw * 3.3f) / 4095.0f;

    // Partitore 2/3 → tensione reale sensore
    return v_adc * 1.5f;
}

float mq135_get_rs(float v_out) {
    if (v_out < 0.1f) {
        // sensore saturato o scollegato
        return 1e6f;
    }

    // Modello elettrico MQ
    return ((MQ135_VC * MQ135_RL) / v_out) - MQ135_RL;
}


/* ================= CALIBRAZIONE ================= */

float mq135_calibrate_r0(float v_out, float temperature_c, float humidity_rh) {
    float rs_air = mq135_get_rs(v_out);
    float factor = get_correction_factor(temperature_c, humidity_rh);

    // R0 fisicamente corretto
    return rs_air / (RATIO_CLEAN_AIR * factor);
}
