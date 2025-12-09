/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP BSP: ESP-HALE
 */

#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/i2s_pdm.h"
#include "driver/i2c_master.h"
#include "iot_button.h"
#include "esp_codec_dev.h"
#include "bsp/config.h"
#include "bsp/display.h"

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
#include "lvgl.h"
#include "esp_lvgl_port.h"
#endif

/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/

#define BSP_CAPS_DISPLAY        1
#define BSP_CAPS_TOUCH          1
#define BSP_CAPS_BUTTONS        1
#define BSP_CAPS_AUDIO          1
#define BSP_CAPS_AUDIO_SPEAKER  1
#define BSP_CAPS_AUDIO_MIC      1
#define BSP_CAPS_SDCARD         0
#define BSP_CAPS_IMU            0
#define BSP_CAPS_CAMERA         0

/**************************************************************************************************
 *  ESP-HALE pinout
 **************************************************************************************************/

/*==============================================================================
 *  Select board version (choose ONE)
 *============================================================================*/
//#define ESP_HALE_BOARD_OPEN_SOURCE   1
#define ESP_HALE_BOARD_PRODUCTION    1

/* Button */
#define BSP_BUTTON_BOOT          (GPIO_NUM_28)

/* I2C */
#define BSP_I2C_SDA              (GPIO_NUM_2)
#define BSP_I2C_SCL              (GPIO_NUM_3)

/* Light */
#if defined(ESP_HALE_BOARD_OPEN_SOURCE)
#define BSP_RGB_CTRL             (GPIO_NUM_26)
#elif defined(ESP_HALE_BOARD_PRODUCTION)
#define BSP_RGB_CTRL             (GPIO_NUM_0)
#endif

/* Audio */
#if defined(ESP_HALE_BOARD_OPEN_SOURCE)
#define AUDIO_ADC_MIC_CHANNEL    (4)
#define AUDIO_PDM_SPEAK_P_GPIO   (GPIO_NUM_9)
#define AUDIO_PDM_SPEAK_N_GPIO   (GPIO_NUM_10)
#define AUDIO_PA_CTL_GPIO        (GPIO_NUM_8)
#elif defined(ESP_HALE_BOARD_PRODUCTION)
#define AUDIO_ADC_MIC_CHANNEL    (5)
#define AUDIO_PDM_SPEAK_P_GPIO   (GPIO_NUM_7)
#define AUDIO_PDM_SPEAK_N_GPIO   (GPIO_NUM_8)
#define AUDIO_PA_CTL_GPIO        (GPIO_NUM_1)
#endif

#define AUDIO_INPUT_SAMPLE_RATE  (16000)
#define AUDIO_OUTPUT_SAMPLE_RATE (16000)


/* LCD */
#if defined(ESP_HALE_BOARD_OPEN_SOURCE)
#define BSP_LCD_PIN_NUM_SCLK     (GPIO_NUM_1)
#define BSP_LCD_PIN_NUM_MOSI     (GPIO_NUM_0)
#define BSP_LCD_PIN_NUM_DC       (GPIO_NUM_7)
#define BSP_LCD_PIN_NUM_CS       (GPIO_NUM_6)
#elif defined(ESP_HALE_BOARD_PRODUCTION)
#define BSP_LCD_PIN_NUM_SCLK     (GPIO_NUM_24)
#define BSP_LCD_PIN_NUM_MOSI     (GPIO_NUM_23)
#define BSP_LCD_PIN_NUM_DC       (GPIO_NUM_26)
#define BSP_LCD_PIN_NUM_CS       (GPIO_NUM_25)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * I2S audio interface
 *
 * There is one device connected to the I2S peripheral:
 *  - PDM for output path
 *
 * For speaker initialization use bsp_audio_codec_speaker_init() which is inside initialize I2S with bsp_audio_init().
 * After speaker initialization, use functions from esp_codec_dev for play audio.
 * Example audio play:
 * \code{.c}
 * esp_codec_dev_open(spk_codec_dev, &fs);
 * esp_codec_dev_write(spk_codec_dev, wav_bytes, bytes_read_from_spiffs);
 * esp_codec_dev_close(spk_codec_dev);
 * \endcode
 **************************************************************************************************/

/**
 * @brief Initialize microphone codec device
 *
 * @return Pointer to codec device handle or NULL when error occurred
 */
esp_codec_dev_handle_t bsp_audio_codec_microphone_init(void);

/**
 * @brief Initialize speaker codec device
 *
 * @return Pointer to codec device handle or NULL when error occurred
 */
esp_codec_dev_handle_t bsp_audio_codec_speaker_init(void);


#define BSP_LED_STRIP_COUNT     (7)     /*!< Total number of LEDs in the strip */
#define BSP_LED_ALL_INDEX       (0xFF)  /*!< Special index to control all LEDs */

typedef enum {
    BSP_LED_INDEX_0 = 0,    /*!< LED index 0 */
    BSP_LED_INDEX_1,        /*!< LED index 1 */
    BSP_LED_INDEX_2,        /*!< LED index 2 */
    BSP_LED_INDEX_3,        /*!< LED index 3 */
    BSP_LED_INDEX_4,        /*!< LED index 4 */
    BSP_LED_INDEX_5,        /*!< LED index 5 */
    BSP_LED_INDEX_6,        /*!< LED index 6 */
    BSP_LED_INDEX_MAX = BSP_LED_STRIP_COUNT
} bsp_led_index_t;

typedef enum {
    BSP_LED_COLOR_OFF     = 0x000000,   /*!< LED off (black) */
    BSP_LED_COLOR_RED     = 0xFF0000,   /*!< Pure red */
    BSP_LED_COLOR_GREEN   = 0x00FF00,   /*!< Pure green */
    BSP_LED_COLOR_BLUE    = 0x0000FF,   /*!< Pure blue */
    BSP_LED_COLOR_WHITE   = 0xFFFFFF,   /*!< Pure white */
    BSP_LED_COLOR_YELLOW  = 0xFFFF00,   /*!< Yellow */
    BSP_LED_COLOR_CYAN    = 0x00FFFF,   /*!< Cyan */
    BSP_LED_COLOR_MAGENTA = 0xFF00FF,   /*!< Magenta */
    BSP_LED_COLOR_ORANGE  = 0xFF8000,   /*!< Orange */
    BSP_LED_COLOR_PURPLE  = 0x800080,   /*!< Purple */
} bsp_led_color_t;

typedef enum {
    BSP_LED_EFFECT_STATIC,       /*!< Static color */
    BSP_LED_EFFECT_BREATHING,    /*!< Breathing effect */
    BSP_LED_EFFECT_RAINBOW,      /*!< Rainbow effect */
    BSP_LED_EFFECT_CHASE,        /*!< Chase effect */
    BSP_LED_EFFECT_BLINK,        /*!< Blink effect */
    BSP_LED_EFFECT_FADE,         /*!< Fade effect */
    BSP_LED_EFFECT_MAX
} bsp_led_effect_t;

typedef enum {
    BSP_LED_BRIGHTNESS_0 = 0,       /*!< 0% brightness */
    BSP_LED_BRIGHTNESS_1 = 16,      /*!< ~6% brightness */
    BSP_LED_BRIGHTNESS_2 = 32,      /*!< ~13% brightness */
    BSP_LED_BRIGHTNESS_3 = 48,      /*!< ~19% brightness */
    BSP_LED_BRIGHTNESS_4 = 64,      /*!< ~25% brightness */
    BSP_LED_BRIGHTNESS_5 = 80,      /*!< ~31% brightness */
    BSP_LED_BRIGHTNESS_6 = 96,      /*!< ~38% brightness */
    BSP_LED_BRIGHTNESS_7 = 112,     /*!< ~44% brightness */
    BSP_LED_BRIGHTNESS_8 = 128,     /*!< ~50% brightness */
    BSP_LED_BRIGHTNESS_9 = 144,     /*!< ~56% brightness */
    BSP_LED_BRIGHTNESS_10 = 160,    /*!< ~63% brightness */
    BSP_LED_BRIGHTNESS_11 = 176,    /*!< ~69% brightness */
    BSP_LED_BRIGHTNESS_12 = 192,    /*!< ~75% brightness */
    BSP_LED_BRIGHTNESS_13 = 208,    /*!< ~82% brightness */
    BSP_LED_BRIGHTNESS_14 = 224,    /*!< ~88% brightness */
    BSP_LED_BRIGHTNESS_15 = 240,    /*!< ~94% brightness */
    BSP_LED_BRIGHTNESS_MAX = 255,   /*!< 100% brightness */
} bsp_led_brightness_t;

typedef struct {
    uint32_t color;           /*!< RGB color (0xRRGGBB format) */
    uint8_t brightness;       /*!< Brightness (0-255) */
    bsp_led_effect_t effect;  /*!< Effect mode */
    uint16_t effect_speed;    /*!< Effect speed (ms) */
} bsp_led_config_t;

/**
 * @brief Initialize WS2812 LED strip
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Initialize failed
 */
esp_err_t bsp_led_init();

/**
 * @brief Deinitialize LED strip
 *
 * @return
 *     - ESP_OK Success  
 *     - ESP_FAIL Deinitialize failed
 */
esp_err_t bsp_led_deinit();

/**
 * @brief Set RGB color for a specific LED
 *
 * @param index LED index (0-5) or BSP_LED_ALL_INDEX for all LEDs
 * @param r Red component (0-255)
 * @param g Green component (0-255) 
 * @param b Blue component (0-255)
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid LED index or color value
 *      - ESP_FAIL Set color failed
 */
esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set color for a specific LED using 32-bit RGB value
 *
 * @param index LED index (0-5) or BSP_LED_ALL_INDEX for all LEDs
 * @param color RGB color in 0xRRGGBB format
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid LED index
 *      - ESP_FAIL Set color failed
 */
esp_err_t bsp_led_set_color(uint8_t index, uint32_t color);

/**
 * @brief Set HSV color for a specific LED
 *
 * @param index LED index (0-5) or BSP_LED_ALL_INDEX for all LEDs
 * @param hue Hue (0-360)
 * @param saturation Saturation (0-255)
 * @param value Value/brightness (0-255)
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid parameters
 *      - ESP_FAIL Set color failed
 */
esp_err_t bsp_led_set_hsv(uint8_t index, uint16_t hue, uint8_t saturation, uint8_t value);

/**
 * @brief Set RGB colors for all LEDs at once
 *
 * @param colors Array of RGB colors (length must be BSP_LED_STRIP_COUNT)
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid parameter
 *      - ESP_FAIL Set colors failed
 */
esp_err_t bsp_led_set_colors(const uint32_t *colors);

/**
 * @brief Set same RGB color for all LEDs
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_FAIL Set color failed
 */
esp_err_t bsp_led_set_all_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Turn off specific LED or all LEDs
 *
 * @param index LED index (0-5) or BSP_LED_ALL_INDEX for all LEDs
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid LED index
 *      - ESP_FAIL Operation failed
 */
esp_err_t bsp_led_clear(uint8_t index);

/**
 * @brief Turn off all LEDs
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_FAIL Operation failed
 */
esp_err_t bsp_led_clear_all();

/**
 * @brief Set brightness for specific LED
 *
 * @param index LED index (0-5) or BSP_LED_ALL_INDEX for all LEDs
 * @param brightness Brightness level (0-255)
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid parameters
 *      - ESP_FAIL Set brightness failed
 */
esp_err_t bsp_led_set_brightness(uint8_t index, uint8_t brightness);

/**
 * @brief Start LED effect
 *
 * @param effect Effect type
 * @param color Base color for effect
 * @param speed Effect speed in milliseconds
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid parameters
 *      - ESP_FAIL Start effect failed
 */
esp_err_t bsp_led_start_effect(bsp_led_effect_t effect, uint32_t color, uint16_t speed);

/**
 * @brief Stop LED effect
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_FAIL Stop effect failed
 */
esp_err_t bsp_led_stop_effect();

/**
 * @brief Set RGB for LED strip (compatible with old interface)
 * @note This function sets the same color for all LEDs
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 *
 * @return
 *      - ESP_OK Success
 *      - ESP_FAIL Set color failed
 */
esp_err_t bsp_led_rgb_set(uint8_t r, uint8_t g, uint8_t b);


/**************************************************************************************************
 *
 * I2C Bus Management
 *
 **************************************************************************************************/

/**
 * @brief I2C configuration structure for BSP
 */

 #define BSP_I2C_CLK_SPEED     (100000)

 typedef struct {
     gpio_num_t sda_io_num;              /*!< I2C SDA GPIO number */
     gpio_num_t scl_io_num;              /*!< I2C SCL GPIO number */
     uint32_t clk_speed;                 /*!< I2C clock speed in Hz */
     bool enable_internal_pullup;       /*!< Enable internal pull-up resistors */
 } bsp_i2c_config_t;
 
 /**
  * @brief Initialize I2C bus
  * 
  * This function initializes the I2C master bus with the provided configuration.
  * The I2C bus can be shared by multiple devices including the touch IC.
  * 
  * @param config I2C configuration structure
  * @return
  *      - ESP_OK: I2C bus initialized successfully
  *      - ESP_ERR_INVALID_ARG: Invalid configuration parameters
  *      - ESP_ERR_NO_MEM: Memory allocation failed
  *      - ESP_FAIL: I2C bus initialization failed
  */
 esp_err_t bsp_i2c_init(const bsp_i2c_config_t *config);
 
 /**
  * @brief Get I2C bus handle
  * 
  * @return I2C master bus handle or NULL if not initialized
  */
 i2c_master_bus_handle_t bsp_i2c_get_bus_handle(void);
 
 /**
  * @brief Deinitialize I2C bus
  * 
  * @return
  *      - ESP_OK: I2C bus deinitialized successfully
  *      - ESP_FAIL: Deinitialization failed
  */
 esp_err_t bsp_i2c_deinit(void);


#if defined(ESP_HALE_BOARD_OPEN_SOURCE)
 /**************************************************************************************************
 *
 * Unified Button Management System
 *
 **************************************************************************************************/

/**
 * @brief Touch Button Layout (Circular arrangement)
 * 
 *          4 (Top Left)    3 (Top Right)
 *                     \   /
 *                      \ /
 *       5 (Left) -------o------- 2 (Right)
 *                      / \
 *                     /   \
 *          6 (Bottom Left) 1 (Bottom Right)
 * 
 * Physical button mapping:
 * - Button 1: Bottom Right position
 * - Button 2: Right position  
 * - Button 3: Top Right position
 * - Button 4: Top Left position
 * - Button 5: Left position
 * - Button 6: Bottom Left position
 */

typedef enum {
    BSP_INPUT_BUTTON_USER = 0,    
    BSP_INPUT_TOUCH_1,            /*!< Touch Button 1 - Bottom Right */
    BSP_INPUT_TOUCH_2,            /*!< Touch Button 2 - Right */
    BSP_INPUT_TOUCH_3,            /*!< Touch Button 3 - Top Right */
    BSP_INPUT_TOUCH_4,            /*!< Touch Button 4 - Top Left */
    BSP_INPUT_TOUCH_5,            /*!< Touch Button 5 - Left */
    BSP_INPUT_TOUCH_6,            /*!< Touch Button 6 - Bottom Left */                    
    BSP_INPUT_MAX
} bsp_button_source_t;

// Positional aliases for better code readability
#define BSP_INPUT_TOUCH_BOTTOM_RIGHT    BSP_INPUT_TOUCH_1  /*!< Touch Button Bottom Right */
#define BSP_INPUT_TOUCH_RIGHT           BSP_INPUT_TOUCH_2  /*!< Touch Button Right */
#define BSP_INPUT_TOUCH_TOP_RIGHT       BSP_INPUT_TOUCH_3  /*!< Touch Button Top Right */
#define BSP_INPUT_TOUCH_TOP_LEFT        BSP_INPUT_TOUCH_4  /*!< Touch Button Top Left */
#define BSP_INPUT_TOUCH_LEFT            BSP_INPUT_TOUCH_5  /*!< Touch Button Left */
#define BSP_INPUT_TOUCH_BOTTOM_LEFT     BSP_INPUT_TOUCH_6  /*!< Touch Button Bottom Left */

typedef enum {
    BSP_BUTTON_EVENT_PRESS_DOWN = 0,
    BSP_BUTTON_EVENT_PRESS_UP,
    BSP_BUTTON_EVENT_SHORT_PRESS,
    BSP_BUTTON_EVENT_LONG_PRESS,
    BSP_BUTTON_EVENT_DOUBLE_CLICK,
    BSP_BUTTON_EVENT_MAX
} bsp_button_event_t;

typedef void (*bsp_button_callback_t)(bsp_button_source_t source, bsp_button_event_t event, void *user_data);

/**
 * @brief Initialize unified button management system
 * 
 * This function provides a one-stop initialization for all button input sources:
 * - Automatically initializes I2C bus for touch IC communication
 * - Configures and registers all 6 touch buttons (BS8112A3)
 * - Sets up unified event processing for all button types
 * - Prepares the system for additional button sources (GPIO buttons, etc.)
 * 
 * @param callback Global button event callback function for all button sources
 * @return
 *      - ESP_OK: Button system initialized successfully
 *      - ESP_ERR_INVALID_ARG: Invalid callback parameter
 *      - ESP_FAIL: Initialization failed (check logs for details)
 * 
 * @note This function handles all the complexity of initializing multiple button
 *       subsystems. After calling this function, all touch buttons are ready to use.
 * 
 */
esp_err_t bsp_button_init(bsp_button_callback_t callback);

/**
 * @brief Register additional button source
 * 
 * Use this function to add custom button sources beyond the built-in touch buttons.
 * The built-in touch buttons (BSP_INPUT_TOUCH_1 to BSP_INPUT_TOUCH_6) are 
 * automatically registered by bsp_button_init().
 * 
 * @param source Button source type (should not conflict with built-in sources)
 * @param config Button configuration structure
 * @return
 *      - ESP_OK: Button source registered successfully
 *      - ESP_ERR_INVALID_ARG: Invalid arguments or source already registered
 *      - ESP_FAIL: Registration failed
 */
esp_err_t bsp_button_register_source(bsp_button_source_t source, const button_config_t *config);

/**
 * @brief Set callback for specific button source and event (Advanced)
 * 
 * This is an advanced function for fine-grained control. Most applications
 * should use the global callback set in bsp_button_init() instead.
 * 
 * @param source Button source type
 * @param event Button event type
 * @param cb Callback function
 * @param user_data User data passed to callback
 * @return
 *      - ESP_OK: Callback set successfully
 *      - ESP_ERR_INVALID_ARG: Invalid arguments
 *      - ESP_FAIL: Setting callback failed
 */
esp_err_t bsp_button_set_callback(bsp_button_source_t source, bsp_button_event_t event, bsp_button_callback_t cb, void *user_data);

/**
 * @brief Deinitialize unified button management system
 * 
 * @return
 *      - ESP_OK: Button system deinitialized successfully
 *      - ESP_FAIL: Deinitialization failed
 */
esp_err_t bsp_button_deinit(void);

/**
 * @brief Check if touch hardware is available and functional
 * 
 * This function can be used to determine if touch buttons are available
 * before attempting to use touch-specific features.
 * 
 * @return
 *      - true: Touch hardware is available and functional
 *      - false: Touch hardware is not available or failed initialization
 */
bool bsp_touch_hardware_available(void);
#endif

/**************************************************************************************************
 *
 * LCD interface
 *
 **************************************************************************************************/
#define BSP_LCD_PIXEL_CLOCK_HZ     (40 * 1000 * 1000)
#define BSP_LCD_SPI_NUM            (SPI2_HOST)

#define BSP_LCD_SPI_MOSI           BSP_LCD_PIN_NUM_MOSI
#define BSP_LCD_SPI_CLK            BSP_LCD_PIN_NUM_SCLK
#define BSP_LCD_SPI_CS             BSP_LCD_PIN_NUM_CS
#define BSP_LCD_DC                 BSP_LCD_PIN_NUM_DC
#define BSP_LCD_RST                (GPIO_NUM_NC)
#define BSP_LCD_BACKLIGHT          (GPIO_NUM_NC)

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
#define BSP_LCD_DRAW_BUFF_SIZE     (BSP_LCD_H_RES * 10)
#define BSP_LCD_DRAW_BUFF_DOUBLE   (0)

typedef struct {
    lvgl_port_cfg_t lvgl_port_cfg;
    uint32_t        buffer_size;
    bool            double_buffer;
    struct {
        unsigned int buff_dma: 1;
        unsigned int buff_spiram: 1;
    } flags;
} bsp_display_cfg_t;

lv_display_t *bsp_display_start(void);

lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

lv_indev_t *bsp_display_get_input_dev(void);

bool bsp_display_lock(uint32_t timeout_ms);

void bsp_display_unlock(void);

void bsp_display_rotate(lv_display_t *disp, lv_disp_rotation_t rotation);

lv_indev_t *bsp_display_indev_init(lv_display_t *disp);
#endif

#ifdef __cplusplus
}
#endif


