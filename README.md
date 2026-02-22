# 🌱 Seed Sentinel: Sensor Node

![Status](https://img.shields.io/badge/Status-Prototype-orange)
[![Language: C++](https://img.shields.io/badge/Language-C++-00599C?style=flat&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Framework: PlatformIO](https://img.shields.io/badge/Framework-PlatformIO-F58220?style=flat&logo=platformio&logoColor=white)](https://platformio.org/)
[![Hardware: ESP32](https://img.shields.io/badge/Hardware-ESP32-E7352C?style=flat&logo=espressif&logoColor=white)](https://www.espressif.com/)

**The physical eyes and ears of the Seed Sentinel ecosystem.**

This repository contains the firmware, wiring schematics, and 3D printable enclosures for the **Seed Sentinel Node**—a low-cost, distributed soil telemetry device.

It is designed to connect to the **[Seed Sentinel Hub](https://github.com/nick-moyer/seed-sentinel)** (Go Backend + AI Agent).

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

### 1. Flashing the Device
1. Open the `firmware/` folder in VS Code.
2. Connect the ESP32 via USB.
3. Click the PlatformIO Upload (→) arrow in the bottom status bar.
  - Note: If it fails to connect, hold the BOOT button on the ESP32 while it says "Connecting...".

### 2. Configuration (WiFi Setup)
Once flashed, the device needs to be connected to your network:

1.  **Connect to Hotspot:** On your phone or computer, connect to the WiFi network named **`Seed_Sentinel_Setup`**.
2.  **Open Portal:** A configuration page should open automatically. If not, visit `http://192.168.4.1` in your browser.
3.  **Enter Details:** Input your WiFi SSID, Password, and the Server URL (e.g., `http://192.168.0.100:8080`).
4.  **Save:** Click "Save & Connect". The device will restart and connect to your network.