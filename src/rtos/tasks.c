#include "tasks.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "app_config.h"
#include "aq_filter.h"
#include "mq_adc.h"
#include "mq7_heater.h"
#include "aht20.h"
#include "oled.h"
#include "mqtt_client.h"
#include "queues.h"
#include "calibration_store.h"

#include "hardware/irq.h"
#include "hardware/structs/timer.h"
#include "pico/stdlib.h"

static TaskHandle_t g_task_acq = NULL;
static TaskHandle_t g_task_mqtt = NULL;
static QueueHandle_t g_telem_q = NULL;
static volatile bool g_calib_request = false;

static float calibrate_ro(uint8_t adc_ch, float rl_ohms, float clean_air_factor);
static float calibrate_mq7_ro(void);
static void mqtt_cmd_handler(const char *payload, uint16_t len);

// ---------- IRQ: use hardware alarm 0 ----------
static void alarm0_irq_handler(void) {
    hw_clear_bits(&timer_hw->intr, 1u << 0);
    if (g_task_acq == NULL) return;

    BaseType_t hpw = pdFALSE;
    vTaskNotifyGiveFromISR(g_task_acq, &hpw);
    portYIELD_FROM_ISR(hpw);
}

static void alarm0_schedule_next(uint32_t us_from_now) {
    uint64_t target = time_us_64() + us_from_now;
    timer_hw->alarm[0] = (uint32_t)target;
}

// ---------- Telemetry ----------
static movavg_t f_mq135, f_mq7;
static aq_state_t st_mq135 = AQ_OK, st_mq7 = AQ_OK;
static float g_mq135_ro = MQ135_RO_OHMS;
static float g_mq7_ro = MQ7_RO_OHMS;

// ---------- MQ7 heater cycle state ----------
typedef enum { HEAT_ON, HEAT_OFF } heat_phase_t;
static heat_phase_t heat_phase = HEAT_ON;
static uint32_t heat_phase_left = MQ7_HEAT_ON_SEC;

static void task_acquisition(void *arg) {
    (void)arg;

    movavg_init(&f_mq135);
    movavg_init(&f_mq7);

    env_data_t env = {0};
    const uint32_t period_us = 1000000u / SAMPLE_HZ;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (g_calib_request) {
            g_calib_request = false;
            printf("[CAL] Commanded calibration...\n");
            g_mq135_ro = calibrate_ro(ADC_MQ135_CH, MQ135_RL_OHMS, MQ135_CLEAN_AIR_FACTOR);
            g_mq7_ro = calibrate_mq7_ro();
            (void)calibration_store_save(g_mq7_ro, g_mq135_ro);
            printf("[CAL] Done. MQ135 Ro=%.2f MQ7 Ro=%.2f\n", g_mq135_ro, g_mq7_ro);
        }

        if (heat_phase_left > 0) heat_phase_left--;
        if (heat_phase_left == 0) {
            if (heat_phase == HEAT_ON) {
                heat_phase = HEAT_OFF;
                heat_phase_left = MQ7_HEAT_OFF_SEC;
                mq7_heater_set(false);
            } else {
                heat_phase = HEAT_ON;
                heat_phase_left = MQ7_HEAT_ON_SEC;
                mq7_heater_set(true);
            }
        }

        uint16_t mq135_raw = mq_adc_read_raw(ADC_MQ135_CH);
        uint16_t mq7_raw   = mq_adc_read_raw(ADC_MQ7_CH);

        uint16_t mq135_f = movavg_push(&f_mq135, mq135_raw);
        uint16_t mq7_f   = movavg_push(&f_mq7, mq7_raw);

        st_mq135 = eval_with_hyst(mq135_f, st_mq135, TH_MQ135_WARN, TH_MQ135_DANG, HYST);
        st_mq7   = eval_with_hyst(mq7_f,   st_mq7,   TH_MQ7_WARN,   TH_MQ7_DANG,   HYST);

        (void)aht20_read(&env);

        telem_t t = {
            .mq135_raw = mq135_raw, .mq135_filt = mq135_f, .mq135_state = st_mq135,
            .mq135_ppm = 0.0f,
            .mq7_raw   = mq7_raw,   .mq7_filt   = mq7_f,   .mq7_state   = st_mq7,
            .mq7_ppm   = 0.0f,
            .env       = env,
            .mq7_heater_on = mq7_heater_get()
        };

        float mq135_rs = mq_calc_rs_ohms(mq135_f, MQ135_RL_OHMS, ADC_VREF_V, MQ_SUPPLY_V);
        float mq7_rs   = mq_calc_rs_ohms(mq7_f,   MQ7_RL_OHMS,   ADC_VREF_V, MQ_SUPPLY_V);
        float mq135_ratio = mq_calc_ratio(mq135_rs, g_mq135_ro);
        float mq7_ratio   = mq_calc_ratio(mq7_rs,   g_mq7_ro);
        t.mq135_ppm = mq_calc_ppm(mq135_ratio, MQ135_CURVE_A, MQ135_CURVE_B);
        t.mq7_ppm   = mq_calc_ppm(mq7_ratio,   MQ7_CURVE_A,   MQ7_CURVE_B);

        if (g_telem_q) {
            if (xQueueSend(g_telem_q, &t, 0) != pdTRUE) {
                telem_t tmp;
                (void)xQueueReceive(g_telem_q, &tmp, 0);
                (void)xQueueSend(g_telem_q, &t, 0);
            }
        }

        oled_show_basic(&t);

        alarm0_schedule_next(period_us);
    }
}

static float calibrate_mq7_ro(void) {
    // MQ7 requires a heat cycle; use low phase for sensing
    mq7_heater_set(true);
    for (uint32_t i = 0; i < MQ7_HEAT_ON_SEC; ++i) {
        sleep_ms(1000);
    }
    mq7_heater_set(false);
    sleep_ms(MQ7_CALIB_SETTLE_MS);
    float ro = calibrate_ro(ADC_MQ7_CH, MQ7_RL_OHMS, MQ7_CLEAN_AIR_FACTOR);
    heat_phase = HEAT_OFF;
    heat_phase_left = MQ7_HEAT_OFF_SEC;
    return ro;
}

static void mqtt_cmd_handler(const char *payload, uint16_t len) {
    if (payload == NULL || len == 0) return;
    if (strstr(payload, "CALIBRATE") || strstr(payload, "calibrate")) {
        g_calib_request = true;
    }
}

static float calibrate_ro(uint8_t adc_ch, float rl_ohms, float clean_air_factor) {
    float sum_rs = 0.0f;
    for (uint32_t i = 0; i < MQ_CALIB_SAMPLES; ++i) {
        uint16_t raw = mq_adc_read_raw(adc_ch);
        float rs = mq_calc_rs_ohms(raw, rl_ohms, ADC_VREF_V, MQ_SUPPLY_V);
        sum_rs += rs;
        sleep_ms(MQ_CALIB_DELAY_MS);
    }
    float avg_rs = sum_rs / (float)MQ_CALIB_SAMPLES;
    if (clean_air_factor <= 0.1f) clean_air_factor = 1.0f;
    return avg_rs / clean_air_factor;
}

static void task_mqtt(void *arg) {
    (void)arg;
    telem_t t = {0};

    for (;;) {
        if (g_telem_q && xQueueReceive(g_telem_q, &t, portMAX_DELAY) == pdTRUE) {
            if (mqtt_client_is_ready()) {
                (void)mqtt_client_publish(&t);
            } else {
                printf("[MQTT] not ready\n");
            }
        }
    }
}

QueueHandle_t queues_get_telem(void) {
    return g_telem_q;
}

void tasks_init(void) {
    g_telem_q = xQueueCreate(TELEM_QUEUE_LEN, sizeof(telem_t));

    mq_adc_init();
    mq7_heater_init();
    mq7_heater_set(true);
    aht20_init();
    oled_init();

    if (!calibration_store_load(&g_mq7_ro, &g_mq135_ro)) {
        printf("[CAL] MQ135 calibrating...\n");
        g_mq135_ro = calibrate_ro(ADC_MQ135_CH, MQ135_RL_OHMS, MQ135_CLEAN_AIR_FACTOR);
        printf("[CAL] MQ135 Ro=%.2f\n", g_mq135_ro);
        printf("[CAL] MQ7 calibrating...\n");
        g_mq7_ro = calibrate_mq7_ro();
        printf("[CAL] MQ7 Ro=%.2f\n", g_mq7_ro);
        (void)calibration_store_save(g_mq7_ro, g_mq135_ro);
    } else {
        printf("[CAL] Loaded Ro from flash. MQ135=%.2f MQ7=%.2f\n", g_mq135_ro, g_mq7_ro);
    }

    if (!mqtt_client_init()) {
        printf("[MQTT] init failed, continuing without network\n");
    }
    mqtt_client_set_cmd_callback(mqtt_cmd_handler);

    xTaskCreate(task_acquisition, "acq", 1024, NULL, tskIDLE_PRIORITY + 2, &g_task_acq);
    xTaskCreate(task_mqtt, "mqtt", 1536, NULL, tskIDLE_PRIORITY + 1, &g_task_mqtt);

    irq_set_exclusive_handler(TIMER_IRQ_0, alarm0_irq_handler);
    irq_set_enabled(TIMER_IRQ_0, true);
    hw_set_bits(&timer_hw->inte, 1u << 0);
    alarm0_schedule_next(1000000u / SAMPLE_HZ);
}

void tasks_start(void) {
    vTaskStartScheduler();
}
