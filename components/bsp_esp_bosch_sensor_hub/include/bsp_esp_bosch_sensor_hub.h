/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "iot_button.h"
#include "display.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct {
    lvgl_port_cfg_t lvgl_port_cfg;  /*!< LVGL port configuration */
    uint32_t        buffer_size;    /*!< Size of the buffer for the screen in pixels */
    bool            double_buffer;  /*!< True, if should be allocated two buffers */
    struct {
        unsigned int buff_dma: 1;    /*!< Allocated LVGL buffer will be DMA capable */
        unsigned int buff_spiram: 1; /*!< Allocated LVGL buffer will be in PSRAM */
    } flags;
} bsp_display_cfg_t;

/* Display */
#define BSP_LCD_DATA0         (GPIO_NUM_25)
#define BSP_LCD_PCLK          (GPIO_NUM_24)
#define BSP_LCD_CS            (GPIO_NUM_NC)
#define BSP_LCD_DC            (GPIO_NUM_23)
#define BSP_LCD_RST           (GPIO_NUM_NC)

#define BSP_I2C_SCL           (GPIO_NUM_26)
#define BSP_I2C_SDA           (GPIO_NUM_27)
#define BSP_LCD_TOUCH_INT     (GPIO_NUM_NC)

/**************************************************************************************************
 *
 * LCD interface
 *
 * ESP-BOX is shipped with 2.4inch ST7789 display controller.
 * It features 16-bit colors, 320x240 resolution and capacitive touch controller.
 *
 * LVGL is used as graphics library. LVGL is NOT thread safe, therefore the user must take LVGL mutex
 * by calling bsp_display_lock() before calling and LVGL API (lv_...) and then give the mutex with
 * bsp_display_unlock().
 *
 * Display's backlight must be enabled explicitly by calling bsp_display_backlight_on()
 **************************************************************************************************/
#define BSP_LCD_PIXEL_CLOCK_HZ     (40 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)

/**************************************************************************************************
 *
 * I2C interface
 *
 * There are multiple devices connected to I2C peripheral:
 *  - Codec ES8311 (configuration only)
 *  - ADC ES7210 (configuration only)
 *  - Encryption chip ATECC608A (NOT populated on most boards)
 *  - LCD Touch controller
 *  - Inertial Measurement Unit ICM-42607-P
 *
 * After initialization of I2C, use BSP_I2C_NUM macro when creating I2C devices drivers ie.:
 * \code{.c}
 * icm42670_handle_t imu = icm42670_create(BSP_I2C_NUM, ICM42670_I2C_ADDRESS);
 * \endcode
 **************************************************************************************************/
#define BSP_I2C_NUM     CONFIG_BSP_I2C_NUM

/**
 * @brief Init I2C driver
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 *      - ESP_FAIL              I2C driver installation error
 *
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling bsp_display_brightness_set()
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_disp_t *bsp_display_start(void);

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling bsp_display_brightness_set()
 *
 * @param cfg display configuration
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_disp_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Take LVGL mutex
 *
 * @param timeout_ms Timeout in [ms]. 0 will block indefinitely.
 * @return true  Mutex was taken
 * @return false Mutex was NOT taken
 */
bool bsp_display_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 *
 */
void bsp_display_unlock(void);

#ifdef __cplusplus
}
#endif
