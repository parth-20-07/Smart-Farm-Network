<!-- omit from toc -->
# Smart Farm Network

**Table of Contents**

- [Introduction](#introduction)
- [Sensor Node](#sensor-node)
  - [Hardware Components](#hardware-components)
  - [Schematic](#schematic)
  - [PCB Design](#pcb-design)
- [Server Functionality](#server-functionality)
  - [Core Functions](#core-functions)
- [Setup and Operation](#setup-and-operation)
- [Data Handling and Alerts](#data-handling-and-alerts)
- [Customization and Expansion](#customization-and-expansion)
- [License](#license)
- [Design Details](#design-details)



# Introduction
The Smart Farm Network is a cutting-edge agricultural monitoring system designed to provide real-time insights into environmental conditions. Using a network of sensor nodes and a central server, it delivers detailed data analysis to help manage and optimize farming operations.

# Sensor Node
Equipped with sensors for temperature, humidity, lux, and CO2, each node collects data at set intervals. An integrated RTC ensures accurate timestamping. The nodes communicate with the server using BLE technology.

## Hardware Components
- **MicroUSB for Charging:** Ensures easy and accessible charging.
- **Battery Management:** Monitors and manages battery levels.
- **Power Conversion Units:** Provide the necessary voltages for different components.
- **ESP32 WROOM Module:** Processes and transmits the sensor data.
- **MicroSD Card Slot:** Enables data logging and storage.

## Schematic

![schematic](./Sensor%20Node/Hardware/Circuit%20Design/Rendered%20Images/Schematic.png)

## PCB Design

**PCB Top View**

![pcb-top](./Sensor%20Node/Hardware/Circuit%20Design/Rendered%20Images/PCB%20Top.png)

**PCB Bottom View**

![pcb-bottom](./Sensor%20Node/Hardware/Circuit%20Design/Rendered%20Images/PCB%20Bottom.png)

# Server Functionality
The server firmware manages data from over 50 nodes, calculating averages, and issuing alerts if readings exceed thresholds. It features:

## Core Functions
- **Data Processing:** Analyzes incoming data and manages MQTT connections to Digital Ocean for cloud uploads.
- **Display Control:** Updates the HMI display with real-time data and alert statuses.
- **Configuration Management:** Offers a web server interface for device configuration.
- **OTA Updates:** Maintains system integrity with the latest firmware updates.

# Setup and Operation
1. **Assembling Sensor Nodes:** Follow the provided schematics to assemble the hardware.
2. **Installing Firmware:** Flash the sensor nodes and server with the respective firmware.
3. **Configuring the Network:** Connect the nodes to the server via BLE and set up the server's Wi-Fi and MQTT settings.
4. **Launching the System:** Power up the system, and it begins automatic data collection and analysis.

# Data Handling and Alerts
The system stores data in microSD cards and uploads it to the cloud. It calculates average values and displays alerts on the HMI screen if parameters exceed set thresholds.

# Customization and Expansion
The firmware's modular design allows for easy addition of new sensors or communication protocols.

# License
This project is licensed under the GNU General Public License v3.0. See [LICENSE.md](LICENSE.md) for details.

# Design Details

- Designed for: City Green Private Ltd.
- Designed by: [Parth Patel](mailto:parth.pmech@gmail.com)