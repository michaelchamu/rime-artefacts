#include <BleKeyboard.h>  // Include the BLE keyboard library
#include <Wire.h>
#include "Adafruit_DRV2605.h"

BleKeyboard bleKeyboard("ESP32 Volume Control", "ESP32 Manufacturer", 100);  // Initialize BLE keyboard with device name and other parameters

int buttonState1 = 0;  // Variable for reading the status of the button1
int buttonState2 = 0;  // Variable for reading the status of the button2

void setup() {
  Serial.begin(115200);

  bleKeyboard.begin();  // Start BLE keyboard

  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1) delay(10);
  }

  drv.selectLibrary(1);

  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
}

void loop() {
  if (bleKeyboard.isConnected()) {  // Check if BLE is connected to the phone

    if (touchRead(T1) > 120000) {  // Check if button1 is pressed
      Serial.println("Volume Up");
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);  // Send volume up command
      drv.setWaveform(0, 82);                  // play effect
      drv.setWaveform(1, 0);                   // end waveform

      // play the effect!
      drv.go();
      delay(200);  // Delay to debounce button
    }

    if (touchRead(T2) > 120000) {  // Check if button2 is pressed
      Serial.println("Volume Down");
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);  // Send volume down command
       drv.setWaveform(0, 71);                  // play effect
      drv.setWaveform(1, 0);                   // end waveform

      // play the effect!
      drv.go();
      delay(200);                                // Delay to debounce button
    }
  }
  delay(10);  // Short delay to prevent overwhelming the loop
}

void vibrate(int effect) {
  //if lowering volume, vibrate from high pitch going lower
  //if increasing volume, vibrate from low pitch going higher
  drv.setWaveform(0, effect);  // play effect
  drv.setWaveform(1, 0);       // end waveform

  // play the effect!
  drv.go();
}