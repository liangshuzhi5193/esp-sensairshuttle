# BMI270 Sensor API Examples

## Overview

This repository provides a comprehensive collection of examples based on the BMI270 sensor, covering different application scenarios including gesture recognition, motion detection, and power management. All examples support multiple ESP32 development boards and provide flexible firmware loading mechanisms.

## Example Categories

### 1. Gesture Recognition

#### 1.1 Circle Gesture Detection (`gesture_recognition/circle_gesture/`)
- **Function**: Detects circular gesture trajectories
- **Features**: Uses dedicated circle firmware with multi-axis detection and adjustable sensitivity
- **API**: Uses `bmi270_circle_sensor_create()` to create sensor instances
- **Applications**: Smart watches, wearable device gesture control

#### 1.2 Mixed Feature Recognition (`gesture_recognition/mix_feature/`)
- **Function**: Multiple gesture feature recognition
- **Features**: Supports combination detection of multiple gesture patterns
- **API**: Uses standard `bmi270_sensor_create()` to create sensor instances
- **Applications**: Complex gesture recognition applications

### 2. Motion Detection

#### 2.1 Any-Motion Interrupt (`motion_detection/anymotion_int/`)
- **Function**: Any-motion interrupt detection
- **Features**: Real-time response to motion events through GPIO interrupts
- **API**: Uses standard `bmi270_sensor_create()` to create sensor instances
- **Applications**: Motion detection, security monitoring, user interaction

#### 2.2 Z-Axis Complete Flip (`motion_detection/z_shake/`)
- **Function**: Z-axis complete flip detection
- **Features**: Uses state machine to precisely track flip process
- **API**: Uses standard `bmi270_sensor_create()` to create sensor instances
- **Applications**: Device flip detection, gesture recognition

### 3. Power Management

#### 3.1 Any-Motion Wakeup (`power_management/anymotion_wakeup/`)
- **Function**: Motion wakeup from deep sleep
- **Features**: Ultra-low power design with multiple wakeup sources
- **API**: Uses standard `bmi270_sensor_create()` to create sensor instances
- **Applications**: Smart watches, IoT devices, wearable devices

## Firmware Loading Mechanism

### Dynamic Firmware Selection

This repository supports two different firmware loading methods:

#### Standard Firmware
```c
// Use standard firmware
bmi270_handle_t bmi_handle = NULL;
bmi270_i2c_config_t i2c_bmi270_conf = {
    .i2c_handle = i2c_bus,
    .i2c_addr = BMI270_I2C_ADDRESS,
};
bmi270_sensor_create(&i2c_bmi270_conf, &bmi_handle);
```

#### Circle Dedicated Firmware
```c
// Use circle dedicated firmware
bmi270_circle_handle_t bmi_handle = NULL;
bmi270_circle_i2c_config_t i2c_bmi270_conf = {
    .i2c_handle = i2c_bus,
    .i2c_addr = BMI270_I2C_ADDRESS,
};
bmi270_circle_sensor_create(&i2c_bmi270_conf, &bmi_handle);
```

### Firmware Feature Comparison

| Feature | Standard Firmware | Circle Firmware |
|---------|-------------------|-----------------|
| Application | General motion detection | Circular gesture recognition |
| API Prefix | `bmi270_` | `bmi270_circle_` |
| Detection Accuracy | Standard accuracy | High-precision gesture recognition |
| Power Consumption | Standard power | Optimized power |
| Functionality | Basic motion detection | Dedicated gesture algorithms |

## Hardware Support

### Supported Development Boards
- **ESP-SPOT-C5**: Espressif SPOT-C5 development board
- **ESP-SPOT-S3**: Espressif SPOT-S3 development board
- **ESP-ASTOM-S3**: ASTOM-S3 development board
- **ESP-ECHOEAR-S3**: ECHOEAR-S3 development board
- **Custom Development Board**: Supports custom pin configuration

### Pin Configuration
Each example supports pin configuration through menuconfig, including:
- I2C SCL/SDA pins
- Interrupt pins
- LED indicator pins
- Button pins

## Compilation and Running

### Basic Compilation Steps
```bash
# Enter example directory
cd examples/[category]/[example_name]

# Set target chip
idf.py set-target esp32s3

# Configure development board
idf.py menuconfig

# Compile
idf.py build

# Flash and monitor
idf.py flash monitor
```

### Configuration Instructions
1. **Board Selection**: Select corresponding development board model
2. **Pin Configuration**: Configure I2C and interrupt pins
3. **Feature Configuration**: Configure detection parameters and sensitivity

## Example Feature Comparison

| Example | Firmware Type | Main Function | Power Characteristics | Application Scenarios |
|---------|---------------|---------------|----------------------|----------------------|
| Circle Gesture | Circle Firmware | Circular gesture recognition | Medium | Smart watches, wearables |
| Any-Motion Int | Standard Firmware | Motion interrupt detection | Medium | Security monitoring, user interaction |
| Z-Shake | Standard Firmware | Z-axis flip detection | Medium | Device flip detection |
| Any-Motion Wakeup | Standard Firmware | Motion wakeup | Ultra-low | IoT devices, wearables |

## Development Guide

### Choosing Appropriate Firmware
- **Standard Firmware**: Suitable for general motion detection and basic functions
- **Circle Firmware**: Suitable for applications requiring high-precision gesture recognition

### API Usage Recommendations
- Use `bmi270_sensor_create()` for standard function development
- Use `bmi270_circle_sensor_create()` for gesture recognition development
- Choose appropriate detection parameters based on application requirements

### Power Optimization
- Use deep sleep mode to reduce power consumption
- Adjust detection thresholds and duration
- Reasonably configure wakeup sources

## Troubleshooting

### Common Issues
1. **I2C Communication Failure**: Check pin connections and pull-up resistors
2. **Interrupt Not Triggering**: Check interrupt pin configuration
3. **Inaccurate Detection**: Adjust detection parameters
4. **High Power Consumption**: Check peripheral configuration

### Debugging Methods
- Use serial monitor to output information
- Check GPIO status
- Verify sensor configuration
- Test interrupt response

## Contributing

Welcome to submit issues and improvement suggestions:
1. Report bugs and issues
2. Propose new feature suggestions
3. Submit code improvements
4. Improve documentation

## License

This project uses the MIT license. See the LICENSE file for details. 