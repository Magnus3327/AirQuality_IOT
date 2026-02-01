#include "mq135.h"
#include "hardware/adc.h"
#include <math.h>

/* ================= PARAMETRI DATASHEET MATTEO ================= 
 * Gas Principale: Ammoniaca (NH3)
 * Alpha (Pendenza): 0.6 (Rapporto R100ppm/R50ppm)
 * B (Esponente): log10(0.6) / log10(100/50) = -0.737
 * A (Costante): 100 (Assumendo Rs/R0 = 1 a 100ppm per calibrazione)
 * ============================================================== */

#define MQ135_VC            5.0f    // Tensione Loop
#define MQ135_RL            985.0f  // Resistenza di carico fisica (Ohm)
#define MQ135_A_NH3         100.0f  
#define MQ135_B_NH3         -0.737f 

void mq135_init(int adc_pin) {
    adc_gpio_init(adc_pin);
}

// Lettura con oversampling per stabilità
float mq135_get_voltage(void) {
    uint32_t sum = 0;
    const int samples = 64;
    for (int i = 0; i < samples; i++) {
        sum += adc_read();
    }
    float avg_raw = (float)sum / (float)samples;
    // Conversione ADC 12-bit (0-3.3V) + Partitore 1.5x
    return ((avg_raw * 3.3f) / 4095.0f) * 1.5f;
}

// Calcolo Rs (Resistenza del sensore)
float mq135_get_rs(float v_out) {
    if (v_out <= 0.1f) return 1000000.0f; // Evita divisione per zero
    return ((MQ135_VC * MQ135_RL) / v_out) - MQ135_RL;
}

// Calibrazione R0 in aria pulita
float mq135_calibrate_r0(float v_out) {
    float rs_air = mq135_get_rs(v_out);
    /* Da datasheet: Rs(aria)/Rs(100ppm NH3) >= 5. 
       Usiamo 5.0 come fattore di scala per aria pulita */
    return rs_air / 5.0f;
}

// Algoritmo Finale PPM NH3
float mq135_get_ppm(float v_out, float r0) {
    if (r0 <= 0 || v_out <= 0.1f) return 0.0f;
    
    float rs = mq135_get_rs(v_out);
    float ratio = rs / r0;
    
    // Modello matematico: PPM = A * (Ratio)^B
    return MQ135_A_NH3 * pow(ratio, MQ135_B_NH3);
}