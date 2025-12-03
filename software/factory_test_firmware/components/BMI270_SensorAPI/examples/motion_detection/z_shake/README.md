# BMI270 Z-Axis Complete Flip Detection Example

## Overview

This example demonstrates how to use the BMI270 sensor to detect Z-axis complete flip sequences. By monitoring changes in Z-axis acceleration, it can recognize specific flip patterns (positive → negative → positive → negative) and trigger corresponding actions when a complete sequence is detected. This example can be used as an ESP-IDF component and supports multiple hardware platforms.

## Features

- **Z-axis Flip Detection**: Detects complete Z-axis flip sequences
- **State Machine Processing**: Uses state machine to precisely track flip process
- **Real-time Monitoring**: Displays Z-axis acceleration values and detection status in real-time
- **Action Triggering**: Executes predefined actions when complete sequence is detected
- **Multi-platform Support**: Supports multiple ESP32 development boards

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
In `Component config` -> `BMI270 Basic IMU`:

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

### 2. Detection Parameter Configuration

You can adjust the following parameters in the code:

```c
// Modify detection thresholds in detect_z_axis_complete_flip function
if (acc_z_current > 0.5f) {  // Positive direction threshold
if (acc_z_current < -0.5f) { // Negative direction threshold

// Modify maximum sample count
const uint8_t max_samples = 150; // Maximum sample count

// Modify sampling delay
vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
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

## Flip Detection Principle

### Detection Sequence
The program detects the following Z-axis flip sequence:
1. **Positive Direction**: Z-axis acceleration > 0.5g
2. **Negative Direction**: Z-axis acceleration < -0.5g
3. **Positive Direction**: Z-axis acceleration > 0.5g
4. **Negative Direction**: Z-axis acceleration < -0.5g

### State Machine
The program uses a state machine to track the flip process:
- `WAITING_FOR_POSITIVE`: Waiting for positive direction
- `WAITING_FOR_NEGATIVE`: Waiting for negative direction
- `WAITING_FOR_POSITIVE_AGAIN`: Waiting for positive direction to appear again
- `WAITING_FOR_NEGATIVE_AGAIN`: Waiting for negative direction to appear again

### Trigger Actions
When a complete sequence is detected, the following actions are executed:
1. Record flip timestamp
2. Send complete flip notification
3. Update device status
4. Execute user-defined callback function

## Usage

### 1. Compile and Flash

```bash
cd examples/motion_detection/bmi270_basic_imu
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Flip Detection

1. Perform Z-axis flip operations according to prompts
2. Observe serial output to confirm detection status
3. When a complete sequence is detected, corresponding actions will be triggered

## Output Example

```
I (xxx) BMI270: BMI270 Z-axis Complete Flip Detection Example
I (xxx) BMI270: Current wrist coordinate axis register configuration:
I (xxx) BMI270:   X-axis: 0+
I (xxx) BMI270:   Y-axis: 1+
I (xxx) BMI270:   Z-axis: 2+
I (xxx) BMI270: === Testing Z-axis complete flip sequence detection ===
I (xxx) BMI270: Please flip the device from Z-axis positive direction to negative, then back to positive, finally to negative
I (xxx) BMI270: [ 1] 0.25 m/s² [Waiting for positive direction]
I (xxx) BMI270: [ 2] 0.85 m/s² [Positive direction detected]
I (xxx) BMI270: [ 3] -0.12 m/s² [Waiting for negative direction]
I (xxx) BMI270: [ 4] -0.75 m/s² [Negative direction detected]
I (xxx) BMI270: [ 5] 0.45 m/s² [Waiting for positive direction again]
I (xxx) BMI270: [ 6] 0.95 m/s² [Positive direction detected again]
I (xxx) BMI270: [ 7] -0.35 m/s² [Waiting for negative direction again]
I (xxx) BMI270: [ 8] -0.85 m/s² [Complete flip sequence detected!]
I (xxx) BMI270: Z-axis flip sequence: positive->negative->positive->negative
I (xxx) BMI270: Trigger action: Complete Z-axis flip event
I (xxx) BMI270: === Executing complete flip trigger action ===
I (xxx) BMI270: 1. Record flip timestamp
I (xxx) BMI270: 2. Send complete flip notification
I (xxx) BMI270: 3. Update device status
I (xxx) BMI270: 4. Execute user-defined callback function
I (xxx) BMI270: === Complete flip action execution finished ===
```

## Flip Testing Methods

### Testing Steps
1. Place the device flat with the front facing up
2. Perform Z-axis flip according to prompts: positive → negative → positive → negative
3. Ensure flip actions are clear and avoid other interfering movements
4. Observe serial output to confirm detection results

### Notes
- Flip actions need to be complete and clear
- Avoid other violent movements during detection
- Ensure BMI270 sensor is correctly connected
- Adjust detection thresholds according to actual needs

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- Adjust detection thresholds and delay times according to actual application scenarios
- Flip detection requires complete sequences, ensure clear motion trajectories
- Avoid other violent movements during detection

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Inaccurate Detection**: Adjust detection thresholds, ensure clear flip actions
3. **State Machine Stuck**: Check if flip sequence is complete
4. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected

## Application Scenarios

- **Smart Watches**: Wrist flip gesture control
- **VR/AR Devices**: Head flip control
- **Game Controllers**: Flip input
- **Smart Home**: Flip control devices
- **Industrial Control**: Flip operation interface 