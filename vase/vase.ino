#include <BleKeyboard.h>  // Include the BLE keyboard library

BleKeyboard bleKeyboard("ESP32 Volume Control", "ESP32 Manufacturer", 100);  // Initialize BLE keyboard with device name and other parameters

int buttonState1 = 0;       // Variable for reading the status of the button1
int buttonState2 = 0;       // Variable for reading the status of the button2

void setup() {
  Serial.begin(115200);
  
  bleKeyboard.begin();  // Start BLE keyboard
}

void loop() {
  if(bleKeyboard.isConnected()) {  // Check if BLE is connected to the phone
    
    buttonState1 = touchRead(T1);  // Read the state of the button1
    buttonState2 = touchRead(T2);  // Read the state of the button2

    if (buttonState1 > 120000) {  // Check if button1 is pressed
      Serial.println("Volume Up");
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);  // Send volume up command
      delay(200);  // Delay to debounce button
    }
    
    if (buttonState2 > 120000) {  // Check if button2 is pressed
      Serial.println("Volume Down");
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);  // Send volume down command
      delay(200);  // Delay to debounce button
    }
  }
  delay(10);  // Short delay to prevent overwhelming the loop
}

void vibrate()
{
  
}