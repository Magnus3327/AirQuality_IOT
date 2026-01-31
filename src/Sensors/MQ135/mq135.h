#ifndef MQ135_H
#define MQ135_H

#include <stdint.h>

// Inizializzazione ADC / GPIO
void mq135_init(int adc_pin);

// Lettura tensione reale all'uscita del sensore (prima del partitore)
float mq135_get_voltage(void);

// Calcolo resistenza Rs del sensore
float mq135_get_rs(float v_out);

// Calibrazione R0 (da fare UNA volta, aria normale, dopo 48h)
float mq135_calibrate_r0(float v_out, float temperature_c, float humidity_rh);

#endif // MQ135_H