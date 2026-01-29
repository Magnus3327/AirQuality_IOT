# Embedded Air Quality Monitoring System  
**Embedded Software for the Internet of Things**

This repository contains all the material related to our project developed for the **Embedded Software for the Internet of Things** course, held by **Prof. Kasim Sinan Yildirim** at the University of Trento.

The goal of the project is to design and implement a **real embedded sensing and IoT system** capable of monitoring **indoor air quality and carbon monoxide concentration**, while also measuring environmental parameters such as temperature, humidity and pressure.  
The system performs **low-level hardware interaction**, software-based data processing, and wireless communication with a home automation server.

---

## Table of Contents
- [About the project](#about-the-project)
- [How it works](#how-it-works)
  - [System working flow](#system-working-flow)
  - [General features](#general-features)
- [Project documentation](#project-documentation)
  - [Project structure](#project-structure)
  - [Hardware Architecture](#hardware-architecture)
  - [Software Architecture](#software-architecture)
- [Configuration](#configuration)
  - [Prerequisites](#prerequisites)
  - [1. Clone the repository](#1-clone-the-repository)
  - [2. Setting up the circuit](#2-setting-up-the-circuit)
    - [Hardware components](#hardware-components)
    - [Pinout](#pinout)
- [Run the project](#run-the-project)
- [Testing](#testing)
- [Conclusions](#conclusions)
- [Additional resources](#additional-resources)

---

## About the project

The project has been developed by the following students:

| Name | Work made |
| -- | -- |
| **Alessandro Gremes** | Hardware design, sensor interfacing, MQ sensors integration |
| **Paolo Sarcletti** | Firmware development, timers and interrupt handling |
| **Matteo Miglio** | Data processing, filtering algorithms, air quality evaluation |
| **Alessandro Turri** | IoT communication, system testing and validation |

*Every team member actively contributed to the design choices and has full knowledge of the entire system.  
Several components were developed using pair programming and collaborative debugging sessions.*

The project is based on the concepts covered during the course, including:
- Low-level embedded programming in C  
- Hardware/software interaction  
- Interrupt-driven design  
- Software modularity  
- Testing and validation strategies  

---

## How it works

### System working flow

MQ135 / MQ7 ──► ADC ──► Data Filtering ──► Air Quality Evaluation
AHT20 / BMP280 ──► I2C ───────────────────────────────────────┘
↓
Wireless IoT Communication

1. **Data acquisition**
   - MQ135 and MQ7 provide analog signals related to air pollution and CO concentration.
   - AHT20 and BMP280 provide temperature, humidity and pressure via I2C.

2. **Signal processing**
   - Analog signals are scaled and sampled through the Pico ADC.
   - Software-based filtering is applied to reduce noise and improve stability.

3. **Air quality evaluation**
   - Threshold-based logic with hysteresis is used to classify air quality states.

4. **IoT communication**
   - Processed data is transmitted wirelessly to a home automation server.

---

### General features
- Continuous air quality and CO monitoring  
- Environmental compensation using temperature and humidity data  
- Software-controlled heater cycle for MQ7  
- Modular and extensible firmware architecture  
- Wireless data reporting  

---

## Project documentation

### Project structure

Embedded-Air-Quality-System/
├── src/ # Source files
├── include/ # Header files
├── docs/ # Schematics and documentation
├── README.md

---

### Hardware Architecture

- **Raspberry Pi Pico W** used as main microcontroller  
- MQ135 and MQ7 powered at 5V  
- Analog outputs scaled using resistive voltage dividers  
- MQ7 heater controlled via GPIO and 2N2222 transistor  
- AHT20 and BMP280 connected on a shared I2C bus  
- Common ground between all components  

---

### Software Architecture

The firmware is written in **bare-metal C** and structured into independent modules:

- ADC driver for analog sensors  
- I2C driver for environmental sensors  
- Timer-based scheduling and interrupt handling  
- Data processing and filtering logic  
- Communication module for IoT integration  

Blocking delays are avoided in favor of **timer-driven execution** to ensure deterministic behavior.

---

## Configuration

### Prerequisites
- Raspberry Pi Pico SDK  
- CMake toolchain  
- USB cable for flashing the board  

---

### 1. Clone the repository

```bash
git clone <repository-url>
```
## 2. Setting up the circuit

### Hardware components
- Raspberry Pi Pico W  
- MQ135 air quality sensor  
- MQ7 CO sensor  
- AHT20 temperature/humidity sensor  
- BMP280 pressure sensor  
- HW131 power board  
- 2N2222 transistor  
- Resistors for voltage dividers  

### Pinout (main connections)

| Component | Pico GPIO |
| -- | -- |
| MQ135 A0 | GP26 (ADC0) |
| MQ7 A0 | GP27 (ADC1) |
| MQ7 Heater Control | GP15 |
| I2C SDA | GP4 |
| I2C SCL | GP5 |

---

## Run the project

1. Build the firmware using the Pico SDK  
2. Flash the binary to the Raspberry Pi Pico W  
3. Power the system  
4. Monitor data via serial output or IoT server  

---

## Testing

A bottom-up testing strategy was adopted:
- Individual sensor testing  
- Validation of ADC and I2C communication  
- Verification of MQ7 heater control cycle  
- Evaluation of filtering effectiveness  
- End-to-end system validation  

---

## Conclusions

The project successfully demonstrates a **complete embedded IoT system**, integrating sensing, processing and communication.  
It highlights the importance of low-level control, modular design, and software-based signal stabilization.

Possible future improvements include:
- Additional gas sensors  
- Power consumption optimization  
- Advanced calibration techniques  
- Extended cloud integration  

---

## Additional resources
- Presentation slides *(to be added)*  
- Demo video *(to be added)*  
