# Smart IoT Voltage & Overcurrent Protection System

**FSMB Recruitment 2026 - Phase 1 Project Assignment**  
**Position:** Embedded Systems Engineer  
**Author:** Mohammad Toki Bin Alam  
**Repository:** [tokibinalam-star/voltage-and-current-protector-device](https://github.com/tokibinalam-star/voltage-and-current-protector-device/)  
**Simulation Platform:** Wokwi (ESP32 DevKit C V4)  

---
## How to use
1. Press SET_VMAX <val> for setting maximum voltage limit. For example, if you want to set max voltage at 250V. Just type SET_VMAX 250.Same goes for SET_VMIN and SET_IMAX.
2. Then try to drag the Current Controller (Potentiometer) and Voltage Controller (Also a potentiometer).
3. If Voltage> Vmax OR Voltage<Vmin OR Current>Imax , the load (Orange LED) will turn off and the RED LED will turn on. This state means Trip state.
4. Otherwise, the circuit will stay in Safe Mode.

   
## Executive Summary

This repository contains the complete firmware, schematic representations, simulation files, and documentation for a microcontroller-based **Voltage & Overcurrent Protector**. The device continuously monitors electrical grid health to protect connected household appliances from damaging grid anomalies, including over-voltage surges, under-voltage brownouts, and over-current overload conditions.

To comply with high-voltage safety guidelines, grid conditions are emulated safely using low-voltage 0–3.3V DC analog stand-ins (potentiometers). The system features dual computer-communication interfaces (Serial CLI and Wi-Fi REST API) allowing users to dynamically configure threshold parameters in real time.

---

## Key Features

* **Real-time Grid Monitoring:** Continuous ADC sampling of scaled grid voltage (0–300V AC) and load current (0–30A AC).
* **Hardware Load Disconnection:** Active high-speed relay control physically breaks the power circuit during fault states[cite: 2].
* **Dual Computer Communication:** 
  * **Serial CLI:** Bi-directional terminal interface for dynamic parameter updates in simulation or local USB connection[cite: 2].
  * **REST API:** Embedded HTTP WebServer listening on Port 80 for network-based parameter configuration.
* **Local Visual Interface:** 16x2 I2C LCD displaying live values and operational status, paired with dual status LEDs (Green = SAFE, Red = TRIPPED).
* **Emulated Appliance Load:** Orange LED powered through the Relay's Normally Open (`NO`) terminal to visually demonstrate load shedding.

---

## Hardware Architecture & Pin Mapping

| Peripheral | Component | ESP32 Pin | Logic Level / Operating Specs |
| :--- | :--- | :--- | :--- |
| **Voltage Sensor (Emulated)** | Slide Potentiometer 1 | `GPIO 34` (ADC1_CH6) | 0–3.3V scaled to 0–300V AC |
| **Current Sensor (Emulated)** | Slide Potentiometer 2 | `GPIO 35` (ADC1_CH7) | 0–3.3V scaled to 0–30A AC |
| **Circuit Breaker** | 5V SPDT Relay Module | `GPIO 4` | Active-HIGH drive |
| **Local Display** | I2C 16x2 LCD | `GPIO 21` (SDA), `GPIO 22` (SCL) | 5V Power, 3.3V I2C Bus |
| **Safe Status Indicator** | Green LED | `GPIO 18` | Lit during normal operation |
| **Fault Status Indicator** | Red LED | `GPIO 19` | Lit during trip state |
| **Demo Appliance Load** | Orange LED | Relay `NO` Terminal | Powered when Relay is engaged |

---
