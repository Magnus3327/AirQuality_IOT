
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

- MQ sensors provide analog signals sampled via ADC
- Environmental sensors are read via I2C bus
- Signal stabilization and filtering are performed in software
- Air quality is classified using threshold-based logic
- Data is transmitted wirelessly to a Home Assistant server

---

## Hardware / Software Interaction

### Analog Sensors
- MQ135 and MQ7 analog outputs are scaled using resistive voltage dividers
- Signals are sampled using the Pico ADC

### MQ7 Heater Control
- Heater powered at 5V
- Controlled via GPIO using a 2N2222 transistor (low-side switching)
- Heater cycle managed in software using timers

### I2C Sensors
- AHT20 and BMP280 connected on the same I2C bus
- Environmental data used for compensation and validation

---

## Software Architecture

The firmware is written in **bare-metal C** and organized into modular components:

- **ADC driver** – analog signal acquisition
- **I2C driver** – environmental sensor communication
- **Timer and interrupt management** – periodic sampling and heater control
- **Data processing module** – filtering, averaging, thresholding
- **Communication module** – wireless data transmission

Timing is handled using **hardware timers** rather than blocking delays to ensure deterministic behavior.

---

## Data Processing
- Sensor warm-up handling
- Software-based filtering (moving average)
- Threshold evaluation with hysteresis
- Air quality classification into discrete states

Signal stabilization is implemented entirely in software due to the slow dynamics of MQ sensors.

---

## IoT Communication
- Wireless communication using Raspberry Pi Pico W
- Periodic data transmission to a Home Assistant server
- System designed to be easily extensible to other IoT platforms

---

## Testing and Validation
The system was tested through:
- Sensor warm-up and stabilization tests
- Noise evaluation on analog signals
- Environmental variation tests
- Verification of software filtering effectiveness

Testing confirmed reliable and stable behavior under different conditions.

---

## Repository Structure

---

## How to Build and Run
1. Connect the hardware according to the schematic
2. Compile the firmware using the Pico SDK toolchain
3. Flash the binary to the Raspberry Pi Pico W
4. Power the system and monitor data on the server

---

## Team Members and Contributions

| Name | Contribution |
|-----|--------------|
| **Alessandro Gremes** | Hardware design, sensor interfacing |
| **Paolo Sarcletti** | Firmware development, timers and interrupts |
| **Matteo Miglio** | Data processing and filtering algorithms |
| **Alessandro Turri** | IoT communication, testing and validation |

All team members have full knowledge of the system and are responsible for the entire project.

---

## Conclusions and Future Work
The project demonstrates a complete embedded IoT system with low-level hardware interaction and software-based intelligence.

Possible future improvements include:
- Additional environmental sensors
- Power consumption optimization
- Advanced calibration techniques
- Extended cloud integration

---

## Links
- GitHub Repository: *(to be added)*
- Presentation Slides: *(to be added)*
- Demo Video: *(to be added)*


