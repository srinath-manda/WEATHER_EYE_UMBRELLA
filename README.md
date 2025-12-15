# WEATHER_EYE_UMBRELLA

**IoT Weather Monitoring System with ESP32, SIM800L, DHT11, UV Sensor, Rain Sensor, and GPS**

---

## 📋 Project Overview

WEATHER_EYE_UMBRELLA is an advanced IoT weather monitoring system that combines multiple sensors and connectivity options to provide comprehensive environmental data logging and real-time alerts. The system uses an ESP32 microcontroller to aggregate sensor data and transmit it to ThingSpeak for cloud-based monitoring, while also sending SMS alerts via SIM800L GSM module when environmental thresholds are exceeded.

### Key Features
- **Real-time Weather Monitoring**: Temperature, humidity, UV index, and rain detection
- **Multi-Protocol Connectivity**: WiFi (ESP32) + GSM (SIM800L)
- **Cloud Data Logging**: Automatic data upload to ThingSpeak with 20-second intervals
- **GPS Location Tracking**: NEO-6M GPS module for precise location data
- **Smart Alerts**: SMS notifications + LED/Buzzer warnings when thresholds are breached
- **Low-Power Design**: Optimized timing intervals and stateful GSM communication
- **Edge Processing**: Local threshold detection and alert triggering

---

## 🛠️ Hardware Components

| Component | Model/Specification | Pin(s) | Purpose |
|-----------|-------------------|--------|----------|
| Microcontroller | ESP32 DevKit | - | Main processing unit |
| Temperature/Humidity Sensor | DHT11 | GPIO 4 | Ambient conditions |
| UV Index Sensor | GUVA-S12SD | GPIO 34 (ADC) | UV radiation measurement |
| Rain Sensor | Digital DO Output | GPIO 32 | Rain detection (LOW = rain) |
| GPS Module | NEO-6M | GPIO 16/17 (UART2) | Location tracking |
| GSM Module | SIM800L | GPIO 26/27 (UART1) | SMS transmission |
| LED Indicator | Standard Red LED | GPIO 2 | Alert visualization |
| Buzzer | 5V Active Buzzer | GPIO 15 | Audible alert |

### Pin Configuration Summary
```
ESP32 Pin Layout:
- DHT11: GPIO 4
- GUVA-S12SD: GPIO 34 (ADC0)
- Rain Sensor: GPIO 32
- LED: GPIO 2
- Buzzer: GPIO 15
- GPS RXD2: GPIO 16
- GPS TXD2: GPIO 17
- SIM800L RX: GPIO 26 (UART1)
- SIM800L TX: GPIO 27 (UART1)
```

---

## 📡 Software Architecture

### System Flow
1. **Sensor Reading** (Every 2 seconds)
   - Read DHT11 temperature and humidity
   - Measure UV voltage from analog pin
   - Check rain sensor status

2. **Threshold Monitoring** (Real-time)
   - Temperature ≥ 32°C
   - Humidity ≥ 70%
   - UV Voltage ≥ 1.6V
   - Rain Detected (GPIO LOW)

3. **Alert Generation** (With 60-second cooldown)
   - LED blink pattern (5 times)
   - Buzzer activation (5 pulses)
   - SMS alert via SIM800L
   - GPS location appended to message

4. **Data Transmission**
   - ThingSpeak Update: Every 20 seconds
   - GPS SMS: Every 10 seconds
   - WiFi-based for main data, GSM for SMS

### Libraries Used
```cpp
#include <WiFi.h>              // WiFi connectivity
#include <HTTPClient.h>        // HTTP POST requests
#include <TinyGPS++.h>         // GPS parsing
#include "DHT.h"               // DHT11 sensor
#include "StatefulGSMLib.h"    // SIM800L management
```

---

## ⚙️ Configuration

### User Configuration (In Code)
```cpp
const char* WIFI_SSID = "Srinath4321";
const char* WIFI_PASS = "";
const char* THINGSPEAK_API_KEY = "URTKKP74L6IQBTM9";
const unsigned long THINGSPEAK_UPDATE_INTERVAL_MS = 20000UL; // 20s
String targetPhone = "+917981454364";
```

### Threshold Configuration
```cpp
const float TEMP_THRESHOLD_C = 32.0;       // Temperature limit
const float HUMIDITY_THRESHOLD = 70.0;     // Humidity limit
const float UV_VOLTAGE_THRESHOLD = 1.6;    // UV sensor voltage limit
```

### Timing Parameters
```cpp
const unsigned long smsGpsInterval = 10000UL;    // GPS SMS: 10 seconds
const unsigned long sensorReadInterval = 2000UL; // Sensor read: 2 seconds
const unsigned long alertCooldown = 60000UL;     // Alert cooldown: 60 seconds
const unsigned long THINGSPEAK_UPDATE_INTERVAL_MS = 20000UL; // 20 seconds
```

---

## 📊 ThingSpeak Channel Fields

| Field | Data | Format |
|-------|------|--------|
| Field 1 | Temperature | Float (2 decimals, °C) |
| Field 2 | Humidity | Float (2 decimals, %) |
| Field 3 | UV Voltage | Float (3 decimals, V) |
| Field 4 | Rain Status | Integer (1=rain, 0=dry) |
| Field 5 | Latitude | Double (6 decimals) |
| Field 6 | Longitude | Double (6 decimals) |

### Sample ThingSpeak POST URL
```
http://api.thingspeak.com/update?api_key=URTKKP74L6IQBTM9&field1=28.50&field2=65.30&field3=1.234&field4=0&field5=12.123456&field6=77.654321
```

---

## 📱 SMS Alert Format

**Threshold Alert:**
```
ALERT: TEMP HUM UV RAIN
GPS: 12.123456,77.654321
```

**GPS Update (Every 10 seconds):**
```
GPS Update: 12.123456,77.654321
```

---

## 🚀 Installation & Setup

### Prerequisites
- Arduino IDE 1.8.19 or higher
- ESP32 Board Support Package
- Required Libraries:
  - DHT Sensor Library (by Adafruit)
  - TinyGPSPlus
  - StatefulGSMLib
  - HTTPClient

### Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/srinath-manda/WEATHER_EYE_UMBRELLA.git
   ```

2. Open Arduino IDE and install required libraries via Library Manager

3. Update configuration in the sketch:
   - WiFi SSID and password
   - ThingSpeak API key
   - Target phone number for SMS

4. Select ESP32 board and appropriate COM port

5. Upload the sketch to ESP32

---

## 🔌 Wiring Diagram Guide

### DHT11 Connection
```
DHT11
VCC → ESP32 3.3V
GND → ESP32 GND
DATA → GPIO 4
```

### GUVA-S12SD UV Sensor
```
VCC → ESP32 3.3V
GND → ESP32 GND
OUT → GPIO 34 (ADC0)
```

### Rain Sensor
```
VCC → ESP32 3.3V
GND → ESP32 GND
DO → GPIO 32 (Digital Output)
```

### NEO-6M GPS Module
```
VCC → ESP32 5V
GND → ESP32 GND
TX → GPIO 16 (RXD2)
RX → GPIO 17 (TXD2)
```

### SIM800L GSM Module
```
VCC → 4.2V Power Supply
GND → ESP32 GND
RX → GPIO 27 (TXD2)
TX → GPIO 26 (UART1 RX)
```

### LED & Buzzer
```
LED:
  Anode → GPIO 2 (via 1kΩ resistor)
  Cathode → ESP32 GND

Buzzer:
  Positive → GPIO 15
  Negative → ESP32 GND
```

---

## 📈 Performance Metrics

- **Sensor Update Rate**: 2 seconds
- **ThingSpeak Upload Rate**: 20 seconds
- **GPS SMS Rate**: 10 seconds
- **WiFi Connection Time**: < 10 seconds
- **SIM800L Initialization**: ~5 seconds
- **Alert Response Time**: < 100ms

---

## 🐛 Debugging Tips

1. **Serial Monitor**: Open at 115200 baud rate to view debug messages
2. **WiFi Issues**: Check SSID/password and WiFi signal strength
3. **SIM800L Problems**: Verify power supply voltage (4.2V recommended)
4. **GPS No Fix**: Wait 30+ seconds for initial lock, check sky visibility
5. **ThingSpeak Not Updating**: Verify API key and internet connectivity
6. **SMS Not Sending**: Check SIM card balance and network signal

---

## 📝 Code Structure

- **User Configuration Section**: Customizable parameters
- **Pin Definitions**: Hardware mapping
- **Sensor Instances**: DHT, GPS, GSM objects
- **Threshold Constants**: Alert triggering values
- **Timing Variables**: Interval management
- **Helper Functions**: Sensor reading, alert generation, data transmission
- **Setup Function**: Hardware initialization
- **Main Loop**: Real-time sensor processing and data transmission

---

## 🎯 Future Enhancements

- [ ] Add barometric pressure sensor (BMP280)
- [ ] Implement wind speed and direction sensors (anemometer)
- [ ] Add LoRaWAN connectivity for extended range
- [ ] Cloud dashboard with real-time visualization
- [ ] Mobile app for instant notifications
- [ ] Data logging to local SD card for backup
- [ ] Multi-threshold alert levels (warning, critical)
- [ ] Battery monitoring and low-power modes

---

## 📄 License

This project is open source and available under the MIT License.

---

## 👨‍💼 Author

**Srinath Manda**  
Student | IoT & Embedded Systems Developer  
[GitHub](https://github.com/srinath-manda) | [LinkedIn](https://linkedin.com/in/srinath-manda)

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---

## 📞 Support

For issues or questions, please open a GitHub issue or contact the project maintainer.
