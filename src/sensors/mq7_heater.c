#include "mq7_heater.h"

#include "app_config.h"

#include "hardware/regs/io_bank0.h"
#if __has_include("hardware/regs/pads_bank0.h")
#include "hardware/regs/pads_bank0.h"
#else
#include "hardware/regs/padsbank0.h"
#endif
#include "hardware/structs/io_bank0.h"
#if __has_include("hardware/structs/pads_bank0.h")
#include "hardware/structs/pads_bank0.h"
#else
#include "hardware/structs/padsbank0.h"
#endif
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

static bool g_heater_on = false;

static void gpio_sio_output_init(uint32_t gpio) {
    io_bank0_hw->io[gpio].ctrl =
        (GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB);

    padsbank0_hw->io[gpio] =
        PADS_BANK0_GPIO0_IE_BITS;

    sio_hw->gpio_oe_set = (1u << gpio);
}

void mq7_heater_init(void) {
    gpio_sio_output_init(MQ7_HEATER_GPIO);
    mq7_heater_set(false);
}

void mq7_heater_set(bool on) {
    g_heater_on = on;
    if (on) {
        sio_hw->gpio_set = (1u << MQ7_HEATER_GPIO);
    } else {
        sio_hw->gpio_clr = (1u << MQ7_HEATER_GPIO);
    }
}

bool mq7_heater_get(void) {
    return g_heater_on;
}
