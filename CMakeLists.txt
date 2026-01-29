cmake_minimum_required(VERSION 3.13)

include(sdk/pico_sdk_import.cmake)

project(air_quality_freertos C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(air_quality
    src/main.c
    src/sensors/aht20.c
    src/sensors/mq_adc.c
    src/sensors/mq7_heater.c
    src/processing/aq_filter.c
    src/display/oled.c
    src/networking/mqtt_client.c
    src/rtos/tasks.c
)

target_include_directories(air_quality PRIVATE
    src
    src/config
    src/sensors
    src/processing
    src/display
    src/networking
    src/rtos
)

# Pico SDK libs
target_link_libraries(air_quality
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_timer
)

# FreeRTOS (you must provide these in your project)
# Example if you add FreeRTOS-Kernel as subdir:
# add_subdirectory(FreeRTOS-Kernel)
# target_link_libraries(air_quality freertos_kernel)

pico_enable_stdio_usb(air_quality 1)
pico_enable_stdio_uart(air_quality 0)

pico_add_extra_outputs(air_quality)
target_link_libraries(air_quality
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_timer
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
)
