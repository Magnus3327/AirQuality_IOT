
# Embedded Air Quality Monitoring System

## Course
**Embedded Software for the Internet of Things**  
University of Trento

## Project Overview
This project implements a complete embedded sensing and IoT system for **indoor air quality monitoring**.  
The system measures air pollution, carbon monoxide concentration, temperature, humidity, and pressure, and publishes the processed data to a home automation server.

The project focuses on **low-level embedded programming**, direct hardware interaction, and software-based data processing, without relying on high-level libraries.

---

## Hardware Platform
- **Raspberry Pi Pico W**
- **MQ135** – Air quality sensor (analog)
- **MQ7** – Carbon monoxide sensor (analog, controlled heater)
- **AHT20** – Temperature and humidity sensor (I2C)
- **BMP280** – Pressure sensor (I2C)
- **HW131 power board** – External power supply
- **2N2222 transistor** – MQ7 heater control

---

## System Architecture

### Basic Working Scheme
