#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Parametri di Sistema */
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      133000000
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configCHECK_FOR_STACK_OVERFLOW          2

/* Gestione dei Tick (Risolve il tuo primo errore) */
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS

/* MPU e Memoria (Risolve il secondo errore) */
#define configENABLE_MPU                        0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configTOTAL_HEAP_SIZE                   (32 * 1024)  // 32KB heap per FreeRTOS

/* Co-routines e Task */
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_EVENT_GROUPS                  1
#define configQUEUE_REGISTRY_SIZE               8

/* Software Timer */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* Funzioni API da includere */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xSemaphoreGetMutexHolder        1
#define INCLUDE_xEventGroupSetBitsFromISR       1
#define INCLUDE_xTimerPendFunctionCall          1

/* Mapping degli Interrupt (Specifico per Pico SDK) */
#define vPortSVCHandler                         isr_svcall
#define xPortPendSVHandler                      isr_pendsv
#define xPortSysTickHandler                     isr_systick

/* Workaround per portCHECK_IF_IN_ISR - definito in portmacro.h ma necessario qui */
#ifndef portCHECK_IF_IN_ISR
#define portCHECK_IF_IN_ISR()                   \
    ( {                                        \
        uint32_t ulIPSR;                       \
        __asm volatile ( "mrs %0, IPSR" : "=r" ( ulIPSR )::); \
        ( ( uint8_t ) ulIPSR ) > 0; } )
#endif

#endif /* FREERTOS_CONFIG_H */