cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(stazione_ambientale_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(stazione_ambientale
    main.cpp
)

# Abilita output USB per debug parallelo
pico_enable_stdio_usb(stazione_ambientale 1)
pico_enable_stdio_uart(stazione_ambientale 0)

# Link delle librerie hardware necessarie
target_link_libraries(stazione_ambientale 
    pico_stdlib 
    hardware_adc 
    hardware_pwm 
    hardware_i2c
)

pico_add_extra_outputs(stazione_ambientale)