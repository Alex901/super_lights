# Super Lights Project

This project is designed to control a lighting system using an ESP32 microcontroller. It features IR and ultrasonic sensors for motion detection and distance measurement, allowing for automated light control based on environmental conditions. The lighting system uses an **8 RGB LED module** for dynamic lighting effects.

---

## Features

- **IR Sensor**: Detects motion and turns on the light based on sensitivity settings.
- **Ultrasonic Sensor (HC-SR04)**: Measures distance and turns off the light if the distance exceeds a configurable threshold.
- **Adjustable Settings**:
  - IR sensitivity (1-100% mapped to 1-20 pulses).
  - Ultrasonic sensitivity (2-100 cm).
  - Timing settings for IR and ultrasonic sensors.
- **Menu System**: Navigate and adjust settings via a simple menu interface.
- **8 RGB LED Module**: Dynamic lighting effects with adjustable brightness and color.

---

## Hardware Requirements

- **ESP32 Development Board**
- **HC-SR04 Ultrasonic Sensor**
  - **TRIG Pin**: GPIO 26
  - **ECHO Pin**: GPIO 5
- **IR Sensor**
  - **Signal Pin**: GPIO 27
- **8 RGB LED Module**
  - **Control Pin**: GPIO 13
- **Buttons**:
  - **UP Button**: GPIO 32
  - **DOWN Button**: GPIO 33
  - **ENTER Button**: GPIO 25
  - **BACK Button**: GPIO 26
  **LED Display** 
  - **SDA Pin**: GPIO 22
  - **SCL Pin**: GPIO 23
  **Audio Amplifier**
  + **Seaker**: L+, L-
  - **BCK**: GPIO 25
  - **DIN**: GPIO 33
  - **LCK**: GPIO 32 
- **Power Supply**: 5V provided to all modules.

---

## GPIO Pin Configuration

| Component               | GPIO Pin |
|-------------------------|----------|
| Ultrasonic TRIG         | GPIO 26  |
| Ultrasonic ECHO         | GPIO 5   |
| IR Sensor Signal        | GPIO 27  |
| RGB LED Module          | GPIO 13  |
| UP Button               | GPIO 32  |
| DOWN Button             | GPIO 33  |
| ENTER Button            | GPIO 25  |
| BACK Button             | GPIO 26  |
| LED Display SDA         | GPIO 22  |
| LED Display SCL         | GPIO 23  |
| Audio Amplifier BCK     | GPIO 25  |
| Audio Amplifier DIN     | GPIO 33  |
| Audio Amplifier LCK     | GPIO 32  |
|-------------------------|----------|

---

## Software Requirements

- **ESP-IDF Version**: 5.4.1
- **CMake**: Used for project configuration and build.
- **Python**: Required for ESP-IDF tools.

---

## How to Get Started

1. **Set Up ESP-IDF**:
   - Install the ESP-IDF development environment by following the official [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).
   - Ensure you have ESP-IDF version 5.4.1 installed.

2. **Clone the Repository**:
   - Clone this project to your local machine:
     ```bash
     git clone https://github.com/your-repo/super_lights.git
     cd super_lights
     ```

3. **Configure the Project**:
   - Run the following command to configure the project:
     ```bash
     idf.py menuconfig
     ```
   - Adjust any settings as needed, such as GPIO pins or features.

4. **Build the Project**:
   - Compile the project using:
     ```bash
     idf.py build
     ```

5. **Flash the Firmware**:
   - Connect your ESP32 board to your computer via USB.
   - Flash the firmware to the board:
     ```bash
     idf.py -p [PORT] flash
     ```
     Replace `[PORT]` with the serial port of your ESP32 (e.g., `/dev/ttyUSB0` or `COM3`).

6. **Monitor the Output**:
   - View the serial output from the ESP32:
     ```bash
     idf.py -p [PORT] monitor
     ```

7. **Connect the Hardware**:
   - Assemble the hardware components as described in the "Hardware Requirements" section.
   - Ensure all connections are secure and powered correctly.

8. **Test the System**:
   - Power on the ESP32 and verify that the system operates as expected.
   - Use the menu system to navigate and adjust settings.

9. **Enjoy**:
   - Explore the features of the Super Lights project and customize it to your needs!