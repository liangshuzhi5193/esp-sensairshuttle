# BMI270 Any-Motion Wakeup Example

## Overview

This example demonstrates how to use the BMI270 sensor's any-motion functionality to implement low-power wakeup. When the device is in deep sleep mode, the BMI270 continuously monitors motion, and once any motion is detected, it wakes up the ESP32 through a GPIO interrupt. This example shows how to combine ESP32's deep sleep functionality with BMI270's motion detection to achieve ultra-low-power wakeup mechanisms.

## Features

- **Deep Sleep**: ESP32 enters ultra-low-power deep sleep mode
- **Any-Motion Wakeup**: BMI270 detects motion and wakes up device through interrupt
- **Multiple Wakeup Sources**: Supports button, IMU interrupt, and other wakeup methods
- **Battery Voltage Monitoring**: Integrated ADC for battery voltage monitoring
- **LED Indication**: System status display through LED
- **Auto Sleep**: Automatically enters deep sleep when no activity

## Hardware Requirements

- ESP32 development board (supports the following models):
  - ESP32-S3
  - ESP32-C5
- BMI270 sensor module
- Buttons and LED indicators
- Battery power supply (optional)

## Pin Configuration

### ESP32-S3 Pin Definitions
```c
#define POWER_CTRL_GPIO  GPIO_NUM_4
#define BOOT_PIN         GPIO_NUM_0   // BOOT button
#define KEY_PIN          GPIO_NUM_12  // User button
#define LED_PIN          GPIO_NUM_11  // LED indicator
#define VBAT_ADC_CHANNEL ADC_CHANNEL_9  // Battery voltage ADC
#define IMU_INT_PIN      GPIO_NUM_5     // IMU interrupt pin
#define I2C_MASTER_SCL_IO       1
#define I2C_MASTER_SDA_IO       2
```

### ESP32-C5 Pin Definitions
```c
#define POWER_CTRL_GPIO  GPIO_NUM_2
#define BOOT_PIN         GPIO_NUM_28  // BOOT button
#define KEY_PIN          GPIO_NUM_5   // User button
#define LED_PIN          GPIO_NUM_27  // LED indicator
#define VBAT_ADC_CHANNEL ADC_CHANNEL_3  // Battery voltage ADC
#define IMU_INT_PIN      GPIO_NUM_3     // IMU interrupt pin
#define I2C_MASTER_SCL_IO       26
#define I2C_MASTER_SDA_IO       25
```

## Working Principle

### Deep Sleep Mechanism

1. **Sleep Preparation**: Configure GPIO, enable wakeup sources
2. **Enter Sleep**: ESP32 enters deep sleep mode
3. **Wakeup Detection**: BMI270 continuously monitors motion
4. **Interrupt Wakeup**: Wake up through GPIO interrupt when motion is detected

### Any-Motion Configuration

```c
// Any-Motion detection parameters
config.cfg.any_motion.duration = 4;      // Duration: 80ms
config.cfg.any_motion.threshold = 0x68;  // Threshold: ~50mg

// Interrupt pin configuration
pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;  // Active high
pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;     // Push-pull output
pin_config.int_latch = BMI2_INT_NON_LATCH;         // Non-latch mode
```

### Wakeup Source Configuration

```c
// Configure EXT1 wakeup source (button and IMU interrupt)
esp_sleep_enable_ext1_wakeup((1ULL << KEY_PIN) | (1ULL << IMU_INT_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);

// Configure EXT0 wakeup source (BOOT button)
esp_sleep_enable_ext0_wakeup(BOOT_PIN, 0);
```

## Usage

### 1. Compile and Flash

```bash
cd examples/power_management/anymotion_wakeup
idf.py set-target esp32s3  # Select target according to your development board
idf.py build
idf.py flash monitor
```

### 2. Test Wakeup Function

1. **System Startup**: Device displays configuration information after startup
2. **Auto Sleep**: Automatically enters deep sleep after 10 seconds of inactivity
3. **Motion Wakeup**: Move device to trigger any-motion wakeup
4. **Button Wakeup**: Press user button to wake up device
5. **BOOT Wakeup**: Press BOOT button to wake up device

## Output Example

```
I (xxx) DEEPSLEEP_WAKEUP: System starting...
I (xxx) DEEPSLEEP_WAKEUP: Configuring GPIOs for deep sleep wakeup...
I (xxx) DEEPSLEEP_WAKEUP: GPIOs configured.
I (xxx) DEEPSLEEP_WAKEUP: Setting up wakeup sources...
I (xxx) DEEPSLEEP_WAKEUP: Entering deep sleep...
I (xxx) DEEPSLEEP_WAKEUP: Wakeup from IMU_INT_PIN
I (xxx) DEEPSLEEP_WAKEUP: IMU motion detected
I (xxx) DEEPSLEEP_WAKEUP: No activity, going to sleep
I (xxx) DEEPSLEEP_WAKEUP: Setting up wakeup sources...
I (xxx) DEEPSLEEP_WAKEUP: Entering deep sleep...
```

## Power Characteristics

### Deep Sleep Power Consumption
- **ESP32 Deep Sleep**: ~10μA
- **BMI270 Active Mode**: ~1.5mA
- **Total Power**: ~1.51mA (during motion detection)

### Wakeup Time
- **ESP32 Wakeup Time**: ~1-2ms
- **BMI270 Response Time**: ~50-100ms
- **Total Wakeup Delay**: ~100-200ms

## Parameter Adjustment

### Increase Wakeup Sensitivity
```c
config.cfg.any_motion.threshold = 0x40;  // Lower threshold to ~32mg
config.cfg.any_motion.duration = 2;      // Reduce duration to 40ms
```

### Decrease Wakeup Sensitivity
```c
config.cfg.any_motion.threshold = 0x80;  // Increase threshold to ~64mg
config.cfg.any_motion.duration = 8;      // Increase duration to 160ms
```

### Adjust Sleep Timeout
```c
const TickType_t sleep_timeout = pdMS_TO_TICKS(5000);  // Sleep after 5 seconds
const TickType_t sleep_timeout = pdMS_TO_TICKS(30000); // Sleep after 30 seconds
```

## Battery Voltage Monitoring

### ADC Configuration
```c
// ADC channel configuration
adc_oneshot_chan_cfg_t chan_config = {
    .atten = ADC_ATTEN_DB_12,      // 12dB attenuation
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};

// Voltage calculation
voltage = voltage * 3 / 2;  // Voltage divider compensation
```

### Battery Status Monitoring
- **Voltage Range**: 0-3.3V (12dB attenuation)
- **Resolution**: 12-bit ADC
- **Calibration Support**: Curve fitting calibration supported

## Notes

- Ensure the BMI270 sensor is correctly connected to the ESP32 I2C pins
- The interrupt pin needs to be correctly connected to the BMI270 INT1 pin
- Most peripherals are turned off during deep sleep
- Some peripherals need to be reinitialized after wakeup
- Adjust motion detection parameters according to actual applications

## Troubleshooting

1. **Cannot Wake Up**: Check interrupt pin connections and configuration
2. **False Wakeup**: Adjust any-motion threshold and duration
3. **High Power Consumption**: Check if peripherals are not properly turned off
4. **Wakeup Delay**: Optimize interrupt response time

## Application Scenarios

- **Smart Watches**: Motion detection wakeup display
- **IoT Devices**: Low-power motion monitoring
- **Security Monitoring**: Device movement detection
- **Wearable Devices**: Gesture wakeup functionality
- **Industrial Sensors**: Motion-triggered data collection 