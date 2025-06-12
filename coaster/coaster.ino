#include <Wire.h>
#include <Trill.h>
#include <WiFi.h>
#include "secrets.h"
#include <ArduinoMqttClient.h>

#define motorPin 3  // Define the GPIO pin connected to the vibromotor

unsigned long vibrationStartTime = 0;
unsigned long vibrationDuration = 800;  // Vibration duration in milliseconds (1 second)
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
      // Print SSID and RSSI for each network found
        WiFi.mode(WIFI_STA);
        //WiFi.begin(SECRET_SSID, SECRET_PASS);
        WiFi.begin(ssid, pass);
        Serial.print("Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print('.');
          delay(1000);
        }
        Serial.println("You're connected to the network");
        Serial.println();
        Serial.println(WiFi.localIP());
        Serial.print("Connecting to broker ");
        Serial.print(MQTT_BROKER);
        mqttClient.setUsernamePassword(MQTT_UNAME, MQTT_PWORD);
        if (!mqttClient.connect(MQTT_BROKER, port)) {
          Serial.print("MQTT connection failed! Error code = ");
          Serial.print("MQTT error: ");
          Serial.println(mqttClient.connectError());

          while (1)
            ;
        }

        Serial.println("You're connected to the MQTT broker!");
      
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
}

void loop() {
  delay(50);
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
  /*
| Color        | Number |
|--------------|--------|
| pink         | 0,11   |
| brown        | 1,15   |
| olive green  | 2      |
| maroon       | 3      |
| yellow       | 4,29   |
| orange       | 7,21,24|
| red          | 9      |
| navy blue    | 12     |
| sky blue     | 14     |
| white        | 17     |
| turquouse    | 18     |
| green        | 19     |
| purple       | 22     |
*/

  for (int pin = 0; pin < 30; pin++) { 
    if (isHeldTimer(pin, 10)) {
      switch (pin) {
        case 11:
          mqttClient.beginMessage(color);
          mqttClient.print(0);
          mqttClient.endMessage();
          break;
        case 15:
          mqttClient.beginMessage(color);
          mqttClient.print(1);
          mqttClient.endMessage();
          break;
        case 21:
        case 24:
          mqttClient.beginMessage(color);
          mqttClient.print(7);
          mqttClient.endMessage();
          break;
        case 29:
          mqttClient.beginMessage(color);
          mqttClient.print(4);
          mqttClient.endMessage();
          break;
        default:
          mqttClient.beginMessage(color);
          mqttClient.print(pin);
          mqttClient.endMessage();
          break;
      }
    }
  }
  if (isDoubleTapped(6)) {
    //power
    mqttClient.beginMessage(power);
    mqttClient.print(6);
    mqttClient.endMessage();
  }

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
    digitalWrite(motorPin, HIGH); 
    timeLastTap[pin] = millis();
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
    startVibration(500);
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
  digitalWrite(motorPin, HIGH);   // Turn on the vibration motor
  vibrationStartTime = millis();  // Store the current time
  vibrationDuration = duration;   // Set the duration for vibration
  isVibrating = true;             // Set the flag to indicate vibration is on
}

void updateVibration() {
  // If vibration is active and the duration has passed, stop the motor
  if (isVibrating && (millis() - vibrationStartTime >= vibrationDuration)) {
    digitalWrite(motorPin, LOW);  // Turn off the vibration motor
    isVibrating = false;          // Reset the flag
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