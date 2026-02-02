cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(AirQuality_IOT C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(AirQuality_IOT
    src/main.c
    src/Display/oled.c
    src/Networking/mqtt.c
    src/Sensors/AHT20/aht20.c
    src/Sensors/MQ135/mq135.c
    src/Sensors/MQ7/mq7.c
)

# Include paths (src PRIMA per lwipopts.h)
target_include_directories(AirQuality_IOT PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/Display
    ${CMAKE_CURRENT_LIST_DIR}/src/Networking
    ${CMAKE_CURRENT_LIST_DIR}/src/Sensors/AHT20
    ${CMAKE_CURRENT_LIST_DIR}/src/Sensors/MQ135
    ${CMAKE_CURRENT_LIST_DIR}/src/Sensors/MQ7
)

# === LINK CORRETTO ===
target_link_libraries(AirQuality_IOT
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_pwm
    pico_lwip_mqtt
    pico_cyw43_arch_lwip_threadsafe_background

    m
)

pico_enable_stdio_usb(AirQuality_IOT 1)
pico_add_extra_outputs(AirQuality_IOT)