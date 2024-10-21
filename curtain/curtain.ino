#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "secrets.h"

#define motorPin A5

int sensorPin = A0;  // Potentiometer input pin
int sensorValue = 0, pastSensorValue = 0;
int currVal = 0, pastVal = 0;
unsigned long previousMillis = 0;
unsigned long motorStartTime = 0;
const unsigned long motorDuration = 1000;  // Duration for which the motor will run (in milliseconds)
bool motorRunning = false;
// Variables to track stabilization
unsigned long lastStableTime = 0;
unsigned long stableThreshold = 500;  // Time (ms) for the value to be stable
int stableVal = -1;                   // Store the final stable value

WiFiClient client;
MqttClient mqttClient(client);

// MQTT config and connection credentials
//const char broker[] = BROKER;
int port = 1883;
const char topic[] = "blinds";


void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  delay(2000);
  initWiFi();
  pinMode(motorPin, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  mqttClient.poll();

  // Read sensor value
  pastSensorValue = sensorValue;
  sensorValue = map(analogRead(sensorPin), 0, 1023, 0, 255);

  // Ignore small fluctuations in sensor values
  if (abs(sensorValue - pastSensorValue) < 6) {
    sensorValue = pastSensorValue;
  }

  // Map sensor value to currVal based on defined ranges
  if (sensorValue > 0 && sensorValue <= 134)
    currVal = 1;
  else if (sensorValue >= 135 && sensorValue <= 165)
    currVal = 2;
  else if (sensorValue >= 166 && sensorValue <= 225)
    currVal = 3;
  else if (sensorValue >= 226 && sensorValue <= 255)
    currVal = 4;
  else
    currVal = pastVal;

  // Check if the current value has changed and has been stable for a certain duration
  if (currVal != pastVal) {
    pastVal = currVal;
    lastStableTime = currentMillis;  // Reset the stability timer
  } else if (currentMillis - lastStableTime >= stableThreshold && stableVal != currVal) {
    // Value has been stable for more than stableThreshold time
    stableVal = currVal;

    // Trigger the motor and start the timer
    motorRunning = true;
    motorStartTime = currentMillis;
    digitalWrite(motorPin, HIGH);

    // Send the final stable value via MQTT
    sendMqttRequest(stableVal);
    Serial.print("MQTT Stable value: ");
    Serial.println(stableVal);
  }

  // Handle non-blocking motor control
  if (motorRunning && currentMillis - motorStartTime >= motorDuration) {
    digitalWrite(motorPin, LOW);  // Turn off the motor
    motorRunning = false;         // Reset the motor running flag
  }

  // Additional non-blocking tasks can be added here
}


void sendMqttRequest(int value) {
  mqttClient.beginMessage("blinds");
  mqttClient.print(value);
  mqttClient.endMessage();
  Serial.println("method called: " + mqttClient.getWriteError());
  // Print the current value to Serial for debugging
  Serial.println(value);
}

void initWiFi() {
  // Scan for available networks
  int numNetworks = WiFi.scanNetworks();
  if (numNetworks == 0) {
    Serial.println("No Wi-Fi networks found");
  } else {
    Serial.println("Available Wi-Fi networks:");
    // Print all found SSIDs
    for (int i = 0; i < numNetworks; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(WiFi.SSID(i));
    }

    // Look for the desired SSID (from secrets.h)
    bool ssidFound = false;
    for (int i = 0; i < numNetworks; i++) {
      if (strcmp(SECRET_SSID, WiFi.SSID(i)) == 0) {
        ssidFound = true;
        Serial.print("Matching SSID found: ");
        Serial.println(SECRET_SSID);

        // Attempt to connect to the network
        WiFi.begin(SECRET_SSID, SECRET_PASS);
        Serial.print("Connecting to ");
        Serial.println(SECRET_SSID);

        // Wait for connection to be established
        int attemptCount = 0;
        while (WiFi.status() != WL_CONNECTED && attemptCount < 20) {
          Serial.print('.');
          delay(500);
          attemptCount++;
        }
        // Check if connected
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to Wi-Fi");
          Serial.print("Attempting to connect to the MQTT broker: ");
          Serial.println(MQTT_BROKER);
          mqttClient.setUsernamePassword(MQTT_UNAME, MQTT_PWORD);
          if (!mqttClient.connect(MQTT_BROKER, port)) {
            Serial.print("MQTT connection failed! Error code = ");
            Serial.println(mqttClient);

            while (1)
              ;
          }

          Serial.println("You're connected to the MQTT broker!");
        } else {
          Serial.println();
          Serial.println("Failed to connect to Wi-Fi");
        }
        break;
      }
    }
    if (!ssidFound) {
      Serial.println("Desired SSID not found in scan results");
    }
  }
}