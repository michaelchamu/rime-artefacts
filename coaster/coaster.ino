/*
   simple-touch-example
*/

#include <Wire.h>
#include <Trill.h>
#include <WiFi.h>
#include "secrets.h"
#include <ArduinoHA.h>
#include <ArduinoMqttClient.h>
#include <driver/mcpwm.h>  //vibration library

#define BROKER_ADDR IPAddress(127, 0, 0, 1)
#define MOTOR_GPIO 3  // Define the GPIO pin connected to the vibromotor

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

void initWiFi() {
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
}

// void initVibration(){
//   // Initialize MCPWM unit
//   mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_GPIO);

//   // Configure MCPWM settings
//   mcpwm_config_t pwm_config;
//   pwm_config.frequency = 1000;    // Frequency in Hz
//   pwm_config.cmpr_a = 0;          // Duty cycle of PWMxA = 0
//   pwm_config.cmpr_b = 0;          // Duty cycle of PWMxB = 0
//   pwm_config.counter_mode = MCPWM_UP_COUNTER; // Up counting mode
//   pwm_config.duty_mode = MCPWM_DUTY_MODE_0;   // Active HIGH

//   // Initialize MCPWM with the above configuration
//   mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
// }

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(9600);
  Serial.println("Initialising Trill");
  delay(2000);

  // connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  initWiFi();
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_GPIO);

  // Configure MCPWM settings
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 1000;                 // Frequency in Hz
  pwm_config.cmpr_a = 0;                       // Duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;                       // Duty cycle of PWMxB = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;  // Up counting mode
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;    // Active HIGH

  // Initialize MCPWM with the above configuration
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  mqttClient.setUsernamePassword("michael", "bosomefi");
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient);

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
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
  if (isTapped(0)) {
    // check status of bulb? on/off
    // if off, set on, if on, set off
    // transmit signal

    Serial.println("0 was tapped");
  }
  if (isTapped(1)) {
    // check status of bulb? on/off
    // if off, set on, if on, set off
    // transmit signal
    Serial.println("2 was tapped");
  }
  if (isTapped(2)) {
    // check status of bulb? on/off
    // if off, set on, if on, set off
    // transmit signal
    Serial.println("2 was tapped");
  }
  if (isTapped(3)) {
    // check status of bulb? on/off
    // if off, set on, if on, set off
    // transmit signal
    Serial.println("3 was tapped");
  }
  if (isTapped(4)) {
    // check status of bulb? on/off
    // if off, set on, if on, set off
    // transmit signal
    Serial.println("4 was tapped");
  }
  if (isTapped(6)) {
    // check status of bulb? on/off
    // if off, set on, if on, set off
    // transmit signal
    Serial.println("6 was tapped");
  }
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
  if (isTapped(7)) {
    Serial.println("7 tapped");
  }
  if (isTapped(9)) {
    Serial.println("9 tapped");
  }
  if (isTapped(11)) {
    Serial.println("11 tapped");
  }
  if (isTapped(12)) {
    Serial.println("12 tapped");
  }
  if (isTapped(14)) {
    Serial.println("14 tapped");
  }
  if (isTapped(15)) {
    Serial.println("15 tapped");
  }
  if (isTapped(17)) {
    Serial.println("17 tapped");
  }
  if (isTapped(18)) {
    Serial.println("18 tapped");
  }
  if (isTapped(19)) {
    Serial.println("19 tapped");
  }
  if (isTapped(21)) {
    Serial.println("21 tapped");
  }
  if (isTapped(22)) {
    Serial.println("22 tapped");
  }
  if (isTapped(24)) {
    Serial.println("24 tapped");
  }
  if (isTapped(29)) {
    Serial.println("28 tapped");
  }
  //-----------------------------------------------------------------------------
  // ^^^^^^^ Only change code above here ^^^^^^^
  //-----------------------------------------------------------------------------
}

bool isBeingTouched(int pin) {
  return sensorTouches[pin];
}

bool isTapped(int pin) {
  //

  // the sensor was touched but has just been released
  // (no longer being touched)
  // and it was touched for less than a half second (500 ms)
  unsigned long elapsedTime = millis() - touchTimers[pin];
  if (!sensorTouches[pin] && prevSensorTouches[pin] && elapsedTime < 500) {
    timeLastTap[pin] = millis();
    //send mqtt request here
    // switch on lights
    //check current status of light from mqtt
    switch (pin) {
      case 0:  //pink
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(0);
        mqttClient.endMessage();
        break;
      case 1:  //brown
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(1);
        mqttClient.endMessage();
        break;
      case 2:  //olive green
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(2);
        mqttClient.endMessage();
        break;
      case 3:  // maroon
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(3);
        mqttClient.endMessage();
        break;
      case 4:  //yellow
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(4);
        mqttClient.endMessage();
        break;
      case 6:  //power on/off
        vibrate(0);
        mqttClient.beginMessage(power);
        mqttClient.print(6);
        mqttClient.endMessage();
        break;
      case 7:  //orange
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(7);
        mqttClient.endMessage();
        break;
      case 9:  //red
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(9);
        mqttClient.endMessage();
        break;
      case 11:  //pink
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(0);
        mqttClient.endMessage();
        break;
      case 12:  //nsvy blue
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(12);
        mqttClient.endMessage();
        break;
      case 14:  //sky blue
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(14);
        mqttClient.endMessage();
        break;
      case 15:  //brown
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(1);
        mqttClient.endMessage();
        break;
      case 17:  //white
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(17);
        mqttClient.endMessage();
        break;
      case 18:  //touquous
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(18);
        mqttClient.endMessage();
        break;
      case 19:  //green
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(19);
        mqttClient.endMessage();
        break;
      case 21:  //orange
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(7);
        mqttClient.endMessage();
        break;
      case 22:  //purple
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(22);
        mqttClient.endMessage();
        break;
      case 24:  //orange
        vibrate(1);
        mqttClient.beginMessage(color);
        mqttClient.print(7);
        mqttClient.endMessage();
        break;
    }
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

void vibrate(int state) {
  // Set duty cycle for PWM signal

  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
  // Apply the duty cycle
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
  if (state == 0)
    delay(700);  // Wait for 1000ms
  else
    delay(200);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
  if (state == 0)
    delay(700);  // Wait for 1000ms
  else
    delay(200);
}

void printSensorData() {
  // Print on a single line the current sensor values
  for (int i = 0; i < 30; i++) {
    Serial.print(sensorValues[i]);
    Serial.print(' ');
  }
  Serial.println("");
}