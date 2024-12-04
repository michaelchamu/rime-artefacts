#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "secrets.h"

#define motorPin A5

int sensorPin = A0;  // Potentiometer input pin
int sensorValue = 0, pastSensorValue = 0;
int currVal = 0, pastVal = 0;
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
  // Map sensor value to currVal based on defined ranges - play around with the ranges to get desired effect
  //2-32, 33-64, 65-95, 96-126, 127-159

  if (sensorValue > 0 && sensorValue <= 32)
    currVal = 1;  //fully close 100%
  else if (sensorValue >= 33 && sensorValue <= 64)
    currVal = 2;  //2/5th open 60% close (100-40)
  else if (sensorValue >= 65 && sensorValue <= 95)
    currVal = 3;  // 3/5th open 40% close (100-60)
  else if (sensorValue >= 96 && sensorValue <= 126)
    currVal = 4;  // 4/5 open 20% close (100-80)
  else if (sensorValue >= 127 && sensorValue <= 159)
    currVal = 5;  //5/5 open 0% close/fully open
  else
    currVal = pastVal;
  // Check if the current value has changed and has been stable for a certain duration
  if (currVal != pastVal) {
    pastVal = currVal;
    lastStableTime = currentMillis;  // Reset the stability timer
  } else if (currentMillis - lastStableTime >= stableThreshold && stableVal != currVal) {
    // Value has been stable for more than stableThreshold time
    stableVal = currVal;
    // Send the final stable value via MQTT
    sendMqttRequest(stableVal, currentMillis);
    Serial.print("Signal value: ");
    Serial.println(sensorValue);
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

void sendMqttRequest(int value, unsigned long currentMillis) {
  // Trigger the motor and start the timer
  motorRunning = true;
  motorStartTime = currentMillis;
  digitalWrite(motorPin, HIGH);
  mqttClient.beginMessage(topic);
  mqttClient.print(value);
  mqttClient.endMessage();
  // Print the current value to Serial for debugging
  Serial.println(value);
}

void initWiFi() {
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
}