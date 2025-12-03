# ESP-HALO Display Example

This example demonstrates the LCD and touch functionality of the ESP-HALO board using the BSP (Board Support Package).

## Features

- ILI9341 LCD controller (284x240 resolution)
- CST816S capacitive touch controller
- LVGL graphics library integration
- Touch event handling demonstration

## Hardware Requirements

- ESP-HALO board (Open Source or Production version)
- ILI9341 LCD display
- CST816S touch panel

## How to Use

### Build and Flash

```bash
idf.py build flash monitor
```

### Configuration

You can configure the LCD interface type using menuconfig:

```bash
idf.py menuconfig
```

Navigate to: `Board Support Package` -> `LCD Interface Type`

- **SPI**: Standard SPI interface (default)
- **Parallel IO**: Use Parlio to simulate SPI or use parallel mode

### Expected Behavior

1. Display shows "ESP-HALO Display Test - CST816S Touch Enabled" label
2. A button labeled "Touch Me!" is displayed in the center
3. Touch counter is displayed at the bottom
4. Each touch increments the counter and logs the event

## Pin Configuration

### Open Source Board
- LCD MOSI: GPIO0
- LCD SCLK: GPIO1
- LCD CS: GPIO6
- LCD DC: GPIO7
- Touch INT: GPIO4
- I2C SDA: GPIO2
- I2C SCL: GPIO3

### Production Board
- LCD MOSI: GPIO23
- LCD SCLK: GPIO24
- LCD CS: GPIO25
- LCD DC: GPIO26
- I2C SDA: GPIO2
- I2C SCL: GPIO3
