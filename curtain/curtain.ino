
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <Arduino_MultiWiFi.h>
#include "secrets.h"
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;

int sensorPin = A0;                        // select the input pin for the potentiometer
int sensorValue = 0, pastSensorValue = 0;  // variable to store the value coming from the sensor
int previousValue = 0;
int currVal = 0, pastVal = 0;
unsigned long currentTime = millis();
long previousMillis = 0;
long interval = 1000;

WiFiClient wifiClient;
MultiWiFi multi;

// mqtt config and connection credentials
const char broker[] = BROKER;
int port = PORT;
const char topic[] = "blinds";

MqttClient mqttClient(wifiClient);

void setup() {
  multi.add(SECRET_SSID, SECRET_PASS);
  multi.add(OFFIS_SECRET_SSID, OFFIS_SECRET_PASS);
  if (multi.run() == WL_CONNECTED) {
    Serial.print("Successfully connected to network: ");
    Serial.println(WiFi.SSID());
  } else {
    Serial.println("Failed to connect to a WiFi network");
  }
  // initialize serial communications
  Serial.begin(9600);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  if (WiFi.SSID() == SECRET_SSID) {
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(BROKER);
    mqttClient.setUsernamePassword("***REMOVED***", "***REMOVED***");
    if (!mqttClient.connect(BROKER, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient);

      while (1)
        ;
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
  } else {
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(OFFIS_BROKER);
    mqttClient.setUsernamePassword("***REMOVED***", "***REMOVED***");
    if (!mqttClient.connect(OFFIS_BROKER, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient);

      while (1)
        ;
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
  }

  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(6);

  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
}

void loop() {
  unsigned long currentMillis = millis();
  mqttClient.poll();
  pastSensorValue = sensorValue;
  sensorValue = map(analogRead(sensorPin), 0, 1023, 0, 255);
  if (abs(sensorValue - pastSensorValue) < 6) sensorValue = pastSensorValue;
  // Serial.println(sensorValue);

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


  if (currVal != pastVal) {

    // update the previous value
    pastVal = currVal;
    // set the effect to play
    if (currentMillis - previousMillis > interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      drv.setWaveform(0, 14);  // play effect
      drv.setWaveform(1, 0);   // end waveform
      // play the effect!
      drv.go();
    }
    Serial.println(currVal);
    // display the current valu
    //vibrate currVal times
    mqttClient.beginMessage(topic);
    mqttClient.print(pastVal);
    mqttClient.endMessage();
  }
}