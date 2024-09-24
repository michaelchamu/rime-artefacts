#include <Wire.h>
#include <Trill.h>
#include <WiFi.h>
#include "secrets.h"
#include <ArduinoMqttClient.h>

#define motorPin 3  // Define the GPIO pin connected to the vibromotor

unsigned long vibrationStartTime = 0;
unsigned long vibrationDuration = 800; // Vibration duration in milliseconds (1 second)
bool isVibrating = false;

WiFiClient client;
MqttClient mqttClient(client);
//const char broker[] =  //"broker.emqx.io";//"192.168.0.213";
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

void initWiFi() {
  //scan SSID
  //if SSID OFFIS is within range, connect to OFFIS network and use OFFIS broker
  //else use home network and broker
  int scanned = WiFi.scanNetworks();

  if (scanned == 0)
    Serial.println("No networks");
  else {
    Serial.print(scanned);
    Serial.println(" networks found");
    for (int i = 0; i < scanned; ++i) {
      // Print SSID and RSSI for each network found
      if (WiFi.SSID(i) == SECRET_SSID) {  //enter the ssid which you want to search
        WiFi.mode(WIFI_STA);
        WiFi.begin(SECRET_SSID, SECRET_PASS);
        Serial.print("Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print('.');
          delay(1000);
        }
        Serial.println("You're connected to the network");
        Serial.println();
        Serial.println(WiFi.localIP());
        Serial.print("Attempting to connect to the MQTT broker: ");
        Serial.println(MQTT_BROKER);
        mqttClient.setUsernamePassword("***REMOVED***", "***REMOVED***");
        if (!mqttClient.connect(MQTT_BROKER, port)) {
          Serial.print("MQTT connection failed! Error code = ");
          Serial.println(mqttClient);

          while (1)
            ;
        }

        Serial.println("You're connected to the MQTT broker!");
      } else if (WiFi.SSID(i) == OFFIS_SECRET_SSID) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(OFFIS_SECRET_SSID, OFFIS_SECRET_PASS);
        Serial.print("Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print('.');
          delay(1000);
        }
        Serial.println("You're connected to the network");
        Serial.println();
        Serial.println(WiFi.localIP());
        Serial.print("Attempting to connect to the MQTT broker: ");
        Serial.println(OFFIS_MQTT_BROKER);
        mqttClient.setUsernamePassword("***REMOVED***", "***REMOVED***");
        if (!mqttClient.connect(OFFIS_MQTT_BROKER, port)) {
          Serial.print("MQTT connection failed! Error code = ");
          //Serial.println(mqttClient);

          while (1)
            ;
        }

        Serial.println("You're connected to the MQTT broker!");
      }
    }
  }
}


void setup() {
  // Initialise serial and touch sensor
  Serial.begin(9600);
  Serial.println("Initialising Trill");
  delay(2000);

  // connect to wifi
  //WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  //delay(100);

  initWiFi();

  Serial.println();
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
pinMode(motorPin, OUTPUT);
  // Initialise the HID keyboard
  // Sends a clean report to the host. This is important on any Arduino type.
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
  mqttClient.poll();
  // Request the latest touch data from Trill
  getSensorData();
  //-----------------------------------------------------------------------------
  // vvvvvvv Only change code between here vvvvvvv
  //-----------------------------------------------------------------------------
  // Uncomment the line below to print all the latest touch data from Trill
  // printSensorData();

  // Example of pressing the spacebar when pin 0 is tapped
  if (isTapped(0)) 
  if (isTapped(1)) 
  if (isTapped(2))
  if (isTapped(3)) 
  if (isTapped(4)) 
  if (isDoubleTapped(6)) //power
  if (isTapped(7)) 
  if (isTapped(9)) 
  if (isTapped(11)) 
  if (isTapped(12)) 
  if (isTapped(14)) 
  if (isTapped(15)) 
  if (isTapped(17)) 
  if (isTapped(18)) 
  if (isTapped(19)) 
  if (isTapped(21)) 
  if (isTapped(22)) 
  if (isTapped(24)) 
  if (isTapped(29))
  //check for and stop vibration
  updateVibration();
  // Example of tapping pin 26 is tapped while pin 0 is held
  // if (isTapped(26) && isBeingTouched(0)) {
  //   Serial.println("0 is being held and 26 was tapped");
  // }

  // Example of double tapping on pin 27
  // if (isDoubleTapped(27)) {
  //   Serial.println("27 was double tapped");
  // }

  // Example of pin 29 being held for 1500 ms (1.5 seconds)
  // if (isHeldTimer(29, 1500)) {
  //   Serial.println("29 was held to time");
  // }
  
  //-----------------------------------------------------------------------------
  // ^^^^^^^ Only change code above here ^^^^^^^
  //-----------------------------------------------------------------------------
}

bool isBeingTouched(int pin) {
  return sensorTouches[pin];
}

bool isTapped(int pin) {
  // the sensor was touched but has just been released
  // and it was touched for less than a half second (500 ms
  unsigned long elapsedTime = millis() - touchTimers[pin];
  if (!sensorTouches[pin] && prevSensorTouches[pin] && elapsedTime < 500) {
    startVibration(vibrationDuration);
    timeLastTap[pin] = millis();

    mqttClient.beginMessage(color);
    mqttClient.print(pin);
    mqttClient.endMessage();
    return true; 
  }
  // otherwise return false 
  
  return false;
}

bool isDoubleTapped(int pin) {
  // the sensor was tapped twice in quick succession
  // if one tap has happened and less than 50ms has passed since last tap
  unsigned long elapsedTime = millis() - timeLastTap[pin];

  if (isTapped(pin) && elapsedTime < 400) {
      startVibration(300);
      mqttClient.beginMessage(power);
      mqttClient.print(6);
      mqttClient.endMessage();
      
    return true;
  }
  // and a second tap occurs
  // otherwise return false
  return false;
}

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

void startVibration(unsigned long duration) {
  // Serial.println("Start vibe");
  digitalWrite(motorPin, HIGH);     // Turn on the vibration motor
  vibrationStartTime = millis();        // Store the current time
  vibrationDuration = duration;         // Set the duration for vibration
  isVibrating = true;                // Set the flag to indicate vibration is on
}

void updateVibration() {
  // If vibration is active and the duration has passed, stop the motor
  if (isVibrating && (millis() - vibrationStartTime >= vibrationDuration)) {
    digitalWrite(motorPin, LOW);    // Turn off the vibration motor
    isVibrating = false;                // Reset the flag
  }
}

void printSensorData() {
  // Print on a single line the current sensor values
  for (int i = 0; i < 30; i++) {
    Serial.print(sensorValues[i]);
    Serial.print(' ');
  }
  Serial.println("");
}