# BMI270 Toy Motion Gesture Recognition Example

## Overview

This example demonstrates how to use the BMI270 sensor's toy motion functionality to detect various toy gestures and trigger interrupts. The toy motion feature can recognize five different gestures: pick up, put down, throw up, throw down, and throw catch. When a gesture is detected, the BMI270 sends a signal to the ESP32 through a GPIO interrupt pin, and the ESP32 executes corresponding processing logic upon receiving the interrupt.

## Features

- **Toy Motion Detection**: Detects five different toy gestures
- **Gesture Recognition**: 
  - Pick up (0x01): Device is picked up
  - Put down (0x02): Device is put down
  - Throw up (0x03): Device is thrown upward
  - Throw down (0x04): Device is thrown downward
  - Throw catch (0x05): Device is thrown and caught
- **GPIO Interrupt**: Uses ESP32 GPIO interrupt to respond to gesture detection
- **Configurable Parameters**: Adjustable gesture detection sensitivity
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
In `Component config` -> `BMI270 Toy Motion Gesture Recognition`:

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

### 2. Gesture Detection Parameter Configuration

You can adjust the following parameters in the code:

```c
// Modify detection parameters in adjust_toy_motion_config function
uint16_t throw_min_duration = 0x06;    // Increase minimum duration to reduce false triggers
uint16_t throw_up_duration = 0x03;     // Increase throw up duration to improve throw up recognition
uint16_t low_g_exit_duration = 0x02;   // Increase low-g exit duration to improve throw down recognition
uint16_t quiet_time_duration = 0x04;   // Increase quiet time duration to improve throw catch recognition
uint16_t slope_thres = 0x1000;         // Increase slope threshold to reduce throw down being misidentified as collision
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

### Toy Motion Detection Mechanism

1. **Acceleration and Gyroscope Monitoring**: BMI270 continuously monitors three-axis acceleration and gyroscope
2. **Gesture Pattern Recognition**: Analyzes motion patterns to identify specific gestures
3. **Threshold Comparison**: Triggers when motion characteristics match gesture patterns
4. **Interrupt Generation**: Sends interrupt signal through INT1 pin after confirming gesture

### Gesture Recognition Flow

1. **GPIO Configuration**: Configure ESP32 GPIO as input mode, enable interrupt
2. **Interrupt Service**: Install GPIO interrupt service
3. **Interrupt Handling**: Check toy motion status when interrupt is received
4. **Gesture Classification**: Read gesture type from register 0x1e and classify the gesture

### Detection Parameter Description

- **Throw Min Duration**: 0x06 - Minimum duration for throw gestures
- **Throw Up Duration**: 0x03 - Duration for upward throw recognition
- **Low G Exit Duration**: 0x02 - Duration for low gravity exit (throw down)
- **Quiet Time Duration**: 0x04 - Duration for quiet period (throw catch)
- **Slope Threshold**: 0x1000 - Threshold for collision detection

## Usage

### 1. Compile and Flash

```bash
cd examples/gesture_recognition/toy_motion
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Gesture Detection

1. After running the program, the system will display current configuration information
2. Perform different gestures to trigger toy motion detection:
   - **Pick Up**: Pick up the device
   - **Put Down**: Put down the device
   - **Throw Up**: Throw the device upward
   - **Throw Down**: Throw the device downward
   - **Throw Catch**: Throw the device and catch it
3. Observe serial output to confirm gesture recognition

## Output Example

```
I (xxx) MAIN: === BMI270 TOY Motion Detection Configuration ===
I (xxx) MAIN: Selected Board: ESP SPOT S3
I (xxx) MAIN: GPIO Configuration:
I (xxx) MAIN:   - Interrupt GPIO: 5
I (xxx) MAIN:   - I2C SCL GPIO: 1
I (xxx) MAIN:   - I2C SDA GPIO: 2
I (xxx) MAIN:   - I2C Type: Software I2C
I (xxx) MAIN: ================================
I (xxx) MAIN: Toy-motion feature enabled, result: 0
I (xxx) MAIN: Move the sensor to get toy-motion interrupt...
GPIO[5] intr, val: 1
I (xxx) MAIN: Toy motion detected - Raw data: 0x0C, Gesture type: 0x03
I (xxx) MAIN: *** THROW UP generated ***
GPIO[5] intr, val: 1
I (xxx) MAIN: Toy motion detected - Raw data: 0x14, Gesture type: 0x05
I (xxx) MAIN: *** THROW CATCH generated ***
```

## Gesture Testing Methods

### Testing Steps
1. **Pick Up Test**: Gently pick up the device from a surface
2. **Put Down Test**: Gently put down the device on a surface
3. **Throw Up Test**: Quickly throw the device upward and let it fall
4. **Throw Down Test**: Quickly throw the device downward
5. **Throw Catch Test**: Throw the device upward and catch it smoothly

### Notes
- Ensure smooth and natural gesture movements
- Avoid overly violent movements to prevent device damage
- For throw gestures, ensure adequate space for safe testing
- Wait for gesture recognition before performing the next gesture

## Parameter Adjustment

### Increase Sensitivity
```c
uint16_t throw_min_duration = 0x04;    // Decrease minimum duration
uint16_t throw_up_duration = 0x02;     // Decrease throw up duration
uint16_t low_g_exit_duration = 0x01;   // Decrease low-g exit duration
uint16_t quiet_time_duration = 0x02;   // Decrease quiet time duration
```

### Decrease Sensitivity
```c
uint16_t throw_min_duration = 0x08;    // Increase minimum duration
uint16_t throw_up_duration = 0x04;     // Increase throw up duration
uint16_t low_g_exit_duration = 0x03;   // Increase low-g exit duration
uint16_t quiet_time_duration = 0x06;   // Increase quiet time duration
```

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- Adjust detection parameters according to actual application scenarios
- Avoid overly violent movements during detection
- For throw gestures, ensure adequate space and safety measures

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin connections and configuration
3. **Inaccurate Gesture Recognition**: Adjust gesture detection parameters
4. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected
5. **Mixed Gesture Detection**: Fine-tune duration and threshold parameters

## Application Scenarios

- **Toy Interaction**: Detect toy movements and gestures
- **Game Control**: Use gestures to control games
- **User Interface**: Gesture-based user interface control
- **Motion Analysis**: Analyze and classify motion patterns
- **Educational Applications**: Motion-based learning applications
- **Entertainment**: Gesture-based entertainment applications 