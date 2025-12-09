# BMI270 Triple Tap Detection Example

## English

### Overview

This example demonstrates how to use the BMI270 Circle sensor for triple tap detection on ESP32 platforms. The example shows how to initialize the BMI270 sensor, configure it for triple tap detection, and handle interrupts using ESP32 GPIO.

### Features

- **Triple Tap Detection**: Detects when the sensor is tapped three times in quick succession
- **Interrupt Handling**: Uses ESP32 GPIO interrupts for efficient event detection
- **Multiple Board Support**: Supports various ESP32 development boards
- **Circle Firmware**: Uses the specialized BMI270 Circle firmware for enhanced gesture recognition
- **Configurable Parameters**: Allows customization of tap sensitivity and timing

### Hardware Requirements

- ESP32 development board (ESP32-S3, ESP32-C5, etc.)
- BMI270 Circle sensor
- I2C connection between ESP32 and BMI270

### Supported Boards

The example automatically detects and configures GPIO pins for the following boards:

- **ESP-SPOT-C5**: INT pin 3, SCL pin 26, SDA pin 25
- **ESP-SPOT-S3**: INT pin 5, SCL pin 1, SDA pin 2  
- **ESP-ASTOM-S3**: INT pin 16, SCL pin 0, SDA pin 45
- **ESP-ECHOEAR-S3**: INT pin 21, SCL pin 1, SDA pin 2
- **Custom**: Uses Kconfig configuration

### Pin Configuration

| Board | INT Pin | SCL Pin | SDA Pin | I2C Type |
|-------|---------|---------|---------|----------|
| ESP-SPOT-C5 | 3 | 26 | 25 | Software |
| ESP-SPOT-S3 | 5 | 1 | 2 | Software |
| ESP-ASTOM-S3 | 16 | 0 | 45 | Software |
| ESP-ECHOEAR-S3 | 21 | 1 | 2 | Hardware |

### How It Works

1. **Initialization**: Configures I2C bus and initializes BMI270 Circle sensor
2. **Sensor Configuration**: Sets up accelerometer and tap detection parameters
3. **Interrupt Setup**: Configures GPIO interrupt for tap detection events
4. **Event Loop**: Continuously monitors for tap events and processes them
5. **Cooling Period**: Implements a 3-second delay between detections to prevent false triggers

### Key Components

#### Sensor Configuration
- Accelerometer ODR: 50Hz
- Tap detection enabled for all axes (X, Y, Z)
- Low-level interrupt configuration for reliable detection

#### Interrupt Handling
- GPIO interrupt on falling edge (matches low-level sensor interrupt)
- 3-second cooling period after detection
- Automatic interrupt re-enabling

#### Tap Detection Logic
- Only detects triple tap events
- Reads tap status register to confirm detection type
- Logs detection events with detailed information

### Build and Flash

```bash
# Navigate to the example directory
cd examples/motion_detection/multi_tap

# Configure the project
idf.py menuconfig

# Build the project
idf.py build

# Flash to your device
idf.py flash monitor
```

### Configuration Options

You can customize the following parameters in `menuconfig`:

- **I2C Configuration**: SCL/SDA pins, I2C frequency
- **Interrupt Pin**: GPIO pin for interrupt detection
- **I2C Type**: Hardware or software I2C implementation

### Expected Output

When a triple tap is detected, you should see output like:

```
I (1234) MAIN: Interrupt detected!
I (1234) MAIN: Tap interrupt detected!
I (1234) MAIN: Tap output: 0x04
I (1234) MAIN: Triple Tap Detected!
I (1234) MAIN: Waiting 3 seconds before next detection...
I (4234) MAIN: Ready for next tap detection...
```

### Troubleshooting

1. **No Interrupts Detected**: Check I2C connection and sensor initialization
2. **False Triggers**: Adjust tap sensitivity or increase cooling period
3. **Build Errors**: Ensure all dependencies are properly installed
4. **GPIO Issues**: Verify interrupt pin configuration matches your board

### API Reference

The example uses the following key APIs:

- `bmi270_circle_sensor_create()`: Initialize BMI270 Circle sensor
- `bmi270_circle_sensor_enable()`: Enable tap detection features
- `bmi270_circle_map_feat_int()`: Map feature interrupts to GPIO
- `bmi2_get_int_status()`: Read interrupt status
- `bmi2_get_regs()`: Read tap status register

---

## 中文

### 概述

本示例演示如何在ESP32平台上使用BMI270 Circle传感器进行三击检测。该示例展示了如何初始化BMI270传感器、配置三击检测功能以及使用ESP32 GPIO处理中断。

### 功能特点

- **三击检测**: 检测传感器在短时间内被连续敲击三次的事件
- **中断处理**: 使用ESP32 GPIO中断进行高效的事件检测
- **多板卡支持**: 支持各种ESP32开发板
- **Circle固件**: 使用专门的BMI270 Circle固件进行增强的手势识别
- **可配置参数**: 允许自定义敲击灵敏度和时序

### 硬件要求

- ESP32开发板（ESP32-S3、ESP32-C5等）
- BMI270 Circle传感器
- ESP32与BMI270之间的I2C连接

### 支持的板卡

示例会自动检测并为以下板卡配置GPIO引脚：

- **ESP-SPOT-C5**: INT引脚3，SCL引脚26，SDA引脚25
- **ESP-SPOT-S3**: INT引脚5，SCL引脚1，SDA引脚2
- **ESP-ASTOM-S3**: INT引脚16，SCL引脚0，SDA引脚45
- **ESP-ECHOEAR-S3**: INT引脚21，SCL引脚1，SDA引脚2
- **自定义**: 使用Kconfig配置

### 引脚配置

| 板卡 | INT引脚 | SCL引脚 | SDA引脚 | I2C类型 |
|------|---------|---------|---------|---------|
| ESP-SPOT-C5 | 3 | 26 | 25 | 软件 |
| ESP-SPOT-S3 | 5 | 1 | 2 | 软件 |
| ESP-ASTOM-S3 | 16 | 0 | 45 | 软件 |
| ESP-ECHOEAR-S3 | 21 | 1 | 2 | 硬件 |

### 工作原理

1. **初始化**: 配置I2C总线并初始化BMI270 Circle传感器
2. **传感器配置**: 设置加速度计和敲击检测参数
3. **中断设置**: 配置GPIO中断用于敲击检测事件
4. **事件循环**: 持续监控敲击事件并处理它们
5. **冷却期**: 在检测之间实现3秒延迟以防止误触发

### 关键组件

#### 传感器配置
- 加速度计输出数据率: 50Hz
- 为所有轴（X、Y、Z）启用敲击检测
- 低电平中断配置以确保可靠检测

#### 中断处理
- GPIO中断在下降沿触发（匹配传感器低电平中断）
- 检测后3秒冷却期
- 自动重新启用中断

#### 敲击检测逻辑
- 仅检测三击事件
- 读取敲击状态寄存器以确认检测类型
- 记录检测事件并显示详细信息

### 编译和烧录

```bash
# 导航到示例目录
cd examples/motion_detection/multi_tap

# 配置项目
idf.py menuconfig

# 编译项目
idf.py build

# 烧录到设备
idf.py flash monitor
```

### 配置选项

您可以在`menuconfig`中自定义以下参数：

- **I2C配置**: SCL/SDA引脚、I2C频率
- **中断引脚**: 用于中断检测的GPIO引脚
- **I2C类型**: 硬件或软件I2C实现

### 预期输出

当检测到三击时，您应该看到类似以下的输出：

```
I (1234) MAIN: Interrupt detected!
I (1234) MAIN: Tap interrupt detected!
I (1234) MAIN: Tap output: 0x04
I (1234) MAIN: Triple Tap Detected!
I (1234) MAIN: Waiting 3 seconds before next detection...
I (4234) MAIN: Ready for next tap detection...
```

### 故障排除

1. **未检测到中断**: 检查I2C连接和传感器初始化
2. **误触发**: 调整敲击灵敏度或增加冷却期
3. **编译错误**: 确保所有依赖项都正确安装
4. **GPIO问题**: 验证中断引脚配置是否与您的板卡匹配

### API参考

示例使用以下关键API：

- `bmi270_circle_sensor_create()`: 初始化BMI270 Circle传感器
- `bmi270_circle_sensor_enable()`: 启用敲击检测功能
- `bmi270_circle_map_feat_int()`: 将功能中断映射到GPIO
- `bmi2_get_int_status()`: 读取中断状态
- `bmi2_get_regs()`: 读取敲击状态寄存器 