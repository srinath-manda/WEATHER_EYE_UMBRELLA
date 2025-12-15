# HARDWARE SETUP GUIDE - WEATHER_EYE_UMBRELLA

## Overview
This document provides detailed instructions for assembling and connecting all hardware components for the WEATHER_EYE_UMBRELLA IoT weather monitoring system.

## Required Components

### Main Microcontroller
- **ESP32 DevKit v1 or ESP32-WROOM-32**
  - Dual-core processor with built-in WiFi and Bluetooth
  - 4MB Flash memory
  - 520KB RAM

### Environmental Sensors
1. **DHT11 Temperature & Humidity Sensor**
   - Operating Voltage: 3.3-5V
   - Temperature Range: 0-50°C
   - Humidity Range: 20-90% RH

2. **GUVA-S12SD UV Index Sensor**
   - Operating Voltage: 3-5V
   - Analog output (0-3.3V)
   - Spectral Range: 280-390nm

3. **Rain Sensor Module (Digital Output)**
   - Operating Voltage: 3.3-5V
   - Digital output (HIGH = dry, LOW = rain detected)
   - Sensitivity: Adjustable

### Location Tracking
- **NEO-6M GPS Module**
  - UART Interface (TTL Serial)
  - Operating Voltage: 3.3-5V
  - Baud Rate: 9600
  - Warm-up Time: ~30 seconds

### Wireless Communication
- **SIM800L GSM Module**
  - Operating Voltage: 3.7-4.2V (requires separate 4.2V PSU)
  - Communication: UART (TTL Serial)
  - Baud Rate: 9600
  - SIM Card: Micro SIM required
  - Network: 2G GSM/GPRS

### Output Devices
- **Red LED** (3-5mm)
  - Forward Voltage: 2V
  - Current: 10-20mA
  - Resistor: 1kΩ

- **Active 5V Buzzer**
  - Operating Voltage: 5V
  - Current: 30-40mA
  - Frequency: 2-4kHz

## Detailed Wiring Instructions

### 1. DHT11 Temperature/Humidity Sensor Connection

```
DHT11 Module
   |
   +--- VCC ----> ESP32 3.3V
   +--- GND ----> ESP32 GND
   +--- DATA ---> ESP32 GPIO 4
```

**Notes:**
- Use a 10kΩ pull-up resistor between DATA and VCC for better signal integrity
- Keep wiring short to avoid signal interference

### 2. GUVA-S12SD UV Sensor Connection

```
GUVA-S12SD Module
   |
   +--- VCC ----> ESP32 3.3V
   +--- GND ----> ESP32 GND
   +--- OUT ----> ESP32 GPIO 34 (ADC0)
```

**Important:**
- GPIO 34 is an ADC-only pin (cannot be used as digital output)
- Maximum ADC voltage: 3.3V
- ADC resolution: 12-bit (0-4095)

### 3. Rain Sensor Module Connection

```
Rain Sensor
   |
   +--- VCC ----> ESP32 3.3V
   +--- GND ----> ESP32 GND
   +--- DO -----> ESP32 GPIO 32 (Digital Input)
```

**Logic Levels:**
- LOW (0V): Rain detected
- HIGH (3.3V): Dry conditions

### 4. NEO-6M GPS Module Connection

```
NEO-6M GPS Module                    ESP32
   |
   +--- VCC ----> ESP32 5V
   +--- GND ----> ESP32 GND
   +--- TX -----> GPIO 16 (RXD2) via logic level converter
   +--- RX -----> GPIO 17 (TXD2) via logic level converter
```

**Critical - Logic Level Conversion:**
- NEO-6M outputs 3.3V TTL levels
- ESP32 GPIO16/17 on UART2 accept 3.3V
- Use a voltage divider or logic level converter for 5V GPS modules:
  ```
  GPS TX (5V) ---[10kΩ]---+--- ESP32 GPIO16
                            |
                          [20kΩ]
                            |
                           GND
  ```

### 5. SIM800L GSM Module Connection

**IMPORTANT: SIM800L Requires Separate Power Supply!**

```
Power Supply (4.2V, 2A minimum)
   |
   +--- Positive ----> SIM800L VCC
   +--- Negative ----> SIM800L GND (common with ESP32 GND)

SIM800L GSM Module                   ESP32
   |
   +--- TX -----> GPIO 26 (RXD1) via logic level converter
   +--- RX -----> GPIO 27 (TXD1)
   +--- GND ----> ESP32 GND (common ground)
```

**Logic Level Conversion (SIM800L):**
```
SIM800L TX (3.3V) ---[1kΩ]---+--- ESP32 GPIO 26
                              |
                            [2kΩ]
                              |
                             GND
```

**SIM Card Installation:**
- Use Micro SIM card
- Insert with contacts facing downward
- Ensure proper insertion (should click)

### 6. LED Indicator Connection

```
Red LED
   |
  Anode (+) ---[1kΩ Resistor]----> ESP32 GPIO 2
  Cathode (-) ----> ESP32 GND
```

**Resistor Calculation:**
- R = (VCC - V_LED) / I
- R = (3.3V - 2V) / 0.02A = 65Ω (use 1kΩ for safety)

### 7. Buzzer Connection

```
5V Active Buzzer
   |
   +--- Positive (+) ----> ESP32 GPIO 15
   +--- Negative (-) ----> ESP32 GND
```

**Note:** Active buzzer handles voltage directly; no resistor needed.

## Pin Summary Table

| Component | Pin Type | ESP32 GPIO | Purpose | Voltage |
|-----------|----------|-----------|---------|----------|
| DHT11 | Digital I/O | GPIO 4 | Temperature/Humidity | 3.3V |
| GUVA-S12SD | Analog Input | GPIO 34 (ADC) | UV Index | 3.3V |
| Rain Sensor | Digital Input | GPIO 32 | Rain Detection | 3.3V |
| GPS RX | UART RX | GPIO 16 (UART2) | GPS Data In | 3.3V |
| GPS TX | UART TX | GPIO 17 (UART2) | GPS Data Out | 3.3V |
| SIM800L RX | UART RX | GPIO 26 (UART1) | GSM Data In | 3.3V |
| SIM800L TX | UART TX | GPIO 27 (UART1) | GSM Data Out | 3.3V |
| LED | Digital Output | GPIO 2 | Status Indicator | 3.3V |
| Buzzer | Digital Output | GPIO 15 | Audio Alert | 5V (from buzzer PSU) |

## Power Distribution

### Main Power Rail (3.3V)
```
ESP32 3.3V ----+---- DHT11 VCC
               +---- GUVA-S12SD VCC
               +---- Rain Sensor VCC
               +---- NEO-6M VCC (via buck converter from 5V)
```

### Ground Rail
```
Common GND ----+---- ESP32 GND
               +---- All sensor GND
               +---- SIM800L GND (important!)
```

### Separate Power Supplies
- **ESP32 & Sensors**: USB (5V 1A) or battery (3.7V LiPo)
- **SIM800L**: 4.2V 2A dedicated PSU
- **Buzzer**: 5V from USB or separate PSU

## Assembly Best Practices

1. **Start with GND connections** - Ensure all grounds are properly connected
2. **Use breadboard** - For prototyping and testing
3. **Short wires** - Minimize signal noise
4. **Power budgeting** - Verify current requirements
5. **Test each sensor** - Before final assembly
6. **Use headers** - For easy component replacement
7. **Label pins** - For easy troubleshooting
8. **Heat shrink** - For wire insulation

## Troubleshooting

### DHT11 Not Reading
- Check 10kΩ pull-up resistor
- Verify GPIO 4 connection
- Ensure 3.3V power

### GPS No Fix
- Wait 30+ seconds for lock
- Check sky visibility
- Verify UART2 connections
- Check baud rate (9600)

### SIM800L Issues
- **Verify 4.2V power supply** (most common issue!)
- Check SIM card insertion
- Verify network signal
- Check UART1 connections

### UV Sensor Not Reading
- Check ADC input (GPIO 34)
- Verify 3.3V power
- Expose to direct sunlight

## Component Cost Estimate (India - 2025)
- ESP32 DevKit: ₹300-400
- DHT11: ₹50-100
- GUVA-S12SD: ₹150-200
- Rain Sensor: ₹100-150
- NEO-6M GPS: ₹400-500
- SIM800L: ₹200-300
- LED & Buzzer: ₹50-100
- Power Supplies: ₹200-300
- Miscellaneous: ₹100-150

**Total Estimated Cost: ₹1500-2200 (USD 18-27)**
