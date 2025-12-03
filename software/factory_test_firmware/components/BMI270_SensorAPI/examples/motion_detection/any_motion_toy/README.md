# BMI270 Any-Motion Detection Example (Toy Firmware)

## Overview

This example demonstrates how to use the BMI270 sensor's any-motion functionality with the toy firmware to detect motion and trigger interrupts. When any motion is detected, the BMI270 sends a signal to the ESP32 through a GPIO interrupt pin, and the ESP32 executes corresponding processing logic upon receiving the interrupt. This example uses the toy firmware which provides enhanced motion detection capabilities.

## Features

- **Any-Motion Detection**: Detects motion in any direction using toy firmware
- **GPIO Interrupt**: Uses ESP32 GPIO interrupt to respond to motion detection
- **Toy Firmware**: Enhanced motion detection with optimized algorithms
- **Configurable Threshold**: Adjustable motion detection sensitivity
- **Multi-platform Support**: Supports multiple ESP32 development boards
- **Real-time Response**: Low-latency interrupt response mechanism
- **Simplified Configuration**: Streamlined setup process

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
In `Component config` -> `BMI270 Any-Motion Detection (Toy Firmware)`:

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

The toy firmware automatically configures optimal parameters for any-motion detection. You can adjust the following parameters in the code if needed:

```c
// Modify interrupt configuration in set_feature_interrupt function
uint8_t data = BMI270_TOY_INT_ANY_MOT_MASK;  // Any-motion interrupt mask

// Modify interrupt pin configuration
pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;  // Active high
pin_config.int_latch = BMI2_INT_LATCH;              // Latch mode
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

### Any-Motion Detection Mechanism (Toy Firmware)

1. **Acceleration Monitoring**: BMI270 continuously monitors three-axis acceleration
2. **Advanced Pattern Recognition**: Toy firmware uses enhanced algorithms for motion detection
3. **Threshold Comparison**: Triggers when acceleration change exceeds optimized thresholds
4. **Interrupt Generation**: Sends interrupt signal through INT1 pin after confirming motion

### Interrupt Processing Flow

1. **GPIO Configuration**: Configure ESP32 GPIO as input mode, enable interrupt
2. **Interrupt Service**: Install GPIO interrupt service
3. **Interrupt Handling**: Check any-motion status when interrupt is received
4. **Status Reporting**: Report motion detection status

### Detection Parameter Description

- **Interrupt Mask**: BMI270_TOY_INT_ANY_MOT_MASK (0x40)
- **Interrupt Level**: Active high
- **Output Mode**: Push-pull output
- **Latch Mode**: Latch mode
- **Firmware**: Toy firmware with optimized detection algorithms

## Usage

### 1. Compile and Flash

```bash
cd examples/motion_detection/any_motion_toy
idf.py set-target esp32s3  # Select target according to your development board
idf.py menuconfig          # Configure development board and pins
idf.py build
idf.py flash monitor
```

### 2. Test Motion Detection

1. After running the program, the system will display current configuration information
2. Move or shake the device to trigger any-motion detection
3. Observe serial output to confirm interrupt triggering
4. The system will continuously monitor for motion

## Output Example

```
I (xxx) MAIN: === BMI270 TOY Any-Motion Detection Configuration ===
I (xxx) MAIN: Selected Board: ESP SPOT S3
I (xxx) MAIN: GPIO Configuration:
I (xxx) MAIN:   - Interrupt GPIO: 5
I (xxx) MAIN:   - I2C SCL GPIO: 1
I (xxx) MAIN:   - I2C SDA GPIO: 2
I (xxx) MAIN:   - I2C Type: Software I2C
I (xxx) MAIN: ================================
I (xxx) MAIN: Any-motion feature enabled, result: 0
I (xxx) MAIN: Move the sensor to get any-motion interrupt...
GPIO[5] intr, val: 1
I (xxx) MAIN: Any-motion interrupt generated! Status: 0x40
GPIO[5] intr, val: 1
I (xxx) MAIN: Any-motion interrupt generated! Status: 0x40
```

## Motion Testing Methods

### Testing Steps
1. Place the device on a stable surface
2. Gently move or shake the device
3. Observe serial output to confirm any-motion interrupt triggering
4. Continue testing with different motion patterns

### Notes
- Motion amplitude needs to exceed the toy firmware's optimized threshold
- The toy firmware provides enhanced sensitivity and accuracy
- Avoid overly violent movements to prevent device damage
- Ensure BMI270 sensor is correctly connected

## Parameter Adjustment

### Interrupt Configuration
```c
// Modify interrupt level
pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;  // Active low

// Modify latch mode
pin_config.int_latch = BMI2_INT_NON_LATCH;        // Non-latch mode
```

### Firmware Selection
The toy firmware provides optimized detection algorithms. If you need to switch to standard firmware, modify the component configuration.

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- The toy firmware provides enhanced motion detection capabilities
- Avoid overly violent movements during detection
- The toy firmware is optimized for handheld device applications

## Troubleshooting

1. **I2C Communication Failure**: Check I2C pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin connections and configuration
3. **Inaccurate Detection**: The toy firmware should provide better accuracy than standard firmware
4. **Compilation Errors**: Ensure correct hardware platform and pin configuration are selected
5. **Firmware Issues**: Verify that the toy firmware is correctly loaded

## Toy Firmware Advantages

- **Enhanced Sensitivity**: Better motion detection sensitivity
- **Optimized Algorithms**: Improved detection algorithms
- **Reduced False Positives**: Better filtering of unwanted triggers
- **Faster Response**: Reduced latency in motion detection
- **Better Accuracy**: More accurate motion pattern recognition

## Application Scenarios

- **Motion Detection**: Detect whether the device is in motion
- **Wake-up Function**: Wake up device from low-power mode
- **Security Monitoring**: Detect if device has been moved
- **User Interaction**: Trigger user interface through motion
- **Industrial Control**: Motion-triggered control signals
- **Activity Monitoring**: Monitor user activity patterns
- **Gesture Recognition**: Basic gesture detection
- **Device Control**: Motion-based device control 