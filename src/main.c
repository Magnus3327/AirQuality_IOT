#include <stdio.h>

#include "pico/stdlib.h"
#include "tasks.h"

// ---------- Main ----------
int main(void) {
    stdio_init_all();
    sleep_ms(1500);
    printf("Air Quality Project (Pico W + FreeRTOS + MQTT)\n");

    tasks_init();
    tasks_start();

    // Should never reach here
    while (1) { tight_loop_contents(); }
}
