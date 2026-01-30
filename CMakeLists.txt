cmake_minimum_required(VERSION 3.13)

include(sdk/pico_sdk_import.cmake)

project(air_quality_freertos C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# --- 1. FIX PER MAC & PORTING RP2040 ---
set(FREERTOS_PORT "GCC_ARM_CM0" CACHE STRING "")

# --- 2. CONFIGURAZIONE TARGET FREERTOS ---
# Definiamo dove si trova FreeRTOSConfig.h (richiesto dal kernel)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/config
)

# --- 3. AGGIUNTA SORGENTI ---
add_executable(air_quality
    src/main.c
    src/sensors/aht20.c
    src/sensors/i2c_ll.c
    src/sensors/mq_adc.c
    src/sensors/mq7_heater.c
    src/processing/aq_filter.c
    src/display/oled.c
    src/networking/mqtt_client.c
    src/rtos/tasks.c
    src/rtos/calibration_store.c
)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/CMakeLists.txt")
    add_subdirectory(FreeRTOS-Kernel)
else()
    message(FATAL_ERROR "FreeRTOS-Kernel non trovato!")
endif()

# --- 4. INCLUDE DIRECTORIES (RISOLVE "No such file or directory") ---
target_include_directories(air_quality PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/config    # Fondamentale per lwipopts.h e FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/src/sensors
    ${CMAKE_CURRENT_LIST_DIR}/src/processing
    ${CMAKE_CURRENT_LIST_DIR}/src/display
    ${CMAKE_CURRENT_LIST_DIR}/src/networking
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos
    # Header del Kernel per i tuoi file .c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/include
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel/portable/GCC/ARM_CM0
)

# --- 5. LINK LIBRERIE ---
target_link_libraries(air_quality
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_timer
    hardware_flash
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    FreeRTOS-Kernel-Heap4
    freertos_config
    m
)

pico_enable_stdio_usb(air_quality 1)
pico_enable_stdio_uart(air_quality 0)
pico_add_extra_outputs(air_quality)