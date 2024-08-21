//set altitude
//connect to arduino cloud


//READ TOUCHES
//read stretch resistor values
//send values in real time to cloud

// BMP180_I2C.ino
//
// shows how to use the BMP180MI library with the sensor connected using I2C.
//
// Copyright (c) 2018 Gregor Christandl
//
// connect the BMP180 to the Arduino like this:
// Arduino - BMC180
// 5V ------ VCC
// GND ----- GND
// SDA ----- SDA
// SCL ----- SCL

#include <Arduino.h>
#include <Wire.h>

#include <BMP180I2C.h>
#include "Adafruit_DRV2605.h"
Adafruit_DRV2605 drv;

#define I2C_ADDRESS 0x77
//create an BMP180 object using the I2C interface
BMP180I2C bmp180(I2C_ADDRESS);
const int touchPin1 = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //wait for serial connection to open (only necessary on some boards)
  while (!Serial)
    ;

  Wire.begin();

  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(1);

  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);

  //begin() initializes the interface, checks the sensor ID and reads the calibration parameters.
  if (!bmp180.begin()) {
    Serial.println("begin() failed. check your BMP180 Interface and I2C Address.");
    while (1)
      ;
  }

  //reset sensor to default parameters.
  bmp180.resetToDefaults();

  //enable ultra high resolution mode for pressure measurements
  bmp180.setSamplingMode(BMP180MI::MODE_UHR);
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);

  //start a temperature measurement
  if (!bmp180.measureTemperature()) {
    Serial.println("could not start temperature measurement, is a measurement already running?");
    return;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true.
  do {
    delay(100);
  } while (!bmp180.hasValue());


  //start a pressure measurement. pressure measurements depend on temperature measurement, you should only start a pressure
  //measurement immediately after a temperature measurement.
  if (!bmp180.measurePressure()) {
    Serial.println("could not start perssure measurement, is a measurement already running?");
    return;
  }

  //wait for the measurement to finish. proceed as soon as hasValue() returned true.
  do {
    delay(100);
  } while (!bmp180.hasValue());

  Serial.print("Pressure: ");
  Serial.print(bmp180.getPressure());
  Serial.println(" Pa");
  int touchValue = touchRead(touchPin1);
  //if(touchValue > 120000 && touchValue2 > 120000 && bmp180.getPressure() > 130000) //both pins are touched and pillow is squeezed, so send value to meter
  if (touchValue > 120000) {
    Serial.print("Touch Value 1: ");
    Serial.println(touchValue);
    // set the effect to play
    drv.setWaveform(0, 14);  // play effect
    drv.setWaveform(1, 0);       // end waveform

    // play the effect!
    drv.go();
  }
}
