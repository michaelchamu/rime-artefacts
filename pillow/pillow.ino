#define BMP280_ADDRESS_ALT (0X60)
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Trill.h>
#include <WiFi.h>
#include "secrets.h"
#include <ArduinoHA.h>
#include <ArduinoMqttClient.h>


#define BROKER_ADDR IPAddress(127, 0, 0, 1)

byte mac[] = { 0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A };

WiFiClient client;
//HADevice device(mac, sizeof(mac));
MqttClient mqttClient(client);
const char broker[] = "192.168.0.213";
int port = 1883;
const char power[] = "power";
const char color[] = "color";

Trill trillSensor;

// arrays holding information about each touch sensing channel
int sensorValues[30];           // raw values from the touch sensors
int touchThresholds[30];        // threshold for what counts as a touch
bool sensorTouches[30];         // true if currently above touch threshold
bool prevSensorTouches[30];     // previous frame of touches
unsigned long touchTimers[30];  // when a pin is first touched
bool heldToTime[30];            // if pin has been held to timer
unsigned long timeLastTap[30];  // time when the last tap occurred

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  // the WiFi radio's status

//setup barometer
Adafruit_BMP280 bmp;  // use I2C interface
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

void setup() {
  // Initialise serial and touch sensor
  //initialise barometer
  if (setupWiFi()) {
    if (setpMQTT()) {
      initialiseBarometer();
      intialiseTrill();
    }
  }
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
  mqttClient.poll();
  // Request the latest touch data from Trill
  getSensorData();

  sensors_event_t pressure_event;
  bmp_pressure->getEvent(&pressure_event);
  //if both sides of pillow are held and pressure has increased from the standard 1018 then send request to dashboard to increase temp
  if (isHeldTimer(29, 1500) && isHeldTimer(0, 1500) && pressure_event.pressure > 1018) {
    Serial.println("Both sides of pillow held and squeezed");
    //check pressure increase
    int pressureDifference = pressure_event.pressure - 1018;
    checkPressureSendValue(pressureDifference);
    //each increase by 6 is equal to 1 unit
    //check new value of pressure and determine by how much it is above 1013 and below 1050
    //if its 0-6 then send 1 and vibrate once
    //if its 7-16 then send 2 and vibrate twice
    //......
    //
    //make mqtt request
  }
}

//setup WiFi
bool setupWiFi() {
  // connect to wifi
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  return true;
}
//
bool setupMQTT() {
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  mqttClient.setUsernamePassword("***REMOVED***", "***REMOVED***");
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient);

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  return true;
}
//setup Trill
void setupTrill() {
  Serial.begin(9600);
  Serial.println("Initialising Trill");
  delay(2000);


  // Initialse Trill and save returned value
  int ret = trillSensor.setup(Trill::TRILL_CRAFT);
  delay(1000);

  //If returned value isn't 0, something went wrong
  if (ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.print("Error code: ");
    Serial.println(ret);
  }

  //Make sure Trill Craft is set to differential mode
  trillSensor.setMode(Trill::DIFF);

  // Fill the arrays with starting data
  for (int i = 0; i < 30; i++) {
    sensorValues[i] = 0;
    touchThresholds[i] = 400;
    sensorTouches[i] = false;
    prevSensorTouches[i] = false;
    touchTimers[i] = 0;
    heldToTime[i] = false;
    timeLastTap[i] = 0;
  }

  // Initialise the HID keyboard
  // Sends a clean report to the host. This is important on any Arduino type.
}
//barometer initialise method
void initialiseBarometer() {
  Serial.begin(9600);
  while (!Serial) delay(100);  // wait for native usb
  Serial.println(F("BMP280 Sensor event test"));

  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  // status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x");
    Serial.println(bmp.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
}
//get sensor data
void getSensorData() {
  // Request the latest sensor values from the chip
  trillSensor.requestRawData();

  // start with the first channel, number 0
  int sensorChannel = 0;

  // if there is data available
  if (trillSensor.rawDataAvailable() > 0) {
    // read in the data
    while (trillSensor.rawDataAvailable() > 0) {
      sensorValues[sensorChannel] = trillSensor.rawDataRead();
      // increase by one to read next channel
      sensorChannel++;
    }
  }

  // save the touches from that last data read as previous touches
  for (int pin = 0; pin < 30; pin++) {
    prevSensorTouches[pin] = sensorTouches[pin];
  }

  // update whether a pin has gone above the touch threshold
  for (int pin = 0; pin < 30; pin++) {
    if (sensorValues[pin] > touchThresholds[pin]) {
      sensorTouches[pin] = true;
    } else {
      sensorTouches[pin] = false;
    }
  }

  // save the current time for the timers
  unsigned long currTime = millis();

  // if the touch is new, save the current time
  for (int pin = 0; pin < 30; pin++) {
    if (sensorTouches[pin] && !prevSensorTouches[pin]) {
      touchTimers[pin] = currTime;
    }
  }
}
//is held timer
bool isHeldTimer(int pin, int minTime) {
  // check if previously was held to time and hasn't been released
  if (heldToTime[pin] && sensorTouches[pin]) {
    return false;
  }
  // check if the pin has been actively touched for at least the minimum time in ms
  unsigned long elapsedTime = millis() - touchTimers[pin];
  if (sensorTouches[pin] && elapsedTime > minTime) {
    heldToTime[pin] = true;
    return true;
  }
  // if sensor has been released, reset the held to time flag
  if (!sensorTouches[pin])
    heldToTime[pin] = false;
  // otherwise
  return false;
}
//checkpressuresend value
void checkPressureSendValue(int pressureDifference) {
  //each increase by 6 is equal to 1 unit
  //check new value of pressure and determine by how much it is above 1013 and below 1050
  //if its 0-6 then send 1 and vibrate once
  //if its 7-16 then send 2 and vibrate twice
  //......
  //
  //make mqtt request
  if (pressureDifference > 1 && pressureDifference <= 6) {
    //pulse 1 vibrate
    SerialPrintln("Send 1");
  } else if (pressureDifference > 6 && pressureDifference <= 12) {
    //pulse 2
    SerialPrintln("Send 2");
  } else if (pressureDifference > 12 && pressureDifference <= 18) {
    //pulse 3
    SerialPrintln("Send 3");
  } else if (pressureDifference > 18 && pressureDifference <= 24) {
    SerialPrintln("Send 4");
  } else if (pressureDifference > 24 && pressureDifference <= 30) {
    SerialPrintln("Send 5");
  } else if (pressureDifference > 30 && pressureDifference <= 36) {
    SerialPrintln("Send 6");
  }
}
