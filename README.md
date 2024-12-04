```markdown
# Smart Home Control Devices Firmware

This repository contains firmware for four innovative smart home control devices. Each device connects to specific smart home gadgets and allows you to interact with them using creative tangible interfaces. Below, you'll find detailed instructions for setting up each device.

---

## Devices Overview

### 1. **Coaster Device**  
   - **Microcontroller:** Xiao ESP32  
   - **Additional Hardware:** Trill Craft Board  
   - **Smart Home Connection:** Controls a TP-Link smart bulb.  
   - **Functionality:** A tactile interface for lighting control.  

### 2. **Artificial Plant**  
   - **Microcontroller:** Arduino Nano 33 IoT  
   - **Smart Home Connection:** Operates a SwitchBot curtain motor.  
   - **Functionality:** A gesture-based interface for curtain control.  

### 3. **Pillow**  
   - **Microcontroller:** JOYNIT ESP32  
   - **Functionality:** Embedded controls for various smart home features.  

### 4. **Volume Control Vase**  
   - **Microcontroller:** Xiao ESP32S3  
   - **Functionality:** Volume control via a Bluetooth HMI.  
   - **Note:** Tested on Android devices. Compatibility with Apple devices is not guaranteed.  

---

## Prerequisites

### Hardware  
- Corresponding microcontrollers and peripheral components (e.g., Trill Craft for the Coaster).  
- USB cables for programming devices.  

### Software  
- [Arduino IDE](https://www.arduino.cc/en/software).  
- Board support packages installed in Arduino IDE for:  
  - Xiao ESP32 & Xiao ESP32S3  
  - Arduino Nano 33 IoT  
  - ESP32 (for JOYNIT ESP32).  

### Clone the Repository  
```bash
git clone https://github.com/michaelchamu/rime-artefacts.git
cd smart-home-firmware
```

---

## Installation Instructions  

### 1. **Coaster Device**  
   - **Microcontroller:** Xiao ESP32  
   - **Peripheral:** Trill Craft board  
   - **Steps:**  
     1. Open Arduino IDE and add the following URL to **File > Preferences > Additional Board Manager URLs**:  
        ```
        https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
        ```
     2. Install the "Seeed Xiao ESP32" board via the **Board Manager**.  
     3. Connect the Xiao ESP32 via USB and select it as the target board.  
     4. Open `coaster.ino` in the repository.  
     5. Upload the firmware by clicking the **Upload** button.  

### 2. **Artificial Plant**  
   - **Microcontroller:** Arduino Nano 33 IoT  
   - **Steps:**  
     1. Install the "Arduino SAMD Boards" library via the **Board Manager**.  
     2. Connect the Arduino Nano 33 IoT via USB and select it as the target board.  
     3. Open `curtain.ino` in the repository.  
     4. Upload the firmware by clicking the **Upload** button.  

### 3. **Pillow**  
   - **Microcontroller:** JOYNIT ESP32  
   - **Steps:**  
     1. Install the "ESP32 by Espressif Systems" board library via the **Board Manager**.  
     2. Connect the JOYNIT ESP32 via USB and select `ESP32 Dev Module` as the target board.  
     3. Open `pillow.ino` in the repository.  
     4. Upload the firmware by clicking the **Upload** button.  

### 4. **Volume Control Vase**  
   - **Microcontroller:** Xiao ESP32S3  
   - **Steps:**  
     1. Install the "ESP32-S3 by Espressif Systems" board library via the **Board Manager**.  
     2. Connect the Xiao ESP32S3 via USB and select it as the target board.  
     3. Open `vase.ino` in the repository.  
     4. Upload the firmware by clicking the **Upload** button.  
     5. Pair the device with an Android phone via Bluetooth.  
     6. Note: Compatibility with Apple devices may vary.  

---

## Smart Home Device Configuration  

### TP-Link Bulb (Coaster Device)  
Ensure your TP-Link bulb is set up and connected to your home Wi-Fi network via the Kasa app. The firmware includes instructions to control the bulb via specific tactile gestures.  

### SwitchBot Curtain Motor (Artificial Plant)  
Ensure your SwitchBot curtain motor is connected to your home network via the SwitchBot app. The firmware enables gesture-based curtain control.  

---

## Troubleshooting  

- **Device not detected**: Ensure correct drivers are installed for your microcontroller.  
- **Upload errors**: Verify the selected board and COM port in Arduino IDE.  
- **Bluetooth pairing issues (Volume Control Vase)**: Restart the device and ensure Bluetooth is enabled on your phone.  

---

## Contribution  

We welcome contributions to enhance these smart home control devices. Feel free to fork this repository, make changes, and submit a pull request.  

---

## License  

This project is licensed under the [MIT License](LICENSE).  

---

Enjoy exploring and customizing your smart home with these creative control devices! 🌟
```