#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "secrets.h"

// WiFi login and connection credentials
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;
int wificonnect;
WiFiClient wifiClient;

// mqtt config and connection credentials
const char broker[] = BROKER;
int port = PORT;
const char topic[] = "blinds";
// set interval for sending messages (milliseconds)
const long interval = 8000;
unsigned long previousMillis = 0;
int count = 0;
bool firstRun = false;
MqttClient mqttClient(wifiClient);

// flex sensor config values
const int FLEX_PIN = A0;
const float VCC = 3.289;      // Measured voltage of Ardunio 3.3V line on Arduino nano IoT
const float R_DIV = 100600.0; // Measured resistance of 10k resistor
// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 700.0; // 1004;//855;  //998;// // resistance when straight
const float BEND_RESISTANCE = 1023.0;    // 934;//747;      //957;//    // resistance at 90 deg

int state;

// motor pins
const int motorPin = 3;

void setup()
{
  // initialize serial communications
  Serial.begin(9600);
  // configure WiFi
  wificonnect = configureWiFi();
  // Configure MQTT
  if (wificonnect == 1)
  {
    configureMQTT();
  }
  else
  {
    wificonnect = configureWiFi();
  }
  // Set Flex sensor defaults
  pinMode(FLEX_PIN, INPUT);
  // set vibration pin
  pinMode(motorPin, OUTPUT);
}

void loop()
{
  mqttClient.poll();

  int flexADC = analogRead(FLEX_PIN);
  Serial.println("FLEX: " + String(flexADC));
  float flexV = flexADC * VCC / 1023.0;
  float flexResistance = R_DIV * (VCC / flexV - 1.0);

  Serial.println("Resistance: " + String(flexResistance) + " ohms");
  // Use the calculated resistance to estimate the sensor's
  // bend angle:
  float angle = map(flexADC, STRAIGHT_RESISTANCE, BEND_RESISTANCE,
                    0, 90.0);
  Serial.println("Bend: " + String(angle) + " degrees");
  Serial.println();
  // flexR = measured resistance * (voltage/)
  bend(flexResistance, angle);
  delay(1500);
  /// consider removing everything below this line
}

int configureWiFi()
{

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
    return 0;
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
    return 0;
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  return 1;
}

// mqtt configuration function is called in setup function once
void configureMQTT()
{
  digitalWrite(motorPin, HIGH); // vibrate
  delay(1000);                  // delay one second
  digitalWrite(motorPin, LOW);  // stop vibrating
  delay(1000);                  // wait 50 seconds.

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  mqttClient.setUsernamePassword("***REMOVED***", "***REMOVED***");
  if (!mqttClient.connect(broker, port))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient);

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

// handle all flex sensor folds
void bend(float resistance, float angle)
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    // save the last time a message was sent
    previousMillis = currentMillis;

    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.println("0. Resistance: " + String(resistance) + " - " + String(angle) + " - State: " + String(state));

    // FULLY STRAIGHT Resistance between 20-21k? Depends on resistor used
    // when conditions are met, a MQTT Message is sent to HomeAssistant which then handles the curtain control and opening etc
    if (resistance >= 20000.0 && resistance < 21000 && angle >= 0 && angle <= 10)
    {
      // if(flex >= 850 && flex <= 890){
      // message to be sent is 0, therefore check current value of state
      Serial.println("1. Resistance: " + String(resistance) + " - " + String(angle) + " - State: " + String(state));
      Serial.print("Making MQTT request state 0");
      // send mqtt request
      //  send message, the Print interface can be used to set the message contents
      mqttClient.beginMessage(topic);
      mqttClient.print(0);
      mqttClient.endMessage();
    }
    // slightly bent
    if (resistance > 21000.0 && angle > 10)
    {

      Serial.println("2. Resistance: " + String(resistance) + " - " + String(angle) + " - State: " + String(state));
      Serial.print("Making MQTT request state 5");
      mqttClient.beginMessage(topic);
      mqttClient.print(5);
      mqttClient.endMessage();
      // set state to 0 again
    }
  }
}

// vibrate
void vibrate(int duration)
{
}