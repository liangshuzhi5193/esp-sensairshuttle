# BMI270 Any-Motion Interrupt Detection Example

## Overview

This example demonstrates how to use the BMI270 sensor's any-motion functionality to detect motion and trigger interrupts. When any motion is detected, the BMI270 sends a signal to the ESP32 through a GPIO interrupt pin, and the ESP32 executes corresponding processing logic upon receiving the interrupt. This example can be used as an ESP-IDF component and supports multiple hardware platforms.

## Features

- **Any-Motion Detection**: Detects motion in any direction
- **GPIO Interrupt**: Uses ESP32 GPIO interrupt to respond to motion detection
- **Configurable Threshold**: Adjustable motion detection sensitivity
- **Delay Reset**: Waits 3 seconds after detection before re-enabling detection
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
In `Component config` -> `BMI270 Any-Motion Interrupt`:

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
// Modify detection parameters in set_feature_config function
config.cfg.any_motion.duration = 0x02;      // Duration: 40ms
config.cfg.any_motion.threshold = 0xFF;     // Threshold: 255mg

// Modify interrupt pin configuration
pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;  // Active low
pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;    // Push-pull output
pin_config.int_latch = BMI2_INT_NON_LATCH;        // Non-latch mode
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

### Any-Motion Detection Mechanism

1. **Acceleration Monitoring**: BMI270 continuously monitors three-axis acceleration
2. **Threshold Comparison**: Triggers when acceleration change exceeds set threshold
3. **Duration**: Motion needs to persist for a certain time to be confirmed
4. **Interrupt Generation**: Sends interrupt signal through INT1 pin after confirming motion

### Interrupt Processing Flow

1. **GPIO Configuration**: Configure ESP32 GPIO as input mode, enable interrupt
2. **Interrupt Service**: Install GPIO interrupt service
3. **Interrupt Handling**: Check any-motion status when interrupt is received
4. **Delay Reset**: Wait 3 seconds after motion detection before re-enabling

### Detection Parameter Description

- **Threshold**: 0xFF (255mg) - Motion detection sensitivity
- **Duration**: 0x02 (40ms) - Time motion needs to persist
- **Interrupt Level**: Active low
- **Output Mode**: Push-pull output
- **Latch Mode**: Non-latch (edge-triggered)

## Usage

### 1. Compile and Flash

```bash
cd examples/motion_detection/anymotion_int
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Motion Detection

1. After running the program, the system will display current configuration information
2. Move or shake the device to trigger any-motion detection
3. Observe serial output to confirm interrupt triggering
4. Wait 3 seconds before performing the next detection

## Output Example

```
I (xxx) MAIN: === BMI270 Interrupt IMU Configuration ===
I (xxx) MAIN: Selected Board: ESP SPOT S3
I (xxx) MAIN: GPIO Configuration:
I (xxx) MAIN:   - Interrupt GPIO: 5
I (xxx) MAIN:   - I2C SCL GPIO: 1
I (xxx) MAIN:   - I2C SDA GPIO: 2
I (xxx) MAIN:   - I2C Type: Software I2C
I (xxx) MAIN: ================================
I (xxx) MAIN: Please move the board to trigger interrupt...
GPIO[5] intr, val: 0
I (xxx) MAIN: Any-motion interrupt is generated!
I (xxx) MAIN: Waiting 3 seconds before next detection...
I (xxx) MAIN: Ready for next motion detection...
```

## Motion Testing Methods

### Testing Steps
1. Place the device on a stable surface
2. Gently move or shake the device
3. Observe serial output to confirm any-motion interrupt triggering
4. Wait 3 seconds before testing again

### Notes
- Motion amplitude needs to exceed the set threshold (255mg)
- Motion needs to persist for a certain time (40ms)
- Avoid overly violent movements to prevent device damage
- Ensure BMI270 sensor is correctly connected

## Parameter Adjustment

### Increase Sensitivity
```c
config.cfg.any_motion.threshold = 0x80;  // Lower threshold to 128mg
config.cfg.any_motion.duration = 0x01;   // Reduce duration to 20ms
```

### Decrease Sensitivity
```c
config.cfg.any_motion.threshold = 0xFF;  // Increase threshold to 255mg
config.cfg.any_motion.duration = 0x04;   // Increase duration to 80ms
```

### Interrupt Configuration Adjustment
```c
pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;  // Active high
pin_config.int_latch = BMI2_INT_LATCH;              // Latch mode
```

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- Adjust detection thresholds and duration according to actual application scenarios
- Avoid overly violent movements during detection

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin connections and configuration
3. **Inaccurate Detection**: Adjust threshold and duration parameters
4. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected

## Application Scenarios

- **Motion Detection**: Detect whether the device is in motion
- **Wake-up Function**: Wake up device from low-power mode
- **Security Monitoring**: Detect if device has been moved
- **User Interaction**: Trigger user interface through motion
- **Industrial Control**: Motion-triggered control signals 