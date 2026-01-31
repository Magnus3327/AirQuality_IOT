cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(AirQuality_IOT C CXX ASM)
pico_sdk_init()

add_executable(main
    src/main.c
    src/Display/oled.c
    src/Sensors/AHT20/aht20.c
    src/Sensors/MQ135/mq135.c
    src/Sensors/MQ7/mq7.c
    # src/Networking/mqtt.c  <-- Commentato per ora!
)

# Questo dice al compilatore di guardare dentro le cartelle per gli header
target_include_directories(main PRIVATE 
    src/Display
    src/Sensors/AHT20
    src/Sensors/MQ135
    src/Sensors/MQ7
    src/Networking
)

target_link_libraries(main 
    pico_stdlib 
    hardware_adc 
    hardware_i2c
    m
    hardware_pwm
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)