cmake_minimum_required(VERSION 3.13)

# Importazione SDK
include(sdk/pico_sdk_import.cmake)

project(air_quality_freertos C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Definizione eseguibile
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

# Includi le directory dei sorgenti
# Assicurati che src/config contenga sia FreeRTOSConfig.h che lwipopts.h
target_include_directories(air_quality PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/config
    ${CMAKE_CURRENT_LIST_DIR}/src/sensors
    ${CMAKE_CURRENT_LIST_DIR}/src/processing
    ${CMAKE_CURRENT_LIST_DIR}/src/display
    ${CMAKE_CURRENT_LIST_DIR}/src/networking
    ${CMAKE_CURRENT_LIST_DIR}/src/rtos
)

# --- CONFIGURAZIONE FREERTOS ---
# Se stai usando FreeRTOS-Kernel come sottocartella (metodo standard)
add_subdirectory(FreeRTOS-Kernel)

# Link delle librerie
target_link_libraries(air_quality
    # Librerie base SDK
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_flash
    
    # Wi-Fi e Networking (Modalità Threadsafe per FreeRTOS)
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    
    # Kernel FreeRTOS (Librerie generate da add_subdirectory)
    FreeRTOS-Kernel-Heap4
    
    # Librerie matematiche
    m
)

# Opzioni USB/UART
pico_enable_stdio_usb(air_quality 1)
pico_enable_stdio_uart(air_quality 0)

# Generazione file .uf2 per il flash
pico_add_extra_outputs(air_quality)