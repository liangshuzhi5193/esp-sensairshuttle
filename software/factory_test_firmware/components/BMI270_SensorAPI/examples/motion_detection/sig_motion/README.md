# BMI270 Significant Motion Detection Example

This example demonstrates how to use the BMI270 sensor for significant motion detection with ESP32. The example is based on the [Bosch BMI270 SensorAPI sig_motion_hw_int example](https://github.com/boschsensortec/BMI270_SensorAPI/blob/master/bmi270_examples/sig_motion_hw_int/sig_motion_hw_int.c).

## Features

- **Significant Motion Detection**: Detects when the device has moved significantly
- **Hardware Interrupt**: Uses BMI270's INT1 pin to trigger interrupts
- **GPIO Interrupt Handling**: ESP32 GPIO interrupt service routine
- **I2C Communication**: Software and hardware I2C support
- **Multiple Board Support**: Configurable for different ESP32 development boards

## Hardware Requirements

- ESP32 development board
- BMI270 sensor module
- I2C connections (SCL, SDA)
- Interrupt connection (INT1 pin)

## Board Configuration

The example supports multiple ESP32 development boards:

- **ESP SPOT C5**: GPIO 3 (INT), GPIO 26 (SCL), GPIO 25 (SDA)
- **ESP SPOT S3**: GPIO 5 (INT), GPIO 1 (SCL), GPIO 2 (SDA)
- **ESP ASTOM S3**: GPIO 16 (INT), GPIO 0 (SCL), GPIO 45 (SDA)
- **ESP ECHOEAR S3**: GPIO 21 (INT), GPIO 1 (SCL), GPIO 2 (SDA)
- **Custom Boards**: Configurable via menuconfig

## Configuration

### I2C Configuration

- **I2C Address**: 0x68 (default BMI270 address)
- **I2C Frequency**: 100kHz
- **I2C Type**: Hardware I2C (configurable for software I2C)

### Interrupt Configuration

- **Interrupt Pin**: INT1
- **Interrupt Level**: Active Low
- **GPIO Trigger**: Any Edge
- **Interrupt Type**: Non-latched

### Significant Motion Configuration

- **Block Size**: 0x01 (medium sensitivity - balanced trigger)
- **Accelerometer ODR**: 50Hz
- **Detection Sensitivity**: Medium (block_size = 0x01)

## Usage

1. **Connect Hardware**:
   - Connect BMI270 SCL to ESP32 SCL pin
   - Connect BMI270 SDA to ESP32 SDA pin
   - Connect BMI270 INT1 to ESP32 interrupt pin
   - Connect BMI270 VCC to 3.3V
   - Connect BMI270 GND to GND

2. **Build and Flash**:
   ```bash
   cd examples/motion_detection/sig_motion
   idf.py build
   idf.py flash monitor
   ```

3. **Test the Example**:
   - **Easy triggers**: Picking up the device, gentle shaking
   - **Medium triggers**: Rotating the device, moving it from one position to another
   - **Strong triggers**: Vigorous shaking, dropping and catching the device
   - Monitor the serial output for detection messages
   - The example waits 3 seconds between detections

## Expected Output

```
I (108) MAIN: Setting sig_motion block_size to 0x01 for medium sensitivity
I (108) MAIN: Sig_motion configuration set successfully with block_size: 0x01
I (108) MAIN: Significant motion feature enabled successfully
I (108) MAIN: Move the device to detect significant motion
I (108) MAIN: Interrupt detected!
I (108) MAIN: Interrupt status: 0x0001
I (108) MAIN: Sig_motion mask: 0x0001
I (108) MAIN: Significant motion interrupt is generated!
I (108) MAIN: Waiting 3 seconds before next detection...
I (108) MAIN: Ready for next significant motion detection...
```

## API Reference

### Key Functions

- `i2c_sensor_bmi270_init()`: Initialize I2C and BMI270 sensor
- `set_feature_config()`: Configure significant motion detection parameters
- `bmi270_enable_sig_motion_int()`: Enable significant motion detection with interrupts
- `gpio_isr_edge_handler()`: GPIO interrupt service routine

### Key Structures

- `struct bmi2_dev`: BMI270 device structure
- `struct bmi2_sens_config`: Sensor configuration structure
- `struct bmi2_int_pin_config`: Interrupt pin configuration structure

## Troubleshooting

### Common Issues

1. **No Interrupt Detected**:
   - Check INT1 pin connection
   - Verify interrupt level configuration
   - Ensure device is moving significantly

2. **I2C Communication Errors**:
   - Check SCL/SDA connections
   - Verify I2C address (0x68)
   - Check power supply (3.3V)

3. **False Detections**:
   - Adjust block_size parameter
   - Modify accelerometer ODR
   - Check for vibration sources

### Debug Information

The example provides detailed debug output:
- Sensor initialization status
- Configuration parameters
- Interrupt status values
- Detection events

## References

- [BMI270 SensorAPI Documentation](https://github.com/boschsensortec/BMI270_SensorAPI)
- [ESP-IDF GPIO Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/gpio.html)
- [ESP-IDF I2C Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/i2c.html) 