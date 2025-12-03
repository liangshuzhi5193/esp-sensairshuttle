# BMI270 Shake Detection Example

## Overview

This example demonstrates how to use the BMI270 sensor's shake functionality to detect shake gestures and trigger interrupts. The shake feature can recognize both slight shake and heavy shake on different axes (X, Y, Z). When a shake is detected, the BMI270 sends a signal to the ESP32 through a GPIO interrupt pin, and the ESP32 executes corresponding processing logic upon receiving the interrupt.

## Features

- **Shake Detection**: Detects shake gestures in any direction
- **Shake Classification**: 
  - Slight shake: Gentle shake detection
  - Heavy shake: Strong shake detection
- **Axis Detection**: Identifies shake direction (X, Y, Z axis)
- **GPIO Interrupt**: Uses ESP32 GPIO interrupt to respond to shake detection
- **Configurable Sensitivity**: Adjustable shake detection sensitivity
- **Multi-platform Support**: Supports multiple ESP32 development boards
- **Real-time Response**: Low-latency interrupt response mechanism

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
In `Component config` -> `BMI270 Shake Detection`:

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

### 2. Shake Detection Parameter Configuration

You can adjust the following parameters in the code:

```c
// Modify shake sensitivity in set_feature_interrupt function
// The shake configuration is handled by the BMI270 firmware
// You can adjust sensitivity by modifying shake_config array if needed
shake_config[1] = 0x02;  // Lower threshold for more sensitivity
shake_config[9] = 0x10;  // Lower threshold for more sensitivity
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

## Working Principle

### Shake Detection Mechanism

1. **Acceleration Monitoring**: BMI270 continuously monitors three-axis acceleration
2. **Shake Pattern Recognition**: Analyzes acceleration patterns to identify shake characteristics
3. **Threshold Comparison**: Triggers when shake amplitude exceeds set thresholds
4. **Axis Detection**: Determines which axis the shake occurred on
5. **Interrupt Generation**: Sends interrupt signal through INT1 pin after confirming shake

### Shake Recognition Flow

1. **GPIO Configuration**: Configure ESP32 GPIO as input mode, enable interrupt
2. **Interrupt Service**: Install GPIO interrupt service
3. **Interrupt Handling**: Check shake status when interrupt is received
4. **Shake Classification**: Read shake data from register 0x1f and classify the shake

### Detection Parameter Description

- **Shake Sensitivity**: Configurable through shake configuration registers
- **Axis Detection**: X, Y, Z axis shake detection
- **Shake Types**: Slight shake (0x08) and heavy shake (0x80)
- **Interrupt Level**: Active high
- **Output Mode**: Push-pull output
- **Latch Mode**: Latch mode

## Usage

### 1. Compile and Flash

```bash
cd examples/motion_detection/shake
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Shake Detection

1. After running the program, the system will display current configuration information
2. Perform shake gestures to trigger shake detection:
   - **Slight Shake**: Gentle shake in any direction
   - **Heavy Shake**: Strong shake in any direction
3. Observe serial output to confirm shake recognition and axis detection

## Output Example

```
I (xxx) MAIN: === BMI270 TOY Shake Detection Configuration ===
I (xxx) MAIN: Selected Board: ESP SPOT S3
I (xxx) MAIN: GPIO Configuration:
I (xxx) MAIN:   - Interrupt GPIO: 5
I (xxx) MAIN:   - I2C SCL GPIO: 1
I (xxx) MAIN:   - I2C SDA GPIO: 2
I (xxx) MAIN:   - I2C Type: Software I2C
I (xxx) MAIN: ================================
I (xxx) MAIN: Shake feature enabled, result: 0
I (xxx) MAIN: Move the sensor to get shake interrupt...
GPIO[5] intr, val: 1
I (xxx) MAIN: Slight shake generated on: x axis
GPIO[5] intr, val: 1
I (xxx) MAIN: Heavy shake generated on: y axis
```

## Shake Testing Methods

### Testing Steps
1. **Slight Shake Test**: Gently shake the device in any direction
2. **Heavy Shake Test**: Shake the device more vigorously
3. **Axis-specific Test**: Shake the device along specific axes (X, Y, Z)
4. **Multi-axis Test**: Shake the device in multiple directions

### Notes
- Ensure natural shake movements
- Avoid overly violent movements to prevent device damage
- Test different shake intensities to understand sensitivity
- Wait for shake recognition before performing the next shake

## Parameter Adjustment

### Increase Sensitivity
```c
// Modify shake configuration for higher sensitivity
shake_config[1] = 0x01;  // Lower threshold
shake_config[9] = 0x08;  // Lower threshold
```

### Decrease Sensitivity
```c
// Modify shake configuration for lower sensitivity
shake_config[1] = 0x04;  // Higher threshold
shake_config[9] = 0x20;  // Higher threshold
```

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- Adjust shake sensitivity according to actual application scenarios
- Avoid overly violent movements during detection
- The shake detection is optimized for handheld device applications

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin connections and configuration
3. **Inaccurate Shake Recognition**: Adjust shake sensitivity parameters
4. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected
5. **No Axis Detection**: Check shake data register reading

## Application Scenarios

- **User Interface Control**: Shake to change settings or navigate menus
- **Game Control**: Shake-based game interactions
- **Gesture Recognition**: Shake as a gesture input method
- **Activity Detection**: Detect user activity through shake patterns
- **Alarm Functions**: Shake to snooze or dismiss alarms
- **Device Control**: Shake to trigger device functions
- **Fitness Applications**: Shake-based fitness tracking
- **Entertainment**: Shake-based entertainment applications 