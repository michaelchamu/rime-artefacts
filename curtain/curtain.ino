#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "secrets.h"

int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
int previousValue = 0;
int currVal = 0, pastVal = 0;
// WiFi login and connection credentials
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;
int wificonnect;
WiFiClient wifiClient;

// mqtt config and connection credentials
const char broker[] = BROKER;
int port = PORT;
const char topic[] = "michael-run"; //"blinds";

MqttClient mqttClient(wifiClient);

void setup() {
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

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  
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
  // configure WiFi
  // wificonnect = configureWiFi();
  // // Configure MQTT
  // if (wificonnect == 1) {
  //   configureMQTT();
  // } else {
  //   wificonnect = configureWiFi();
  // }
  //configure rotary knob
  // r.setChangedHandler(rotate);
  // r.setLeftRotationHandler(showDirection);
  // r.setRightRotationHandler(showDirection);
}

void loop() {
  mqttClient.poll();

 sensorValue = map(analogRead(sensorPin), 0, 1023, 0, 255);
// Serial.println(sensorValue);

  if(sensorValue > 0 && sensorValue <= 134)
    currVal = 1;
  if(sensorValue >= 135 && sensorValue <= 165)
    currVal = 2;
  if(sensorValue >= 166 && sensorValue <= 225)
    currVal = 3;
  if(sensorValue >= 226 && sensorValue <= 255)
    currVal = 4;
  if (currVal != pastVal) {
    // update the previous value
    pastVal = currVal;
    // display the current value
    Serial.println(pastVal);
     mqttClient.beginMessage(topic);
     mqttClient.print(pastVal);
     mqttClient.endMessage();
  }
}

int configureWiFi() {

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
    return 0;
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
    return 0;
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
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
void configureMQTT() {

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
}

// void sendSignal(int value) {
//   //pulse vibrate
//  Serial.println("here");
//       mqttClient.beginMessage("blinds");
//       mqttClient.print(2);
//       mqttClient.endMessage();
//    Serial.println("there");
 
// }

// on change
void rotate(Rotary& r) {
   
   //todo uncomment
   Serial.println(r.getPosition());
    if(r.getPosition() > 0 && r.getPosition() <= 5)
   {
    Serial.println(r.getPosition());
    mqttClient.beginMessage("michael-stop");
    mqttClient.print("hello");
    mqttClient.endMessage();
   }
    else if(r.getPosition() > 5 && r.getPosition() <= 10)
   {
    Serial.println(r.getPosition());
    mqttClient.beginMessage("michael-run");
    mqttClient.print("nope");
    mqttClient.endMessage();
   }
  //  if(r.getPosition() > 0 && r.getPosition() <= 5)
  //  {
  //   Serial.println(r.getPosition());
  //   // mqttClient.beginMessage("blinds");
  //   // mqttClient.print(4);
  //   // mqttClient.endMessage();
  //  }
    // mqttClient.beginMessage("blinds");
    // mqttClient.print(r.getPosition());
    // mqttClient.endMessage();
}

// on left or right rotattion
void showDirection(Rotary& r) {
    if(r.getPosition() > 0 && r.getPosition() <= 5)
   {
  Serial.println(r.directionToString(r.getDirection()));}
}


// vibrate
void vibrate(int duration) {
}