# 🌫️ Air Quality Monitoring Station (Pico W + Raspberry Pi 5)

An advanced **IoT air quality monitoring system** using a **Raspberry Pi Pico W** for sensor data collection and a **Raspberry Pi 5** for back-end data processing and visualization through **MQTT** and **Home Assistant**.

---

## 🚀 Features

* **Real-time Monitoring**: Tracks CO, Gas ($NH_3$, Benzene, Toluene), Temperature, and Humidity.
* **Dual-Stage Heating**: Implements the 5V/1.5V heating cycle required by the MQ-7 CO sensor via PWM.
* **Resilient Networking**: Stable MQTT publishing with automatic Wi-Fi reconnection and status polling.
* **Local Processing**: Dockerized backend for data privacy, low latency, and easy deployment.
* **Visual Dashboard**: Fully integrated with **Home Assistant** for historical data tracking and alerts.

---

## 🛠 Hardware Components

| Component | Description |
| :--- | :--- |
| **Microcontroller** | Raspberry Pi Pico W |
| **Gas Sensors** | MQ-135 (Air quality), MQ-7 (CO detection, PWM-heated) |
| **Env. Sensor** | AHT20 (Temperature & Humidity, I2C) |
| **Display** | OLED SSD1306 (128×64, I2C) |
| **Server** | Raspberry Pi 5 (8GB) running Docker |

---
## 📦 Requirements

### Hardware
- Raspberry Pi Pico W
- MQ-135 Air Quality Sensor
- MQ-7 CO Sensor
- AHT20 Temperature & Humidity Sensor
- SSD1306 OLED Display (128×64, I2C)
- Raspberry Pi 5 (backend & Home Assistant)
- Stable 5V power supply

### Software
- Raspberry Pi Pico SDK (C/C++)
- CMake ≥ 3.13
- GNU Arm Embedded Toolchain
- Docker & Docker Compose
- Home Assistant OS or Container
- MQTT Broker (Mosquitto)

---

## 📁 Project Structure

```text
AirQuality_IOT
├── CMakeLists.txt
├── docs
│   ├── docker-compose.yaml        # Docker stack definition
│   └── home_assistant_mqtt.yaml   # MQTT Sensor configuration for HA
├── pico_sdk_import.cmake
├── README.md
└── src
    ├── main.c                     # System logic & warm-up cycle
    ├── lwipopts.h                 # LwIP network configuration
    ├── Display/                   # OLED SSD1306 drivers
    ├── Networking/                # MQTT client implementation
    └── Sensors/
        ├── AHT20/                 # Temp & Humidity driver
        ├── MQ135/                 # Air Quality sensor logic
        └── MQ7/                   # CO sensor PWM control
```

---

## ⚙️ Backend Setup (Raspberry Pi 5)

The backend is fully containerized. It is recommended to manage the stack via **Portainer** or `docker-compose`.

1. **Deploy Stack**: Use the `docs/docker-compose.yaml` file.
2. **MQTT Configuration**: Ensure your `mosquitto.conf` allows external connections:
   ```conf
   listener 1883 0.0.0.0
   allow_anonymous true
   ```
3. **Home Assistant**: Append the content of `docs/home_assistant_mqtt.yaml` to your Home Assistant `configuration.yaml` file.

---

## 📊 Sensor Calibration

### MQ-135 and MQ-7 Warm-up
All MQ sensors require a stabilization period to reach the correct internal temperature.

* **$R_0$ Calibration**: Automatically performed in clean air during the system startup sequence.
* **MQ-7 Heating Cycle**:
    * **60s @ 5V**: Cleaning phase (High voltage).
    * **90s @ 1.5V**: Measurement phase (Low voltage). Data is only considered valid and published during this window.

---

## 💻 Pico W Firmware Build

1. **Install Pico SDK**: Ensure the Raspberry Pi Pico SDK is correctly installed and `PICO_SDK_PATH` is set.
2. **Configure Credentials**: Edit `src/Networking/mqtt.h` and `src/main.c`:
   ```c
   #define WIFI_SSID       "Your_SSID"
   #define WIFI_PASS       "Your_Password"
   #define MQTT_BROKER_IP  "192.168.1.4" // Your Pi 5 IP
   ```
3. **Build**:
   ```bash
   mkdir build && cd build
   cmake .. -DPICO_BOARD=pico_w
   make
   ```
4. **Flash**: Copy the generated `.uf2` file to the Pico W in BOOTSEL mode.

---

## 🧪 Testing and Debugging

- Sensor readings were validated by comparing values with datasheet reference curves.
- MQTT communication was tested under Wi-Fi disconnection scenarios to ensure automatic reconnection.
- PWM heating cycle of the MQ-7 was verified using an oscilloscope.
- I2C bus stability was tested with multiple devices connected simultaneously.

### Known Issues
- MQ sensors require long warm-up times and are sensitive to environmental changes.
- Gas concentration values are relative and not intended for medical or certified measurements.

----

## 🧱 Future Improvements

* ☁️ **Cloud Integration**: Optional bridge to AWS IoT or Azure for remote monitoring.
* 💨 **Particulate Matter**: Add support for PM2.5/PM10 sensors (e.g., SDS011).
* 📱 **Mobile Alerts**: Push notifications via Home Assistant Companion app for gas threshold breaches.

---

## 👤 User Guide

1. Power on the Raspberry Pi Pico W and the backend Raspberry Pi 5.
2. Wait for the MQ sensor warm-up phase (approx. 3–5 minutes).
3. Open Home Assistant and navigate to the Air Quality dashboard.
4. Monitor real-time values for:
   - CO concentration
   - Air quality (MQ-135)
   - Temperature and Humidity
5. Alerts are triggered automatically when predefined thresholds are exceeded.
   
---

## 🎥 Presentation and Demo

- 📊 Project Presentation: *(link to slides)*
- ▶️ Demo Video: *(YouTube link)*

--- 

## 👥 Credits

This project was developed as part of the **Embedded Software for the Internet of Things** course at the **University of Trento**.

**Bachelor's Degree in Computer, Communications and Electronic Engineering**

## 👥 Team Contributions

- **Matteo Miglio**: Sensor drivers (MQ-135, MQ-7), calibration logic.
- **Paolo Sarcletti**: Networking stack, MQTT communication, Wi-Fi handling.
- **Alessandro Gremes**: System integration, firmware architecture, Home Assistant integration.
- **Alessandro Turri**: Backend Docker setup, dashboard configuration, documentation.
