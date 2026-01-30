#include <stdio.h>

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tasks.h"

/* Richiesto da configCHECK_FOR_STACK_OVERFLOW == 2 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    printf("STACK OVERFLOW in task %s\n", pcTaskName ? pcTaskName : "?");
    for (;;) { tight_loop_contents(); }
}

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
