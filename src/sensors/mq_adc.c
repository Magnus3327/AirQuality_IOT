#include "mq_adc.h"

#include "app_config.h"

#include "hardware/regs/io_bank0.h"
#if __has_include("hardware/regs/pads_bank0.h")
#include "hardware/regs/pads_bank0.h"
#else
#include "hardware/regs/padsbank0.h"
#endif
#include "hardware/regs/resets.h"
#include "hardware/regs/adc.h"
#include "hardware/structs/adc.h"
#include "hardware/structs/io_bank0.h"
#if __has_include("hardware/structs/pads_bank0.h")
#include "hardware/structs/pads_bank0.h"
#else
#include "hardware/structs/padsbank0.h"
#endif
#include "hardware/structs/resets.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"
#include "hardware/clocks.h"
#include <math.h>

// ADC is 12-bit on RP2040
#define ADC_COUNTS 4095.0f

static void adc_gpio_to_analog(uint32_t gpio) {
    // Disable output, pull-ups, and digital input enable on ADC GPIOs
    padsbank0_hw->io[gpio] = PADS_BANK0_GPIO0_IE_BITS * 0u; // IE = 0
    padsbank0_hw->io[gpio] &= ~(PADS_BANK0_GPIO0_PUE_BITS | PADS_BANK0_GPIO0_PDE_BITS);
    // Function select to NULL (disable digital), use SIO disabled
    io_bank0_hw->io[gpio].ctrl =
        (GPIO_FUNC_NULL << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);
    (void)sio_hw; // keep include for direct register usage elsewhere
}

void mq_adc_init(void) {
    // Reset ADC
    resets_hw->reset |= RESETS_RESET_ADC_BITS;
    resets_hw->reset &= ~RESETS_RESET_ADC_BITS;
    while ((resets_hw->reset_done & RESETS_RESET_ADC_BITS) == 0u) {}

    // Configure GPIOs for analog
    adc_gpio_to_analog(ADC_GPIO_MQ135);
    adc_gpio_to_analog(ADC_GPIO_MQ7);

    // Enable ADC
    adc_hw->cs = ADC_CS_EN_BITS;

    // Set clock divider (integer part only, keep ADC clock below 48MHz)
    uint32_t div_int = clock_get_hz(clk_adc) / 48000000u;
    adc_hw->div = (div_int << ADC_DIV_INT_LSB);
}

uint16_t mq_adc_read_raw(uint8_t channel) {
    adc_hw->cs = (adc_hw->cs & ~ADC_CS_AINSEL_BITS) |
                 ((uint32_t)channel << ADC_CS_AINSEL_LSB) |
                 ADC_CS_EN_BITS;

    adc_hw->cs |= ADC_CS_START_ONCE_BITS;
    while ((adc_hw->cs & ADC_CS_READY_BITS) == 0u) {}
    return (uint16_t)adc_hw->result;
}

float mq_calc_rs_ohms(uint16_t adc_raw, float rl_ohms, float vref_v, float vcc_v) {
    float vout = (adc_raw / ADC_COUNTS) * vref_v;
    if (vout < 0.01f) vout = 0.01f;
    if (vout > (vcc_v - 0.01f)) vout = vcc_v - 0.01f;
    return rl_ohms * (vcc_v / vout - 1.0f);
}

float mq_calc_ratio(float rs_ohms, float ro_ohms) {
    if (ro_ohms < 1.0f) return 0.0f;
    return rs_ohms / ro_ohms;
}

float mq_calc_ppm(float ratio, float a, float b) {
    if (ratio <= 0.0f) return 0.0f;
    return a * powf(ratio, b);
}
