# Noctua

![Version](https://img.shields.io/badge/Noctua-v1.1-blueviolet?style=flat-square)
![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Arduino](https://img.shields.io/badge/Framework-Arduino-008184?style=flat-square&logo=arduino&logoColor=white)
![Open Source](https://img.shields.io/badge/Open_Source-%E2%9D%A4-brightgreen?style=flat-square)

**Noctua** is an educational device based on the **ESP32** designed for **scanning and data collection of Wi-Fi and Bluetooth networks** in the environment.  
The project focuses on **practical learning** in networking, security, wireless protocols, embedded systems, and data organization.

Inspired by the owl, Noctua observes silently, records information, and allows for later analysis. No noise, no drama.

> ⚠️ **Disclaimer** > Noctua is strictly **educational**. The project does not interfere with the operation of networks or devices.  
> Any modification or use outside of the ethical and legal scope is **not the responsibility of the project**.

---

## Project Objectives

- Perform **passive scanning** of nearby Wi-Fi networks and BLE devices.  
- Collect relevant technical metadata for study and analysis.  
- Record data on a **microSD** for later offline analysis.  
- Explore concepts of FSM, events, states, and logging in embedded systems.

---

## Features

- **Multi-protocol Monitoring:** Passive scanning of Wi-Fi networks and Bluetooth (Classic/BLE) devices in real-time.

- **Headless Status Interface:** An intuitive 3-LED system that provides immediate feedback on operation status (Active, Success, or Error) without needing a display.

- **Offline Data Persistence:** Structured log recording on a microSD card (CSV format).

- **Remote Access Server (HTTP):** Integrated web interface to download logs directly via browser. Includes a **physical kill-switch** to instantly stop the server and cut radio transmission for stealth.

- **Wardriving Mode:** Continuous capture of SSIDs and RSSI levels to analyze coverage and density.

- **Hardware Resilience:** Watchdog timers and SD health checks to prevent locking up in the field.

### 🟢 Hybrid Parallel Processing Architecture

- **Asynchronous Dual-Core Execution:** Full utilization of the ESP32's dual-core architecture via FreeRTOS. **Core 1** manages the FSM and radio sensors, while **Core 0** handles heavy microSD write operations.
  
- **Queue-Based Communication:** Inter-core message queues ensure no network data is lost during traffic spikes.

- **Hybrid Compatibility Mode:** The `ASYNC_SD_HANDLER` macro allows automatic toggling between Parallel Mode (Dual-Core) and Sequential Mode (Single-Core) for broader hardware support (ESP32-S2/C3).

---

## Hardware & Controls

### Required Components

- **1x ESP32 NodeMCU** – Main microcontroller  
- **2x Tactile buttons (12mm)** – Navigation and Control  
- **1x microSD Module** – Storage  
- **3x LEDs (5mm)** – Status Indicators:
  - 🟢 **Green:** **Success** (Scan completed / Data saved successfully / Server Running)
  - 🟡 **Yellow:** **Error** (SD Card fail / Connection error / Timeout)
  - 🔴 **Red:** **Active/Busy** (Wardrive running)
- **3x 330R or 510R Resistors** – LED protection
- **1x 5V Power Bank** – For portability

### Navigation Logic (FSM)

The device uses a **Menu/Select** paradigm with status feedback:

- **Button A (Navigation):** Cycles through the available modes in the menu.

- **Button B (Action):**

  - **In Menu:** Confirms selection and starts the mode.

  - **In Wardrive:** Stops the scan and saves data.

  - **In Server:** **Panic mode** – Immediately stops the HTTP server, disconnects Wi-Fi, and turns off the radio.

---

## Finite State Machine (FSM)

1. **IDLE**: Device is ready. User uses **Button A** to select the desired operation.

2. **SCAN**: Executes the selected routine.

3. **WARDRIVE_MODE**: Continuous loop of scanning and logging. Exits immediately upon pressing **Button B**.

4. **WEB_SERVER**: Creates an Access Point to serve files. Exits immediately upon pressing **Button B**.

![Cycle](/docs/FSM.png)

---

## What Noctua Collects

### Wi-Fi

- SSID  
- BSSID (Access Point MAC)  
- Wi-Fi Channel  
- RSSI (Signal Strength)  
- Security Type  
- The IP received by the device
- DHCP Status
- Subnet Mask
- Hostname

### Bluetooth (BLE)

- MAC Address  
- Device name (when available)  
- RSSI  
- Address type

### Wardriving mode

- SSID
- BSSID

---

## License

This project is **open source**, licensed under the **Apache License 2.0**.  
You may study, modify, and distribute the code, **provided that you maintain educational and ethical use.**
