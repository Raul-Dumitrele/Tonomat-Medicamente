# 💊 Embedded Automated Medicine Dispenser

![Project Status](https://img.shields.io/badge/Status-Completed-success)
![Platform](https://img.shields.io/badge/Platform-Arduino%20%7C%20STM32-blue)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%28Embedded%29-00599C)

## 📖 Overview

This project is a safety-critical **embedded system** designed to automate the dispensing of pharmaceutical products. It replicates the functionality of a smart vending machine, featuring secure access control, environmental monitoring, and persistent inventory tracking.

The system is built on **C++** and focuses on hardware-software integration, utilizing **EEPROM** for data persistence and **PWM signals** for precise actuator control.

---

## 🚀 Key Features

* **🔐 Secure Access Control:** Users must authenticate via a 4x4 Keypad using unique 5-character alphanumeric codes (e.g., `1234A`) to dispense specific medication.
* **💾 Non-Volatile Memory (EEPROM):** Inventory levels are stored in the microcontroller's EEPROM. Data is preserved even after power loss or system reset.
* **🌡️ Environmental Monitoring:** Continuous tracking of Temperature and Humidity using a **DHT11** sensor to ensure safe storage conditions.
* **⚙️ Admin Maintenance Mode:** Hidden backdoor access (Code: `9999A`) allowing authorized personnel to instantly restock the machine.
* **🦾 Precise Actuation:** Servo motor control logic calculates the dispensing mechanism movement based on user-requested quantity.

---

## 🛠️ Hardware Architecture

### Components Used
* **Microcontroller:** Arduino Mega / STM32 (Compatible board)
* **Actuators:** 2x SG90 Micro Servos
* **Sensors:** DHT11 (Temperature & Humidity)
* **Input:** 4x4 Matrix Membrane Keypad
* **Display:** 16x2 LCD with I2C Interface

### Pinout Configuration

| Component | Pin Type | Pin Number | Note |
| :--- | :--- | :--- | :--- |
| **Servo 1** (Algocalmin) | PWM | `D11` | Dispensing Mechanism A |
| **Servo 2** (Paracetamol)| PWM | `D12` | Dispensing Mechanism B |
| **DHT11 Sensor** | Digital | `D36` | Temp/Humidity Data |
| **Keypad Rows** | Digital | `D9, D8, D7, D6` | Input Matrix |
| **Keypad Cols** | Digital | `D5, D4, D3, D2` | Input Matrix |
| **LCD Display** | I2C | `SDA / SCL` | Address `0x27` |

---

## 💻 Software Architecture

The firmware is designed using **Object-Oriented** principles to ensure modularity.

### 1. Data Structures
Instead of hardcoding variables, a `struct` is used to encapsulate product data:

```cpp
struct Medicine {
  char name[12];       // Product Name (Display)
  const char code[6];  // Security PIN
  int stock;           // RAM Stock Counter
  int eepromAddr;      // EEPROM Address for Persistence
  Servo* servo;        // Hardware Abstraction
};
