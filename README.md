# Noctua

![Version](https://img.shields.io/badge/Noctua-v1.0-blueviolet?style=flat-square)
![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Arduino](https://img.shields.io/badge/Framework-Arduino-008184?style=flat-square&logo=arduino&logoColor=white)
![Open Source](https://img.shields.io/badge/Open_Source-%E2%9D%A4-brightgreen?style=flat-square)

**Noctua** is an educational device based on the **ESP32** designed for **scanning and data collection of Wi-Fi and Bluetooth networks** in the environment.  
The project focuses on **practical learning** in networking, security, wireless protocols, embedded systems, and data organization.

Inspired by the owl, Noctua observes silently, records information, and allows for later analysis. No noise, no attacks, no drama.

> ⚠️ **Disclaimer** > Noctua is strictly **educational**. The project does not explore vulnerabilities and does not interfere with the operation of networks or devices.  
> Any modification or use outside of the ethical and legal scope is **not the responsibility of the original project**.

---

## Project Objectives

- Perform **passive scanning** of nearby Wi-Fi networks and BLE devices.  
- Identify **open networks**, without any attempt at intrusion.  
- Collect relevant technical metadata for study and analysis.  
- Record data on a **microSD** for later offline analysis.  
- Explore concepts of FSM, events, states, and logging in embedded systems.  

---

## Features

- **Multi-protocol Monitoring:** Passive scanning of Wi-Fi networks and Bluetooth (Classic/BLE) devices in real-time.

- **Metadata Capture:** Automatic collection of public information from open networks during the connection process.

- **Offline Data Persistence:** Structured log recording on a microSD card (CSV format), facilitating subsequent analysis in spreadsheet software.

- **Modular Architecture:** Code fully decoupled into specific modules (FSM, Scanners, Logger, Indicators), ensuring scalability and easy maintenance.

- **State Management (FSM):** Logic based on a Finite State Machine for a predictable flow.

- **Hardware Resilience (Watchdog):** Continuous verification system to ensure the initialization and availability of the microSD module.

- **Diagnostic Interface:** Debugging via serial output (native support for CH340 converters).

- **Intelligent Visual Feedback:** LED signaling system for state indication, recording errors, and signal strength.

- **Remote Access Server (HTTP):** Integrated web interface that allows listing and downloading logs directly via a browser, eliminating the need for physical removal of the microSD card.

- **Wardriving:** Continuous capture of SSIDs and power levels (RSSI), allowing for the analysis of coverage variation and network density along a path through logs.

### 🟢 Hybrid Parallel Processing Architecture

- **Asynchronous Dual-Core Execution:** Full utilization of the ESP32's dual-core architecture via FreeRTOS. While **Core 1** manages the FSM and radio sensors (Wi-Fi/BT) in real-time, **Core 0** is dedicated exclusively to heavy microSD write operations, eliminating I/O bottlenecks.

- **Queue-Based Communication (RTOS Queues):** Implementation of inter-core message queues for data packet transport. This ensures that no network data is lost during traffic spikes, acting as a high-speed hardware buffer.

- **Smart Memory Management:** Dynamic memory allocation for data structs within discovery callbacks, ensuring thread safety and preventing race conditions between the scanning and logging tasks.

- **Hybrid Compatibility Mode:** Through compilation macros (`ASYNC_SD_HANDLER`), the system can automatically toggle between Parallel Mode (Dual-Core) and Sequential Mode (Single-Core). This ensures the firmware remains stable across all ESP32 variants (S2, C3, or the Classic model).

- **Sequential Log Fallback:** In single-core systems, the firmware executes an automated "queue drain" routine immediately after each scan, ensuring data persistence without disrupting the FSM logic.

---

## What Noctua Collects

### Wi-Fi

- SSID  
- BSSID (Access Point MAC)  
- Wi-Fi Channel  
- Band (2.4 GHz / 5 GHz)  
- RSSI (Signal Strength)  
- Security Type  
- The IP received by the device
- DHCP Status
- Subnet Mask
- Hostname

---

### Bluetooth (BLE)

- MAC Address  
- Device name (when available)  
- RSSI  
- Address type  
- Observed BLE channel  

---

![SCM](/docs/schematics1.png)

## Required Components

- **1x ESP32 NodeMCU** – Main microcontroller  
- **3x Tactile buttons** – Simple flow control  
- **1x 5V Battery** – Portable operation (**Optional**)
- **1x microSD Module** – Storage for collected data  
- **2x 330Ω Resistors** – In case you wish to connect extra LEDs (**Optional**)
- **2x LEDs** – More polished visual feedback (**Optional**)

---

## Project Architecture

### Hardware

- ESP32 connected to:
  - Tactile buttons  
  - microSD module  
- Buttons:
  - **Button A** → Start new WiFi scan  
  - **Button B** → Start new Bluetooth scan  
  - **Button C** → Start the HTTP server  

---

## Finite State Machine (FSM)

1. **IDLE** - Device on, awaiting action  

2. **SCAN** - Passive Wi-Fi or Bluetooth scan  

3. **PROCESS** - Organization and filtering of collected data  

4. **WEB_SERVER** - Start of the HTTP Server
  
![Cycle](/docs/FSM.png)

> The FSM is simple by design. Clarity > Unnecessary complexity.

---

## License

This project is **open source**, licensed under the **Apache License 2.0**.  
You may study, modify, and distribute the code, **provided that you maintain educational and ethical use.**
