# OPC UA ESP32 Server

An OPC UA server implementation for ESP32 microcontrollers, with a purpose of controlling and monitoring environmental parameters inside a laboratory black-box.

## About

This project implements an OPC UA server on ESP32 hardware, allowing the device to communicate using the OPC UA protocol - a standard for industrial automation and IoT applications. The implementation is based on [opcua-esp32](https://github.com/cmbahadir/opcua-esp32) library.
## Features

- OPC UA server running on ESP32
- Industrial-standard communication protocol
- Support for data nodes and variables
- WiFi connectivity
- Configurable server endpoints

## Hardware Requirements

- ESP32 development board
- I2C bus and sensors
- USB cable for programming and power
- WiFi network for connectivity

## Installation

### Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) development framework
- Git

### Setup

1. Clone the repository:
```bash
git clone https://github.com/jakubjrysz/blackbox-environment-control-system)
```

2. Configure your WiFi credentials and server settings, using provided template:
```bash
idf.py menuconfig
-> Connection settings
-> WiFi ssid & password
```

3. Build the project:
```bash
idf.py build
```

4. Flash to your ESP32:
```bash
idf.py -p (PORT) flash monitor
```

Replace `(PORT)` with your ESP32's serial port (e.g., `/dev/ttyUSB0` on Linux or `COM3` on Windows).

## Usage

Once flashed and running, the ESP32 will:
1. Connect to the configured WiFi network
2. Start the OPC UA server
3. Expose data nodes accessible via OPC UA clients

You can connect to the server using any OPC UA client (such as UAExpert or Prosys OPC UA Browser) using the server's IP address and configured endpoint.


## Acknowledgments

This project is built upon:
- [open62541](https://github.com/open62541/open62541) - An open source OPC UA implementation
- [opcua-esp32](https://github.com/cmbahadir/opcua-esp32) - ESP32 port by cmbahadir
- [DL_OPC_SERVER](https://github.com/Mikor01/DesignLab_OPC_SERVER) - ESP32 OPC UA server with different purpose


## Project Progress
30.10.25 - begining of the project

10.11.25 - added library for sensor communication

23.11.25 - setup of first working version of OPC UA server

09.12.25 - combining together sensors and server
