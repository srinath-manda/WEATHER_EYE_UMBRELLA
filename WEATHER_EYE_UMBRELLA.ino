/*
  ESP32 + SIM800L + DHT11 + GUVA-S12SD + Rain Sensor (Digital DO) + NEO-6M GPS
  WEATHER_EYE_UMBRELLA - IoT Weather Monitoring System
  
  Features:
  - Sends sensor data to ThingSpeak via WiFi
  - Sends GPS SMS every 10s using SIM800L (StatefulGSMLib)
  - Alerts via SMS + LED + Buzzer when thresholds exceeded
  - Real-time temperature, humidity, UV index, and rain detection
*/

/* ---------- USER CONFIG ---------- */
const char* WIFI_SSID = "Srinath4321";
const char* WIFI_PASS = "";
const char* THINGSPEAK_API_KEY = "URTKKP74L6IQBTM9";
const unsigned long THINGSPEAK_UPDATE_INTERVAL_MS = 20000UL; // 20s
String targetPhone = "+917981454364";
/* --------------------------------- */


#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include "DHT.h"
#include "StatefulGSMLib.h"


// ------------------- Pin Definitions -------------------
#define DHTPIN 4
#define DHTTYPE DHT11


#define GUVA_PIN 34
#define RAIN_DO_PIN 32  // Digital output of Rain Sensor


#define LED_PIN 2
#define BUZZER_PIN 15


#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600


TinyGPSPlus gps;


// Use UART2 for GPS communication
HardwareSerial gpsSerial(2);


// SIM800L (UART1)
#define MODEM_RX_PIN 26   // SIM800L TX → ESP32 RX
#define MODEM_TX_PIN 27   // SIM800L RX ← ESP32 TX
#define MODEM_RST_PIN -1
#define MODEM_PWRKEY_PIN -1
#define MODEM_PWR_EXT_PIN -1
#define MODEM_BAUD_RATE 9600


HardwareSerial HSerial1(1);
SIM800L sim800(HSerial1); // SIM800L instance


DHT dht(DHTPIN, DHTTYPE);


// ------------------- Thresholds -------------------
const float TEMP_THRESHOLD_C = 32.0;
const float HUMIDITY_THRESHOLD = 70.0;
const float UV_VOLTAGE_THRESHOLD = 1.6;
// Rain DO: LOW = rain detected, HIGH = dry


// ------------------- Timing -------------------
unsigned long lastThingSpeakUpdate = 0;
unsigned long lastSmsGpsSent = 0;
const unsigned long smsGpsInterval = 10000UL; // 10 seconds
unsigned long lastSensorRead = 0;
const unsigned long sensorReadInterval = 2000UL;
unsigned long lastAlertSent = 0;
const unsigned long alertCooldown = 60000UL; // 1 min between alerts
bool alertActive = false;


// ------------------- Helper Functions -------------------
float readAnalogVoltage(int adcPin) {
  int raw = analogRead(adcPin);
  return (raw / 4095.0f) * 3.3f;
}


String gpsToString() {
  if (gps.location.isValid()) {
    return String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  }
  return "NoFix";
}


void triggerAlert(const String &reason) {
  unsigned long now = millis();
  if (now - lastAlertSent < alertCooldown) return;
  lastAlertSent = now;
  alertActive = true;


  Serial.println("ALERT: " + reason);


  if (sim800.state() == STATE_READY) {
    String msg = "ALERT: " + reason + "\nGPS: " + gpsToString();
    sim800.sendSMS(targetPhone, msg);
    Serial.println("Alert SMS sent: " + msg);
  } else {
    Serial.println("SIM800L not ready: cannot send ALERT SMS now.");
  }


  // Blink LED + Buzzer 5 times (blocking but short)
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(300);
  }
}


void maybeSendGpsSms() {
  unsigned long now = millis();
  if (now - lastSmsGpsSent < smsGpsInterval) return;
  lastSmsGpsSent = now;


  String gpsStr = gpsToString();
  if (gpsStr != "NoFix" && sim800.state() == STATE_READY) {
    String message = "GPS Update: " + gpsStr;
    sim800.sendSMS(targetPhone, message);
    Serial.println("GPS SMS sent: " + message);
  } else {
    Serial.println("No GPS fix or SIM800L not ready.");
  }
}


void updateThingSpeak(float temp, float hum, float uvVolt, bool rainDetected, double lat, double lng) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - cannot post to ThingSpeak.");
    return;
  }


  HTTPClient http;
  String url = "http://api.thingspeak.com/update";
  String postData = "api_key=" + String(THINGSPEAK_API_KEY) +
                    "&field1=" + String(temp, 2) +
                    "&field2=" + String(hum, 2) +
                    "&field3=" + String(uvVolt, 3) +
                    "&field4=" + String(rainDetected ? 1 : 0) +
                    "&field5=" + String(lat, 6) +
                    "&field6=" + String(lng, 6);


  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  if (httpCode > 0)
    Serial.printf("ThingSpeak update %d : %s\n", httpCode, http.getString().c_str());
  else
    Serial.printf("ThingSpeak update failed: %s\n", http.errorToString(httpCode).c_str());
  http.end();
}


// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  delay(200);


  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);


  pinMode(RAIN_DO_PIN, INPUT); // Digital rain sensor


  dht.begin();
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2); // GPS module
  Serial.println("GPS serial started at 9600 baud");


  // SIM800L
  Serial.println("Starting SIM800L modem...");
  HSerial1.begin(MODEM_BAUD_RATE, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
  sim800.begin(MODEM_BAUD_RATE, MODEM_RX_PIN, MODEM_TX_PIN,
               MODEM_PWRKEY_PIN, MODEM_RST_PIN, MODEM_PWR_EXT_PIN);


  // wait a short time for modem state to stabilize (non-blocking style)
  unsigned long modemStart = millis();
  while (millis() - modemStart < 5000UL) {
    sim800.loop();
    delay(100);
  }
  if (sim800.state() == STATE_READY) {
    Serial.println("SIM800L ready.");
  } else {
    Serial.println("SIM800L not ready after init. Current state: " + String(sim800.state()));
  }


  // WiFi
  Serial.println("Connecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long startWiFi = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startWiFi < 10000UL) {
    delay(250);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  else
    Serial.println("\nWiFi not connected.");


  lastThingSpeakUpdate = millis();
  lastSmsGpsSent = millis();
  lastSensorRead = millis();
}


// ------------------- Main Loop -------------------
void loop() {
  sim800.loop();


  // GPS Parsing
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }


  unsigned long now = millis();


  // Periodic sensor read
  if (now - lastSensorRead >= sensorReadInterval) {
    lastSensorRead = now;


    float tempC = dht.readTemperature();
    float hum = dht.readHumidity();
    if (isnan(tempC) || isnan(hum)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      float uvVolt = readAnalogVoltage(GUVA_PIN);
      bool rainDetected = (digitalRead(RAIN_DO_PIN) == LOW); // LOW = rain


      Serial.printf("Temp: %.2f C, Hum: %.2f %%, UV: %.3f V, Rain: %s\n",
                    tempC, hum, uvVolt, rainDetected ? "YES" : "NO");


      bool overTemp = tempC >= TEMP_THRESHOLD_C;
      bool overHum = hum >= HUMIDITY_THRESHOLD;
      bool overUV = uvVolt >= UV_VOLTAGE_THRESHOLD;


      if (overTemp || overHum || overUV || rainDetected) {
        String reason = "";
        if (overTemp) reason += "TEMP ";
        if (overHum) reason += "HUM ";
        if (overUV) reason += "UV ";
        if (rainDetected) reason += "RAIN ";
        triggerAlert(reason);
      }
    }
  }


  // ThingSpeak update
  if (now - lastThingSpeakUpdate >= THINGSPEAK_UPDATE_INTERVAL_MS) {
    lastThingSpeakUpdate = now;
    double lat = gps.location.isValid() ? gps.location.lat() : 0.0;
    double lng = gps.location.isValid() ? gps.location.lng() : 0.0;


    float tempC = dht.readTemperature();
    float hum = dht.readHumidity();
    if (isnan(tempC) || isnan(hum)) {
      Serial.println("Failed to read from DHT sensor! Skipping ThingSpeak update.");
    } else {
      float uvVolt = readAnalogVoltage(GUVA_PIN);
      bool rainDetected = (digitalRead(RAIN_DO_PIN) == LOW);
      updateThingSpeak(tempC, hum, uvVolt, rainDetected, lat, lng);
    }
  }


  // GPS SMS updates
  maybeSendGpsSms();


  delay(10);
}
