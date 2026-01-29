cmake_minimum_required(VERSION 3.13)

# Include il file di supporto del Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(mq135_test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(mq135_test
    main.cpp
)

# Abilita l'output USB seriale e disabilita quello UART standard (opzionale)
pico_enable_stdio_usb(mq135_test 1)
pico_enable_stdio_uart(mq135_test 0)

# Collega le librerie necessarie (hardware_adc è fondamentale qui)
target_link_libraries(mq135_test 
    pico_stdlib 
    hardware_adc
)

# Crea i file di output (bin, elf, uf2)
pico_add_extra_outputs(mq135_test)