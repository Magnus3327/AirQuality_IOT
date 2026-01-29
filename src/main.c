#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_config.h"
#include "mq_adc.h"
#include "mq7_heater.h"
#include "aq_filter.h"
#include "i2c_sensors.h"
#include "ha_publish.h"

// Task handles for ISR notification
static TaskHandle_t g_task_acq = NULL;

// ---------- IRQ: use hardware alarm 0 ----------
static void alarm0_irq_handler(void) {
    // Clear IRQ
    hw_clear_bits(&timer_hw->intr, 1u << 0);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(g_task_acq, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void alarm0_schedule_next(uint32_t us_from_now) {
    uint64_t target = time_us_64() + us_from_now;
    timer_hw->alarm[0] = (uint32_t) target;
}

// ---------- Telemetry ----------
static movavg_t f_mq135, f_mq7;
static aq_state_t st_mq135 = AQ_OK, st_mq7 = AQ_OK;

// ---------- MQ7 heater cycle state ----------
typedef enum { HEAT_ON, HEAT_OFF } heat_phase_t;
static heat_phase_t heat_phase = HEAT_ON;
static uint32_t heat_phase_left = MQ7_HEAT_ON_SEC;

// ---------- Task: acquisition + processing ----------
static void task_acquisition(void *arg) {
    (void)arg;

    movavg_init(&f_mq135);
    movavg_init(&f_mq7);

    env_data_t env = {0};

    const uint32_t period_us = 1000000u / SAMPLE_HZ;

    for (;;) {
        // Wait for IRQ tick (not a polling delay)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // ----- Heater phase bookkeeping (1 Hz tick assumed) -----
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

        // ----- Read ADC raw -----
        uint16_t mq135_raw = mq_adc_read_raw(ADC_MQ135_CH);
        uint16_t mq7_raw   = mq_adc_read_raw(ADC_MQ7_CH);

        // ----- Filter -----
        uint16_t mq135_f = movavg_push(&f_mq135, mq135_raw);
        uint16_t mq7_f   = movavg_push(&f_mq7, mq7_raw);

        // ----- Evaluate with hysteresis -----
        st_mq135 = eval_with_hyst(mq135_f, st_mq135, TH_MQ135_WARN, TH_MQ135_DANG, HYST);
        st_mq7   = eval_with_hyst(mq7_f,   st_mq7,   TH_MQ7_WARN,   TH_MQ7_DANG,   HYST);

        // ----- Read I2C env (optional; stub returns false until implemented) -----
        (void)i2c_read_environment(&env);

        // ----- Publish (serial for now) -----
        telem_t t = {
            .mq135_raw = mq135_raw, .mq135_filt = mq135_f, .mq135_state = st_mq135,
            .mq7_raw   = mq7_raw,   .mq7_filt   = mq7_f,   .mq7_state   = st_mq7,
            .env       = env,
            .mq7_heater_on = mq7_heater_get()
        };
        ha_publish_send(&t);

        // Re-arm alarm for next period
        alarm0_schedule_next(period_us);
    }
}

// ---------- Main ----------
int main(void) {
    stdio_init_all();
    sleep_ms(1500);
    printf("Air Quality Project (Pico W + FreeRTOS)\n");

    mq_adc_init();
    mq7_heater_init();
    mq7_heater_set(true); // start heater ON

    i2c_sensors_init();
    // Optional scan (enable once for debugging)
    // i2c_scan_bus();

    ha_publish_init();

    // Create tasks
    xTaskCreate(task_acquisition, "acq", 1024, NULL, tskIDLE_PRIORITY + 2, &g_task_acq);

    // Setup IRQ alarm0
    irq_set_exclusive_handler(TIMER_IRQ_0, alarm0_irq_handler);
    irq_set_enabled(TIMER_IRQ_0, true);
    hw_set_bits(&timer_hw->inte, 1u << 0);

    // Arm first tick
    alarm0_schedule_next(1000000u / SAMPLE_HZ);

    // Start scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) { tight_loop_contents(); }
}

