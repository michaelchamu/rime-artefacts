#include <BleKeyboard.h>  // Include the BLE keyboard library
#include <Wire.h>

BleKeyboard bleKeyboard("ESP32 Volume", "ESP32 Manufacturer", 100);  // Initialize BLE keyboard with device name and other parameters
Trill trillSensor;

const unsigned int NUM_TOTAL_PADS = 30;
CustomSlider::WORD rawData[NUM_TOTAL_PADS];

const uint8_t sliderNumPads = 30;

uint8_t sliderPads[sliderNumPads] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
const unsigned int numSliders = 1;
CustomSlider slider;
int lastTouched = 0;

void setup() {
  Serial.begin(115200);

  int ret;
  while (trillSensor.setup(Trill::TRILL_CRAFT)) {
    Serial.println("failed to initialise trillSensor");
    Serial.println("Retrying...");
    delay(100);
  }
  Serial.println("Success initialising trillSensor");
  trillSensor.setMode(Trill::DIFF);
  // We recommend a prescaler value of 4
  trillSensor.setPrescaler(4);
  // Experiment with this value to avoid corss talk between sliders if they are position close together
  trillSensor.setNoiseThreshold(200);

  bleKeyboard.begin();  // Start BLE keyboard
}

void loop() {
  // Read 20 times per second
  delay(50);
  if (!trillSensor.requestRawData()) {
    Serial.println("Failed reading from device. Is it disconnected?");
    return setup();
  }
  if (bleKeyboard.isConnected()) {
    unsigned n = 0;
    // read all the data from the device into a local buffer
    while (trillSensor.rawDataAvailable() > 0 && n < NUM_TOTAL_PADS) {
      rawData[n++] = trillSensor.rawDataRead();
    }
    for (uint8_t n = 0; n < numSliders; ++n) {
      // have each custom slider process the raw data into touches
      slider.process(rawData);
      if (slider.getNumTouches() > 0) {
        for (int i = 0; i < slider.getNumTouches(); i++) {
          if (lastTouched > slider.touchLocation(i)) {
            Serial.println("Volume Up");
            bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
            
          } else if (lastTouched < slider.touchLocation(i)) {
            Serial.println("Volume Down");
            bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
          } else
            Serial.println("No change");
          lastTouched = slider.touchLocation(i);
        }
      }
    }
    Serial.println("");
  } 
}

