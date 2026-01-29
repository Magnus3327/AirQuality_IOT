#pragma once
#include <stdint.h>

// ---------- Pins ----------
#define PIN_I2C_SDA        4   // GP4
#define PIN_I2C_SCL        5   // GP5

#define PIN_MQ7_HEATER_EN  15  // GP15 -> base resistor -> 2N2222

#define ADC_MQ135_CH       0   // ADC0 = GP26
#define ADC_MQ7_CH         1   // ADC1 = GP27

// ---------- Sampling ----------
#define SAMPLE_HZ          1u     // 1 Hz sampling loop (MQ sensors are slow)
#define MQ_FILTER_N        16u    // moving average window

// ---------- MQ7 heater cycle (example, simplified) ----------
// In real MQ-7 usage, heater cycles are important. For demo: implement ON/OFF phases.
#define MQ7_HEAT_ON_SEC    60u
#define MQ7_HEAT_OFF_SEC   90u

// ---------- Air quality thresholds (raw ADC scaled 0..4095) ----------
// You will tune these empirically during testing.
#define TH_MQ135_WARN      1800u
#define TH_MQ135_DANG      2600u

#define TH_MQ7_WARN        1600u
#define TH_MQ7_DANG        2400u

// Hysteresis (raw ADC counts)
#define HYST              80u

