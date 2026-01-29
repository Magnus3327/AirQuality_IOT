cmake_minimum_required(VERSION 3.13)

# Include il file di supporto del Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(env_sensor_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(env_sensor_test
    main.cpp
)

# Abilita l'output su USB seriale
pico_enable_stdio_usb(env_sensor_test 1)
pico_enable_stdio_uart(env_sensor_test 0)

# Link alle librerie: hardware_i2c è necessaria per GP4/GP5
target_link_libraries(env_sensor_test 
    pico_stdlib 
    hardware_i2c
)

# Genera i file uf2, bin, ecc.
pico_add_extra_outputs(env_sensor_test)