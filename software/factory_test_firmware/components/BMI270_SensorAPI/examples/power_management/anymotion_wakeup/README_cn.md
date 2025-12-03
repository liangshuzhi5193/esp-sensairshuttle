# BMI270 Any-Motion 唤醒示例

## 概述

这个示例演示了如何使用BMI270传感器的any-motion功能来实现低功耗唤醒。当设备处于深度睡眠模式时，BMI270会持续监测运动，一旦检测到任何运动就会通过GPIO中断唤醒ESP32。该例程展示了如何结合ESP32的深度睡眠功能和BMI270的运动检测来实现超低功耗的唤醒机制。

## 功能特性

- **深度睡眠**：ESP32进入超低功耗的深度睡眠模式
- **Any-Motion唤醒**：BMI270检测运动并通过中断唤醒设备
- **多唤醒源**：支持按键、IMU中断等多种唤醒方式
- **电池电压监测**：集成ADC监测电池电压
- **LED指示**：通过LED显示系统状态
- **自动休眠**：无活动时自动进入深度睡眠

## 硬件要求

- ESP32开发板（支持以下型号）：
  - ESP32-S3
  - ESP32-C5
- BMI270传感器模块
- 按键和LED指示灯
- 电池电源（可选）

## 引脚配置

### ESP32-S3 引脚定义
```c
#define POWER_CTRL_GPIO  GPIO_NUM_4
#define BOOT_PIN         GPIO_NUM_0   // BOOT按钮
#define KEY_PIN          GPIO_NUM_12  // 用户按键
#define LED_PIN          GPIO_NUM_11  // LED指示灯
#define VBAT_ADC_CHANNEL ADC_CHANNEL_9  // 电池电压ADC
#define IMU_INT_PIN      GPIO_NUM_5     // IMU中断引脚
#define I2C_MASTER_SCL_IO       1
#define I2C_MASTER_SDA_IO       2
```

### ESP32-C5 引脚定义
```c
#define POWER_CTRL_GPIO  GPIO_NUM_2
#define BOOT_PIN         GPIO_NUM_28  // BOOT按钮
#define KEY_PIN          GPIO_NUM_5   // 用户按键
#define LED_PIN          GPIO_NUM_27  // LED指示灯
#define VBAT_ADC_CHANNEL ADC_CHANNEL_3  // 电池电压ADC
#define IMU_INT_PIN      GPIO_NUM_3     // IMU中断引脚
#define I2C_MASTER_SCL_IO       26
#define I2C_MASTER_SDA_IO       25
```

## 工作原理

### 深度睡眠机制

1. **睡眠准备**：配置GPIO、启用唤醒源
2. **进入睡眠**：ESP32进入深度睡眠模式
3. **唤醒检测**：BMI270持续监测运动
4. **中断唤醒**：检测到运动时通过GPIO中断唤醒

### Any-Motion配置

```c
// Any-Motion检测参数
config.cfg.any_motion.duration = 4;      // 持续时间：80ms
config.cfg.any_motion.threshold = 0x68;  // 阈值：约50mg

// 中断引脚配置
pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;  // 高电平有效
pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;     // 推挽输出
pin_config.int_latch = BMI2_INT_NON_LATCH;         // 非锁存模式
```

### 唤醒源配置

```c
// 配置EXT1唤醒源（按键和IMU中断）
esp_sleep_enable_ext1_wakeup((1ULL << KEY_PIN) | (1ULL << IMU_INT_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);

// 配置EXT0唤醒源（BOOT按键）
esp_sleep_enable_ext0_wakeup(BOOT_PIN, 0);
```

## 使用方法

### 1. 编译和烧录

```bash
cd examples/power_management/anymotion_wakeup
idf.py set-target esp32s3  # 根据你的开发板选择目标
idf.py build
idf.py flash monitor
```

### 2. 测试唤醒功能

1. **系统启动**：设备启动后显示配置信息
2. **自动休眠**：10秒无活动后自动进入深度睡眠
3. **运动唤醒**：移动设备触发any-motion唤醒
4. **按键唤醒**：按下用户按键唤醒设备
5. **BOOT唤醒**：按下BOOT按键唤醒设备

## 输出示例

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

## 功耗特性

### 深度睡眠功耗
- **ESP32深度睡眠**：约10μA
- **BMI270工作模式**：约1.5mA
- **总功耗**：约1.51mA（运动检测时）

### 唤醒时间
- **ESP32唤醒时间**：约1-2ms
- **BMI270响应时间**：约50-100ms
- **总唤醒延迟**：约100-200ms

## 参数调整

### 提高唤醒敏感度
```c
config.cfg.any_motion.threshold = 0x40;  // 降低阈值到约32mg
config.cfg.any_motion.duration = 2;      // 减少持续时间到40ms
```

### 降低唤醒敏感度
```c
config.cfg.any_motion.threshold = 0x80;  // 提高阈值到约64mg
config.cfg.any_motion.duration = 8;      // 增加持续时间到160ms
```

### 调整休眠超时
```c
const TickType_t sleep_timeout = pdMS_TO_TICKS(5000);  // 5秒后休眠
const TickType_t sleep_timeout = pdMS_TO_TICKS(30000); // 30秒后休眠
```

## 电池电压监测

### ADC配置
```c
// ADC通道配置
adc_oneshot_chan_cfg_t chan_config = {
    .atten = ADC_ATTEN_DB_12,      // 12dB衰减
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};

// 电压计算
voltage = voltage * 3 / 2;  // 分压器补偿
```

### 电池状态监测
- **电压范围**：0-3.3V（12dB衰减）
- **分辨率**：12位ADC
- **校准支持**：支持曲线拟合校准

## 注意事项

- 确保BMI270传感器正确连接到ESP32的I2C引脚
- 中断引脚需要正确连接到BMI270的INT1引脚
- 深度睡眠期间大部分外设会被关闭
- 唤醒后需要重新初始化部分外设
- 根据实际应用调整运动检测参数

## 故障排除

1. **无法唤醒**：检查中断引脚连接和配置
2. **误唤醒**：调整any-motion阈值和持续时间
3. **功耗过高**：检查是否有外设未正确关闭
4. **唤醒延迟**：优化中断响应时间

## 应用场景

- **智能手表**：运动检测唤醒显示
- **IoT设备**：低功耗运动监测
- **安全监控**：设备移动检测
- **可穿戴设备**：手势唤醒功能
- **工业传感器**：运动触发数据采集
