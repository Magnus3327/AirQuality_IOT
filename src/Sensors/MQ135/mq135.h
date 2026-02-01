#ifndef MQ135_H
#define MQ135_H

void  mq135_init(int adc_pin);
float mq135_get_voltage(void);
float mq135_get_rs(float v_out);
float mq135_calibrate_r0(float v_out);
float mq135_get_ppm(float v_out, float r0);

#endif