# BMI270 Circle Gesture Recognition Example

## Overview

This example demonstrates how to use the BMI270 sensor for circle gesture detection. By configuring different rotation axes (X, Y, Z axes) and sensitivity parameters, it can recognize clockwise and counterclockwise circular motions. This example can be used as an ESP-IDF component and supports multiple hardware platforms.

## Features

- **Circle Gesture Detection**: Detects circular motion on specific axes
- **Direction Recognition**: Distinguishes between clockwise and counterclockwise directions
- **Multi-axis Support**: Supports rotation detection on X, Y, Z three axes
- **Sensitivity Adjustment**: Configurable detection sensitivity for different axes
- **GPIO Interrupt**: Uses ESP32 GPIO interrupt to respond to gesture detection
- **Delay Reset**: Waits 3 seconds after detection before re-enabling detection

## Hardware Requirements

- ESP32 development board (supports the following models):
  - ESP-SPOT-C5
  - ESP-SPOT-S3  
  - ESP-ASTOM-S3
  - ESP-ECHOEAR-S3
  - Custom development board
- BMI270 sensor module
- Connection wires

## Configuration

### 1. Menuconfig Configuration

Run `idf.py menuconfig` to enter the configuration interface:

#### Board Selection
In `Component config` -> `BMI270 Circle Gesture`:

- **ESP-SPOT-C5**: Select this option to use ESP-SPOT-C5 development board
- **ESP-SPOT-S3**: Select this option to use ESP-SPOT-S3 development board
- **ESP-ASTOM-S3**: Select this option to use ESP-ASTOM-S3 development board
- **ESP-ECHOEAR-S3**: Select this option to use ESP-ECHOEAR-S3 development board
- **Custom Board**: Select this option to use custom development board

#### Pin Configuration (Custom Board)
If you select Custom Board, you need to configure the following pins:
- **I2C SCL Pin**: I2C clock pin
- **I2C SDA Pin**: I2C data pin
- **Interrupt Pin**: Interrupt pin

### 2. Code Configuration

#### XYZ Axis Selection
Modify the rotation axis configuration in `main.c`:

```c
// Modify axis_of_rotation in the set_feature_config function
config[0].cfg.circle_gest_det.cgd_cfg1.axis_of_rotation = 0x01; // X-axis
// or
config[0].cfg.circle_gest_det.cgd_cfg1.axis_of_rotation = 0x02; // Y-axis  
// or
config[0].cfg.circle_gest_det.cgd_cfg1.axis_of_rotation = 0x03; // Z-axis (default)
```

#### Sensitivity Adjustment
Modify sensitivity parameters in `main.c`:

```c
// Modify sensitivity configuration in the set_feature_config function
config[0].cfg.circle_gest_det.cgd_cfg1.sensitivity = 0x01; // Low sensitivity
// or
config[0].cfg.circle_gest_det.cgd_cfg1.sensitivity = 0x02; // Medium sensitivity
// or
config[0].cfg.circle_gest_det.cgd_cfg1.sensitivity = 0x03; // High sensitivity
```

## Pin Connections

### Predefined Development Board Pins

#### ESP-SPOT-C5
- Interrupt pin: GPIO 3
- I2C SCL: GPIO 26
- I2C SDA: GPIO 25

#### ESP-SPOT-S3
- Interrupt pin: GPIO 5
- I2C SCL: GPIO 1
- I2C SDA: GPIO 2

#### ESP-ASTOM-S3
- Interrupt pin: GPIO 16
- I2C SCL: GPIO 0
- I2C SDA: GPIO 45

#### ESP-ECHOEAR-S3
- Interrupt pin: GPIO 21
- I2C SCL: GPIO 1
- I2C SDA: GPIO 2

### Custom Development Board
Configure custom pins through menuconfig, or directly modify pin definitions in the code.

## Axis Configuration Details

### Axis Selection Description

- **X-axis (0x01)**: Device rotates around X-axis, i.e., pitch direction circular motion
- **Y-axis (0x02)**: Device rotates around Y-axis, i.e., yaw direction circular motion  
- **Z-axis (0x03)**: Device rotates around Z-axis, i.e., roll direction circular motion

### Gesture Direction Definition

#### Z-axis Rotation (Default Configuration)
- **Clockwise**: Clockwise rotation when viewed from the front of the device
- **Counterclockwise**: Counterclockwise rotation when viewed from the front of the device

#### X-axis Rotation
- **Clockwise**: Clockwise rotation when device tilts forward
- **Counterclockwise**: Counterclockwise rotation when device tilts forward

#### Y-axis Rotation
- **Clockwise**: Clockwise rotation when device swings left and right
- **Counterclockwise**: Counterclockwise rotation when device swings left and right

### Sensitivity Parameters

- **0x01 (Low sensitivity)**: Requires larger circular motion to trigger
- **0x02 (Medium sensitivity)**: Standard sensitivity, suitable for most applications
- **0x03 (High sensitivity)**: Smaller circular motion can trigger

## Usage

### 1. Compile and Flash

```bash
cd examples/gesture_recognition/bmi270_circle_gesture
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Gestures

1. Perform corresponding circular motion according to the selected axis
2. Observe serial output to confirm gesture detection results
3. Wait 3 seconds before performing the next detection

## Output Example

```
I (xxx) MAIN: Move the board in circular motion
I (xxx) MAIN: Circle Gesture Detected!
I (xxx) MAIN: Clockwise direction
I (xxx) MAIN: Waiting 3 seconds before next detection...
I (xxx) MAIN: Ready for next circle gesture detection...
```

## Gesture Testing Methods

### Z-axis Test (Default)
- Place the device flat with the front facing up
- Perform clockwise or counterclockwise rotation around the Z-axis
- Suitable for circular gestures on desktop or handheld devices

### X-axis Test
- Place the device vertically
- Perform forward and backward swinging around the X-axis
- Suitable for vertically mounted sensors

### Y-axis Test
- Place the device vertically
- Perform left and right swinging around the Y-axis
- Suitable for horizontally mounted sensors

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- Choose appropriate rotation axis and sensitivity according to actual application scenarios
- Gesture detection requires complete circular motion, ensure clear motion trajectory
- Avoid other violent movements during detection

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin connections and configuration
3. **Inaccurate Gesture Detection**: Adjust motion trajectory, ensure complete circular motion
4. **Wrong Direction Recognition**: Confirm the selected axis matches the actual motion direction
5. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected

## Application Scenarios

- **Smart Watches**: Wrist rotation gesture control
- **VR/AR Devices**: Head motion control
- **Game Controllers**: Gesture input
- **Smart Home**: Gesture control devices
- **Industrial Control**: Gesture operation interface 