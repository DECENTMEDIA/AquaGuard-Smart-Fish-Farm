# AquaGuard – Smart Fish Farm Monitoring and Automation System

## 1. Project Overview

AquaGuard is an ESP32-based smart fish farm monitoring and automation system designed to monitor water conditions and support automated water management.

The project combines embedded systems, sensors, electronics, and automation to help fish farmers monitor important water parameters and respond to changing conditions.

## 2. Problem Statement

Maintaining suitable water conditions is important for fish health and farm productivity.

Traditional fish farming may require farmers to manually monitor water conditions and manage water exchange.

AquaGuard aims to support this process through sensor-based monitoring, visual alerts, and automated water management.

## 3. Project Objectives

- Monitor fish pond water temperature.
- Monitor water pH.
- Monitor water turbidity.
- Detect water levels using a float switch and ultrasonic sensor.
- Display sensor readings on an LCD.
- Provide alerts using LEDs and a buzzer.
- Support automated water inlet and drainage using relays.
- Develop an embedded system prototype for smart fish farming.

## 4. Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| DS18B20 | Water temperature monitoring |
| pH sensor | Water acidity monitoring |
| Turbidity sensor | Water clarity monitoring |
| HC-SR04 ultrasonic sensor | Distance and water level measurement |
| Float switch | Water level detection |
| 20×4 I2C LCD | Display sensor readings |
| Relay modules | Control water inlet and drainage |
| LEDs | System status indication |
| Buzzer | Audible alerts |

## 5. System Architecture

The ESP32 receives readings from the connected sensors.

The readings are processed by the firmware and displayed on the LCD.

The system also uses water level detection and relay outputs to support automated water management.

LEDs and the buzzer provide status indications and alerts.

## 6. Software and Development Tools

- Arduino IDE
- C/C++ for ESP32 programming
- Wokwi for circuit simulation
- KiCad for PCB design

## 7. Project Features

- Sensor-based water monitoring
- Temperature measurement
- pH monitoring
- Turbidity monitoring
- Water level detection
- LCD dashboard
- Audible and visual alerts
- Relay-controlled water management

## 8. Repository Structure

```text
AquaGuard-Smart-Fish-Farm/
│
├── Arduino_Code/
│   └── AquaGuard.ino
│
├── Wokwi_Simulation/
│   ├── diagram.json
│   └── libraries.txt
│
├── PCB_Design/
│   └── KiCad project files
│
└── README.md
```

The repository structure may be updated as the project develops.

## 9. Development Status

AquaGuard is under prototype development and testing.

Current development activities include sensor integration, ESP32 firmware development, LCD monitoring, water level detection, relay control, circuit simulation, and PCB design.

Further testing and calibration are required to validate the complete system.

## 10. Future Improvements

- Sensor calibration and validation
- Improved water quality monitoring
- Additional automation features
- Remote monitoring and notifications
- Improved enclosure and PCB integration
- Further prototype testing

## 11. Project Team

AquaGuard Project Team

## 12. Acknowledgment

Developed as an embedded systems and smart fish farming project focused on monitoring, automation, and practical engineering applications.
