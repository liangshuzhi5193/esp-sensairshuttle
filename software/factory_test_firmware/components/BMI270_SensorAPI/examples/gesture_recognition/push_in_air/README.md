# BMI270 Push in Air Detection Example

## Overview

This example demonstrates how to use the BMI270 sensor's push functionality to detect push gestures in the air and trigger interrupts. The push feature can recognize push gestures in different directions (X, Y, Z axis) and provides direction information. When a push is detected, the BMI270 sends a signal to the ESP32 through a GPIO interrupt pin, and the ESP32 executes corresponding processing logic upon receiving the interrupt.

## Features

- **Push Detection**: Detects push gestures in the air
- **Direction Detection**: Identifies push direction (X, Y, Z axis) with improved Z-axis accuracy
- **Rolling Integration**: Uses rolling data to adjust high-g axis selection
- **GPIO Interrupt**: Uses ESP32 GPIO interrupt to respond to push detection
- **Configurable Sensitivity**: Adjustable push detection sensitivity with optimized thresholds
- **Multi-platform Support**: Supports multiple ESP32 development boards
- **Real-time Response**: Low-latency interrupt response mechanism
- **Z-axis Direction Fix**: Enhanced Z-axis direction detection using raw accelerometer data

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
In `Component config` -> `BMI270 Push in Air Detection`:

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

### 2. Push Detection Parameter Configuration

You can adjust the following parameters in the code:

```c
// Default high-g threshold for push detection
uint16_t high_g_threshold = 0x0800;  // Default threshold

// Default accelerometer and gyroscope configuration
config[BMI2_ACCEL].cfg.acc.range = BMI2_ACC_RANGE_8G;   // Default 8G range
config[BMI2_GYRO].cfg.gyr.range = BMI2_GYR_RANGE_1000;  // Default 1000dps range

// Interrupt configuration
uint8_t data = BMI270_TOY_INT_PUSH_MASK;
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

### Push Detection Mechanism

1. **Acceleration and Gyroscope Monitoring**: BMI270 continuously monitors three-axis acceleration and gyroscope
2. **Push Pattern Recognition**: Analyzes acceleration patterns to identify push characteristics
3. **Direction Detection**: Determines which axis the push occurred on
4. **Rolling Integration**: Uses rolling data to dynamically adjust high-g axis selection
5. **Interrupt Generation**: Sends interrupt signal through INT1 pin after confirming push

### Push Recognition Flow

1. **GPIO Configuration**: Configure ESP32 GPIO as input mode, enable interrupt
2. **Interrupt Service**: Install GPIO interrupt service
3. **Interrupt Handling**: Check push status when interrupt is received
4. **Direction Analysis**: Read push direction using high-g direction function
5. **Axis Adjustment**: Adjust high-g axis selection based on rolling data

### Detection Parameter Description

- **High-G Threshold**: 0x0800 - Push detection sensitivity
- **Push Interrupt Mask**: BMI270_TOY_INT_PUSH_MASK (0x20)
- **Rolling Interrupt Mask**: BMI270_TOY_INT_GI_INS1_ROLLING_MASK (0x10)
- **Interrupt Level**: Active high
- **Output Mode**: Push-pull output
- **Latch Mode**: Latch mode

## Usage

### 1. Compile and Flash

```bash
cd examples/gesture_recognition/push_in_air
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Push Detection

1. After running the program, the system will display current configuration information
2. Perform push gestures in the air to trigger push detection:
   - **Forward Push**: Push the device forward
   - **Backward Push**: Push the device backward
   - **Side Push**: Push the device to the side
   - **Up/Down Push**: Push the device up or down
3. Observe serial output to confirm push recognition and direction detection

## Output Example

```
I (xxx) MAIN: === BMI270 TOY Push Detection Configuration ===
I (xxx) MAIN: Selected Board: ESP SPOT S3
I (xxx) MAIN: GPIO Configuration:
I (xxx) MAIN:   - Interrupt GPIO: 5
I (xxx) MAIN:   - I2C SCL GPIO: 1
I (xxx) MAIN:   - I2C SDA GPIO: 2
I (xxx) MAIN:   - I2C Type: Software I2C
I (xxx) MAIN: ================================
I (xxx) MAIN: Push feature enabled, result: 0
I (xxx) MAIN: Rolling feature enabled, result: 0
I (xxx) MAIN: Move the sensor to get push interrupt...
GPIO[5] intr, val: 1
I (xxx) MAIN: Push generated, direction: +X
GPIO[5] intr, val: 1
I (xxx) MAIN: Rolling generated
```

## Push Testing Methods

### Testing Steps
1. **Forward Push Test**: Push the device forward in the air
2. **Backward Push Test**: Push the device backward in the air
3. **Side Push Test**: Push the device to the left or right
4. **Up/Down Push Test**: Push the device upward or downward
5. **Multi-direction Test**: Test pushes in different directions

### Notes
- Ensure natural push movements in the air
- Avoid overly violent movements to prevent device damage
- Test different push intensities to understand sensitivity
- Wait for push recognition before performing the next push

## Parameter Adjustment

### Increase Sensitivity
```c
// Modify high-g threshold for higher sensitivity
uint16_t high_g_threshold = 0x0400;  // Lower threshold
```

### Decrease Sensitivity
```c
// Modify high-g threshold for lower sensitivity
uint16_t high_g_threshold = 0x1000;  // Higher threshold
```

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- Adjust push sensitivity according to actual application scenarios
- Avoid overly violent movements during detection
- The push detection is optimized for air gesture applications

## Z-axis Direction Detection Fix

### Problem Description
The original `get_toy_high_g_direction` function had a defect in Z-axis direction detection. The register 0x38's bit3 (sign bit) was always set to 1 for Z-axis detection, causing all Z-axis pushes to be reported as "+Z" regardless of actual direction.

### Solution Implementation
The function has been enhanced to properly detect Z-axis direction:

1. **X and Y Axis**: Continue using the standard sign bit (bit3) for direction detection
2. **Z Axis**: Use raw accelerometer data from register 0x12 to determine direction:
   - Read 6 bytes of accelerometer data (X, Y, Z axes)
   - Extract Z-axis acceleration value
   - Determine direction based on Z-axis acceleration sign:
     - `accel_z > 0`: "+Z" (upward push)
     - `accel_z < 0`: "-Z" (downward push)

### Code Changes
```c
// Enhanced Z-axis direction detection in get_toy_high_g_direction()
else if (high_g_out[0] & 0x04) {
    direction[1] = 'z';
    // Z轴需要特殊处理：通过读取原始加速度数据来判断方向
    uint8_t accel_data[6];
    rslt = bmi2_get_regs(0x12, accel_data, 6, bmi2_dev);
    if (rslt == BMI2_OK) {
        int16_t accel_z = (int16_t)(accel_data[5] << 8) | accel_data[4];
        
        // 根据Z轴加速度的符号判断方向
        if (accel_z > 0) {
            direction[0] = '+';  // 向上推压
        } else {
            direction[0] = '-';  // 向下推压
        }
    }
}
```

### Benefits
- **Accurate Z-axis Direction**: Now correctly distinguishes between upward and downward pushes
- **Backward Compatibility**: X and Y axis detection remains unchanged
- **Robust Error Handling**: Falls back to original method if accelerometer read fails
- **Improved User Experience**: More accurate gesture recognition for Z-axis movements

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin connections and configuration
3. **Inaccurate Push Recognition**: Adjust high-g threshold parameters
4. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected
5. **No Direction Detection**: Check high-g direction function
6. **Z-axis Direction Issues**: Verify accelerometer data reading and direction logic

## Application Scenarios

- **Air Gesture Control**: Control devices through air push gestures
- **Game Control**: Push-based game interactions
- **User Interface**: Push as a gesture input method
- **Virtual Reality**: Air gesture recognition for VR applications
- **Smart Home**: Push gestures for smart home control
- **Presentation Control**: Push gestures for presentation navigation
- **Accessibility**: Push gestures for accessibility applications
- **Entertainment**: Push-based entertainment applications 