# 🌱 Seed Sentinel: Sensor Node

![Status](https://img.shields.io/badge/Status-Prototype-orange)
[![Language: C++](https://img.shields.io/badge/Language-C++-00599C?style=flat&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Framework: PlatformIO](https://img.shields.io/badge/Framework-PlatformIO-F58220?style=flat&logo=platformio&logoColor=white)](https://platformio.org/)
[![Hardware: ESP32](https://img.shields.io/badge/Hardware-ESP32-E7352C?style=flat&logo=espressif&logoColor=white)](https://www.espressif.com/)

**The physical eyes and ears of the Seed Sentinel ecosystem.**

This repository contains the firmware, wiring schematics, and 3D printable enclosures for the **Seed Sentinel Node**—a low-cost, distributed soil telemetry device.

It is designed to connect to the **[Seed Sentinel Hub](https://github.com/nickmoyer/seed-sentinel)** (Go Backend + AI Agent).

---

## 📂 Repository Structure

| Folder | Description |
| :--- | :--- |
| **`firmware/`** | The **PlatformIO** project (C++ code) for the ESP32. |

---

## 🛒 Bill of Materials (BOM)

To build one node, you need the following components (~$10 USD total):

1.  **Microcontroller:** ESP32 Development Board (ESP-WROOM-32 / "DOIT DevKit V1").
2.  **Sensor:** Capacitive Soil Moisture Sensor v1.2.
3.  **Cabling:** 3x Male-to-Female Jumper Wires.
4.  **Power:** USB-C or Micro-USB cable (depending on your board).

---

## 🔌 Wiring Guide

**⚠️ CRITICAL:** You must use **ADC1** pins (GPIO 32-39). Do not use ADC2 pins, or the sensor will fail when WiFi connects.

| Sensor Pin | Wire Color | ESP32 Pin | Note |
| :--- | :--- | :--- | :--- |
| **GND** | ⚫ Black | **GND** | Common Ground |
| **VCC** | 🔴 Red | **3V3** | **Do NOT use 5V/VIN** |
| **AOUT** | 🟡 Yellow | **GPIO 32** | Analog Input (ADC1) |

---

## ⚡ Firmware Setup (PlatformIO)

This project uses **PlatformIO** (VS Code Extension). Do not use the standard Arduino IDE.

### 1. Configuration (Secrets)
We use a `secrets.ini` file to keep WiFi credentials out of Git.

1.  Navigate to the `firmware/` directory.
2.  Create a file named `secrets.ini`.
3.  Add your network details:

```ini
[secrets]
; Your WiFi Credentials
wifi_ssid = YOUR_SSID_HERE
wifi_pass = YOUR_PASSWORD_HERE

; Your Computer's IP Address (Where Go is running)
server_ip = http://YOUR_SERVER_IP:8080
```

### 2. Flashing the Device
1. Open the firmware/ folder in VS Code.
2. Connect the ESP32 via USB.
3. Click the PlatformIO Upload (→) arrow in the bottom status bar.
  - Note: If it fails to connect, hold the BOOT button on the ESP32 while it says "Connecting...".