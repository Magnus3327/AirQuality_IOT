cmake_minimum_required(VERSION 3.13)

# Import the Pico SDK
include(pico_sdk_import.cmake)

# Set the target board
set(PICO_BOARD pico_w)

project(AirQuality_IOT C CXX ASM)

# Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Define project sources for clarity
set(PROJECT_SOURCES
    src/main.c
    src/Display/oled.c
    src/Networking/mqtt.c
    src/Sensors/AHT20/aht20.c
    src/Sensors/MQ135/mq135.c
    src/Sensors/MQ7/mq7.c
)

add_executable(AirQuality_IOT ${PROJECT_SOURCES})

# Include paths
# Note: ${CMAKE_CURRENT_LIST_DIR}/src is first so lwipopts.h is found by LwIP
target_include_directories(AirQuality_IOT PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/Display
    ${CMAKE_CURRENT_LIST_DIR}/src/Networking
    ${CMAKE_CURRENT_LIST_DIR}/src/Sensors/AHT20
    ${CMAKE_CURRENT_LIST_DIR}/src/Sensors/MQ135
    ${CMAKE_CURRENT_LIST_DIR}/src/Sensors/MQ7
)

# Link Libraries
target_link_libraries(AirQuality_IOT
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_pwm
    pico_lwip_mqtt
    pico_cyw43_arch_lwip_threadsafe_background
    m # Math library for pow() and log() functions in gas sensors
)

# Enable USB serial output, disable UART (optional, based on your usage)
pico_enable_stdio_usb(AirQuality_IOT 1)
pico_enable_stdio_uart(AirQuality_IOT 0)

# Create extra outputs (.bin, .hex, .uf2)
pico_add_extra_outputs(AirQuality_IOT)