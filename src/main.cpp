#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "credentials.h"  // Include the credentials file

// Single color LED pins
#define RED_LED_PIN D5
#define GREEN_LED_PIN D6
#define BLUE_LED_PIN D7

//  USER CONFIG 
// Distance thresholds (km)
const double VISIBLE_RADIUS   = 500.0;
const double SLIGHTLY_FAR_RADIUS = 1000.0;

// API endpoint
const char* ISS_API_URL = "http://api.open-notify.org/iss-now.json";

// Timing constants
const unsigned long CHECK_ISS_INTERVAL = 30000;   // Check ISS position every 30 seconds
const unsigned long WIFI_CHECK_INTERVAL = 5000;   // Check WiFi connection every 5 seconds
const unsigned long BLINK_FAST = 200;             // Fast blink interval in ms
const unsigned long BLINK_MEDIUM = 500;           // Medium blink interval in ms

// Global variables for non-blocking operation
unsigned long lastISSCheck = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastLEDUpdate = 0;
int currentLEDState = 0;
int ledMode = 0;  // 0 = off, 1 = steady red, 2 = blinking blue, 3 = fast blinking green

//  HELPER FUNCTIONS 
double deg2rad(double deg) {
  return deg * (PI / 180.0);
}

double haversine(double lat1, double lon1, double lat2, double lon2) {
  double dLat = deg2rad(lat2 - lat1);
  double dLon = deg2rad(lat2 - lon1);
  lat1 = deg2rad(lat1);
  lat2 = deg2rad(lat2);

  double a = sin(dLat/2) * sin(dLat/2) +
             sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  const double R = 6371.0; // Earth radius in km
  return R * c;
}

// Function to ensure WiFi connection
void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconnecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    // Wait up to 10 seconds for connection
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconnected!");
    } else {
      Serial.println("\nFailed to reconnect!");
    }
  }
}

// Function to update LED states non-blockingly
void updateLEDs() {
  unsigned long currentMillis = millis();
  
  switch (ledMode) {
    case 0:  // All LEDs off
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      break;
      
    case 1:  // Steady Red (ISS is far away)
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      break;
      
    case 2:  // Blinking Blue (ISS is approaching)
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      
      if (currentMillis - lastLEDUpdate >= BLINK_MEDIUM) {
        lastLEDUpdate = currentMillis;
        currentLEDState = !currentLEDState;
        digitalWrite(BLUE_LED_PIN, currentLEDState);
      }
      break;
      
    case 3:  // Fast Blinking Green (ISS is nearby/visible)
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      
      if (currentMillis - lastLEDUpdate >= BLINK_FAST) {
        lastLEDUpdate = currentMillis;
        currentLEDState = !currentLEDState;
        digitalWrite(GREEN_LED_PIN, currentLEDState);
      }
      break;
  }
}

// Function to check ISS position and update LED mode
void checkISSPosition() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, ISS_API_URL); 

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      StaticJsonDocument<256> doc;
      DeserializationError err = deserializeJson(doc, payload);

      if (!err) {
        double issLat = doc["iss_position"]["latitude"].as<double>();
        double issLon = doc["iss_position"]["longitude"].as<double>();
        double dist = haversine(MY_LAT, MY_LON, issLat, issLon);

        Serial.print("ISS Position: ");
        Serial.print(issLat);
        Serial.print(", ");
        Serial.println(issLon);
        
        Serial.print("Distance from your location: ");
        Serial.print(dist);
        Serial.println(" km");

        // Update LED mode based on distance
        if (dist <= VISIBLE_RADIUS) {
          Serial.println("ISS is nearby! (Green LED blinking fast)");
          ledMode = 3;  // Fast blinking green
        }
        else if (dist <= SLIGHTLY_FAR_RADIUS) {
          Serial.println("ISS is approaching (Blue LED blinking)");
          ledMode = 2;  // Blinking blue
        }
        else {
          Serial.println("ISS is far away (Red LED solid)");
          ledMode = 1;  // Steady red
        }
      } else {
        Serial.println("Failed to parse JSON");
      }
    } else {
      Serial.printf("HTTP GET failed, code: %d\n", httpCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected for ISS check!");
  }
}

//  SETUP & LOOP 
void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  
  // Initialize all LEDs to OFF
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check WiFi connection periodically
  if (currentMillis - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = currentMillis;
    ensureWiFiConnected();
  }
  
  // Check ISS position periodically
  if (currentMillis - lastISSCheck >= CHECK_ISS_INTERVAL) {
    lastISSCheck = currentMillis;
    checkISSPosition();
  }
  
  // Update LEDs (non-blocking)
  updateLEDs();
  
  // Small delay to prevent CPU hogging
  delay(10);
}