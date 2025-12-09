# BMI270 传感器 API 示例

## 概述

本仓库提供了基于 BMI270 传感器的完整示例集合，涵盖了手势识别、运动检测和电源管理等不同应用场景。所有示例都支持多种 ESP32 开发板，并提供了灵活的固件加载机制。

## 示例分类

### 1. 手势识别 (Gesture Recognition)

#### 1.1 Circle Gesture Detection (`gesture_recognition/circle_gesture/`)
- **功能**：检测圆形手势轨迹
- **特点**：使用专用的 circle 固件，支持多轴检测和可调敏感度
- **API**：使用 `bmi270_circle_sensor_create()` 创建传感器实例
- **应用**：智能手表、可穿戴设备的手势控制

#### 1.2 Mixed Feature Recognition (`gesture_recognition/mix_feature/`)
- **功能**：多种手势特征混合识别
- **特点**：支持多种手势模式的组合检测
- **API**：使用标准 `bmi270_sensor_create()` 创建传感器实例
- **应用**：复杂手势识别应用

### 2. 运动检测 (Motion Detection)

#### 2.1 Any-Motion Interrupt (`motion_detection/anymotion_int/`)
- **功能**：任意运动中断检测
- **特点**：通过 GPIO 中断实时响应运动事件
- **API**：使用标准 `bmi270_sensor_create()` 创建传感器实例
- **应用**：运动检测、安全监控、用户交互

#### 2.2 Z-Axis Complete Flip (`motion_detection/z_shake/`)
- **功能**：Z 轴完整翻转检测
- **特点**：使用状态机精确跟踪翻转过程
- **API**：使用标准 `bmi270_sensor_create()` 创建传感器实例
- **应用**：设备翻转检测、手势识别

### 3. 电源管理 (Power Management)

#### 3.1 Any-Motion Wakeup (`power_management/anymotion_wakeup/`)
- **功能**：运动唤醒深度睡眠
- **特点**：超低功耗设计，支持多种唤醒源
- **API**：使用标准 `bmi270_sensor_create()` 创建传感器实例
- **应用**：智能手表、IoT 设备、可穿戴设备

## 固件加载机制

### 动态固件选择

本仓库支持两种不同的固件加载方式：

#### 标准固件
```c
// 使用标准固件
bmi270_handle_t bmi_handle = NULL;
bmi270_i2c_config_t i2c_bmi270_conf = {
    .i2c_handle = i2c_bus,
    .i2c_addr = BMI270_I2C_ADDRESS,
};
bmi270_sensor_create(&i2c_bmi270_conf, &bmi_handle);
```

#### Circle 专用固件
```c
// 使用 circle 专用固件
bmi270_circle_handle_t bmi_handle = NULL;
bmi270_circle_i2c_config_t i2c_bmi270_conf = {
    .i2c_handle = i2c_bus,
    .i2c_addr = BMI270_I2C_ADDRESS,
};
bmi270_circle_sensor_create(&i2c_bmi270_conf, &bmi_handle);
```

### 固件特点对比

| 特性 | 标准固件 | Circle 固件 |
|------|----------|------------|
| 适用场景 | 通用运动检测 | 圆形手势识别 |
| API 前缀 | `bmi270_` | `bmi270_circle_` |
| 检测精度 | 标准精度 | 高精度手势识别 |
| 功耗 | 标准功耗 | 优化功耗 |
| 功能特性 | 基础运动检测 | 专用手势算法 |

## 硬件支持

### 支持的开发板
- **ESP-SPOT-C5**：乐鑫 SPOT-C5 开发板
- **ESP-SPOT-S3**：乐鑫 SPOT-S3 开发板
- **ESP-ASTOM-S3**：ASTOM-S3 开发板
- **ESP-ECHOEAR-S3**：ECHOEAR-S3 开发板
- **自定义开发板**：支持自定义引脚配置

### 引脚配置
每个示例都支持通过 menuconfig 进行引脚配置，包括：
- I2C SCL/SDA 引脚
- 中断引脚
- LED 指示灯引脚
- 按键引脚

## 编译和运行

### 基本编译步骤
```bash
# 进入示例目录
cd examples/[category]/[example_name]

# 设置目标芯片
idf.py set-target esp32s3

# 配置开发板
idf.py menuconfig

# 编译
idf.py build

# 烧录和监控
idf.py flash monitor
```

### 配置说明
1. **Board Selection**：选择对应的开发板型号
2. **Pin Configuration**：配置 I2C 和中断引脚
3. **Feature Configuration**：配置检测参数和敏感度

## 示例特性对比

| 示例 | 固件类型 | 主要功能 | 功耗特性 | 应用场景 |
|------|----------|----------|----------|----------|
| Circle Gesture | Circle 固件 | 圆形手势识别 | 中等 | 智能手表、可穿戴 |
| Any-Motion Int | 标准固件 | 运动中断检测 | 中等 | 安全监控、用户交互 |
| Z-Shake | 标准固件 | Z 轴翻转检测 | 中等 | 设备翻转检测 |
| Any-Motion Wakeup | 标准固件 | 运动唤醒 | 超低 | IoT 设备、可穿戴 |

## 开发指南

### 选择合适固件
- **标准固件**：适用于通用运动检测和基础功能
- **Circle 固件**：适用于需要高精度手势识别的应用

### API 使用建议
- 使用 `bmi270_sensor_create()` 进行标准功能开发
- 使用 `bmi270_circle_sensor_create()` 进行手势识别开发
- 根据应用需求选择合适的检测参数

### 功耗优化
- 使用深度睡眠模式降低功耗
- 调整检测阈值和持续时间
- 合理配置唤醒源

## 故障排除

### 常见问题
1. **I2C 通信失败**：检查 I2C 引脚连接和上拉电阻
2. **中断不触发**：检查中断引脚配置
3. **检测不准确**：调整检测参数
4. **功耗过高**：检查外设配置

### 调试方法
- 使用串口监控输出信息
- 检查 GPIO 状态
- 验证传感器配置
- 测试中断响应

## 贡献指南

欢迎提交问题和改进建议：
1. 报告 bug 和问题
2. 提出新功能建议
3. 提交代码改进
4. 完善文档说明

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。 