#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

// ---------- Board + pin config ----------
// ADC pins (GPIO 26-28) and channels
#define ADC_GPIO_MQ135    26u
#define ADC_GPIO_MQ7      27u
#define ADC_MQ135_CH      0u
#define ADC_MQ7_CH        1u

// MQ7 heater control (GPIO driving transistor/MOSFET)
#define MQ7_HEATER_GPIO   15u

// I2C bus for AHT20 + OLED
#define I2C0_SDA_GPIO     4u
#define I2C0_SCL_GPIO     5u
#define I2C_BAUD_HZ       400000u

// I2C device addresses
#define AHT20_I2C_ADDR    0x38u
#define OLED_I2C_ADDR     0x3Cu

// OLED geometry
#define OLED_W            128u
#define OLED_H            64u

// ---------- Sampling + processing ----------
#define SAMPLE_HZ         1u
#define MOVAVG_LEN        8u
#define HYST              12u

// Thresholds (raw ADC units) - tune for your sensors
#define TH_MQ135_WARN     1200u
#define TH_MQ135_DANG     2000u
#define TH_MQ7_WARN       1200u
#define TH_MQ7_DANG       2000u

// ---------- MQ sensor electrical params ----------
#define ADC_VREF_V        3.3f
#define MQ_SUPPLY_V       5.0f

#define MQ7_RL_OHMS       10000.0f
#define MQ135_RL_OHMS     10000.0f

// Ro must be calibrated in clean air; placeholders below.
#define MQ7_RO_OHMS       10000.0f
#define MQ135_RO_OHMS     10000.0f

// Rs/Ro in clean air (from datasheet curves; tune for your sensor)
#define MQ7_CLEAN_AIR_FACTOR    27.0f
#define MQ135_CLEAN_AIR_FACTOR  3.6f

// Calibration sampling
#define MQ_CALIB_SAMPLES   64u
#define MQ_CALIB_DELAY_MS  100u
#define MQ7_CALIB_SETTLE_MS 30000u

// Curve approximation: ppm = A * (Rs/Ro)^B
// TODO: set these from datasheet/curve fitting for your target gas.
#define MQ7_CURVE_A       99.042f
#define MQ7_CURVE_B       -1.518f
#define MQ135_CURVE_A     10.0f
#define MQ135_CURVE_B     -1.0f

// MQ7 heater cycle timing (seconds)
#define MQ7_HEAT_ON_SEC   60u
#define MQ7_HEAT_OFF_SEC  90u

// ---------- Wi-Fi / MQTT ----------
// Set these before flashing
#define WIFI_SSID         "matteo"
#define WIFI_PASS         "16169999"

#define MQTT_BROKER_IP0   192
#define MQTT_BROKER_IP1   168
#define MQTT_BROKER_IP2   1
#define MQTT_BROKER_IP3   10
#define MQTT_BROKER_PORT  1883

#define MQTT_CLIENT_ID    "pico_air_quality"
#define MQTT_BASE_TOPIC   "homeassistant/sensor/air_quality"
#define MQTT_STATE_TOPIC  "homeassistant/sensor/air_quality/state"
#define MQTT_CMD_TOPIC    "homeassistant/sensor/air_quality/cmd"

// ---------- Telemetry types ----------
typedef struct {
    float temp_c;
    float rh_pct;
} env_data_t;

typedef enum {
    AQ_OK = 0,
    AQ_WARN = 1,
    AQ_DANG = 2
} aq_state_t;

typedef struct {
    uint16_t mq135_raw;
    uint16_t mq135_filt;
    aq_state_t mq135_state;
    float mq135_ppm;
    uint16_t mq7_raw;
    uint16_t mq7_filt;
    aq_state_t mq7_state;
    float mq7_ppm;
    env_data_t env;
    bool mq7_heater_on;
} telem_t;

#endif
