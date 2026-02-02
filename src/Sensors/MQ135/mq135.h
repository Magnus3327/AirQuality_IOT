#ifndef MQ135_H
#define MQ135_H

/**
 * @brief Initialize the MQ-135 sensor ADC pin.
 * @param adc_pin GPIO pin connected to the sensor Analog Output (AO)
 */
void mq135_init(int adc_pin);

/**
 * @brief Read raw voltage from the sensor with oversampling.
 * @return Compensated voltage in Volts (V)
 */
float mq135_get_voltage(void);

/**
 * @brief Calculate the internal sensor resistance (Rs).
 * @param v_out Measured output voltage
 * @return Current Rs value in Ohms
 */
float mq135_get_rs(float v_out);

/**
 * @brief Calibrate R0 in clean air based on the current environment.
 * @param v_out Measured voltage in clean air
 * @param t Ambient temperature (°C)
 * @param h Ambient humidity (%RH)
 * @return Calculated R0 resistance value
 */
float mq135_calibrate_r0(float v_out, float t, float h);

/**
 * @brief Calculate gas concentration in PPM with environmental compensation.
 * @param v_out Current output voltage
 * @param r0 Calibrated resistance in clean air
 * @param t Ambient temperature (°C)
 * @param h Ambient humidity (%RH)
 * @return Estimated gas concentration in PPM
 */
float mq135_get_ppm(float v_out, float r0, float t, float h);

#endif