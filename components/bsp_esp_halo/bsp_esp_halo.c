#include <stdio.h>
#include <math.h>
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "bsp_err_check.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "adc_mic.h"
#include "driver/i2s_pdm.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "hal/rtc_io_hal.h"
#include "hal/gpio_ll.h"

#include "esp_codec_dev_defaults.h"

#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "led_strip_interface.h"

#include "touch_ic_bs8112a3.h"

#include "bsp_esp_halo.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "esp_lcd_ili9341.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_touch_cst816s.h"

#ifndef CONFIG_BSP_LCD_INTERFACE_SPI
#include "esp_lcd_panel_io_interface.h"
#endif

static const char *TAG = "ESP-HALE";

/* ==================== I2S ==================== */
static i2s_chan_handle_t tx_handle_ = NULL;


#define BSP_I2S_GPIO_CFG(_dout)       \
    {                          \
        .clk = GPIO_NUM_NC,    \
        .dout = _dout,  \
        .invert_flags = {      \
            .clk_inv = false, \
        },                     \
    }

/**
 * @brief Mono Duplex I2S configuration structure
 *
 * This configuration is used by default in bsp_audio_init()
 */
#define BSP_I2S_DUPLEX_MONO_CFG(_sample_rate, _dout)                                                         \
    {                                                                                                 \
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(_sample_rate),                                          \
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),        \
        .gpio_cfg = BSP_I2S_GPIO_CFG(_dout),                                                                 \
    }

#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

// I2C
static i2c_master_bus_handle_t s_i2c_bus_handle = NULL;

esp_codec_dev_handle_t bsp_audio_codec_microphone_init(void)
{
    audio_codec_adc_cfg_t cfg = DEFAULT_AUDIO_CODEC_ADC_MONO_CFG(AUDIO_ADC_MIC_CHANNEL, AUDIO_INPUT_SAMPLE_RATE);
    const audio_codec_data_if_t *adc_if = audio_codec_new_adc_data(&cfg);

    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .data_if = adc_if,
    };

    return esp_codec_dev_new(&codec_dev_cfg);
}

esp_codec_dev_handle_t bsp_audio_codec_speaker_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle_, NULL));

    i2s_pdm_tx_config_t pdm_cfg_default = BSP_I2S_DUPLEX_MONO_CFG(AUDIO_OUTPUT_SAMPLE_RATE, AUDIO_PDM_SPEAK_P_GPIO);
    pdm_cfg_default.clk_cfg.up_sample_fs = 480; // ;AUDIO_OUTPUT_SAMPLE_RATE / 100;
    pdm_cfg_default.slot_cfg.sd_scale = I2S_PDM_SIG_SCALING_MUL_4;
    pdm_cfg_default.slot_cfg.hp_scale = I2S_PDM_SIG_SCALING_MUL_4;
    pdm_cfg_default.slot_cfg.lp_scale = I2S_PDM_SIG_SCALING_MUL_4;
    pdm_cfg_default.slot_cfg.sinc_scale = I2S_PDM_SIG_SCALING_MUL_4;
    const i2s_pdm_tx_config_t *p_i2s_cfg = &pdm_cfg_default;

    ESP_ERROR_CHECK(i2s_channel_init_pdm_tx_mode(tx_handle_, p_i2s_cfg));

    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = I2S_NUM_0,
        .rx_handle = NULL,
        .tx_handle = tx_handle_,
    };

    const audio_codec_data_if_t *i2s_data_if = audio_codec_new_i2s_data(&i2s_cfg);

    i2s_channel_enable(tx_handle_);


    if(AUDIO_PA_CTL_GPIO != GPIO_NUM_NC) {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << AUDIO_PA_CTL_GPIO);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
    }
    gpio_set_drive_capability(AUDIO_PDM_SPEAK_P_GPIO, GPIO_DRIVE_CAP_0);

    if(AUDIO_PDM_SPEAK_N_GPIO != GPIO_NUM_NC){
        ESP_LOGI(TAG, "BSP_PDM_SPEAK_N_GPIO: %d", AUDIO_PDM_SPEAK_N_GPIO);
        PIN_FUNC_SELECT(IO_MUX_GPIO10_REG, PIN_FUNC_GPIO);
        gpio_set_direction(AUDIO_PDM_SPEAK_N_GPIO, GPIO_MODE_OUTPUT);
        esp_rom_gpio_connect_out_signal(AUDIO_PDM_SPEAK_N_GPIO,I2SO_SD_OUT_IDX,1,0);
        gpio_set_drive_capability(AUDIO_PDM_SPEAK_N_GPIO, GPIO_DRIVE_CAP_0);
    }


    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .data_if = i2s_data_if,
        .codec_if = NULL,
    };
    return esp_codec_dev_new(&codec_dev_cfg);
}


// LED strip config
static const led_strip_config_t bsp_strip_config = {
    .strip_gpio_num = BSP_RGB_CTRL,
    .max_leds = BSP_LED_STRIP_COUNT,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB,
    .led_model = LED_MODEL_WS2812,
    .flags.invert_out = false,
};

static const led_strip_rmt_config_t bsp_rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
    .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
    .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
};

// LED
static led_strip_handle_t led_strip = NULL;
static bsp_led_config_t led_configs[BSP_LED_STRIP_COUNT];
static esp_timer_handle_t led_effect_timer = NULL;
static bsp_led_effect_t current_effect = BSP_LED_EFFECT_STATIC;
static bool effect_running = false;
static uint8_t effect_step = 0;

/* LED */
esp_err_t bsp_led_init()
{
    ESP_LOGI(TAG, "Initializing LED strip with %d LEDs on GPIO %d", 
             BSP_LED_STRIP_COUNT, bsp_strip_config.strip_gpio_num);

    esp_err_t ret = led_strip_new_rmt_device(&bsp_strip_config, &bsp_rmt_config, &led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return ret;
    }

    for (int i = 0; i < BSP_LED_STRIP_COUNT; i++) {
        led_configs[i].color = BSP_LED_COLOR_OFF;
        led_configs[i].brightness = BSP_LED_BRIGHTNESS_2;
        led_configs[i].effect = BSP_LED_EFFECT_STATIC;
        led_configs[i].effect_speed = 1000;
    }

    ret = led_strip_clear(led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear LED strip: %s", esp_err_to_name(ret));
        led_strip_del(led_strip);
        return ret;
    }

    ESP_LOGI(TAG, "LED strip initialized successfully");
    return ESP_OK;
}

esp_err_t bsp_led_deinit()
{
    if (led_effect_timer) {
        esp_timer_stop(led_effect_timer);
        esp_timer_delete(led_effect_timer);
        led_effect_timer = NULL;
    }
    
    if (led_strip) {
        esp_err_t ret = led_strip_del(led_strip);
        led_strip = NULL;
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t bsp_led_set_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (!led_strip) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ESP_OK;

    if (index == BSP_LED_ALL_INDEX) {
        for (int i = 0; i < BSP_LED_STRIP_COUNT; i++) {
            ret |= led_strip_set_pixel(led_strip, i, r, g, b);
            led_configs[i].color = (r << 16) | (g << 8) | b;
        }
    } else if (index < BSP_LED_STRIP_COUNT) {
        ret = led_strip_set_pixel(led_strip, index, r, g, b);
        led_configs[index].color = (r << 16) | (g << 8) | b;
    } else {
        return ESP_ERR_INVALID_ARG;
    }

    if (ret == ESP_OK) {
        ret = led_strip_refresh(led_strip);
    }

    return ret;
}

esp_err_t bsp_led_set_color(uint8_t index, uint32_t color)
{
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    return bsp_led_set_rgb(index, r, g, b);
}

esp_err_t bsp_led_set_hsv(uint8_t index, uint16_t hue, uint8_t saturation, uint8_t value)
{
    if (!led_strip) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ESP_OK;

    if (index == BSP_LED_ALL_INDEX) {
        for (int i = 0; i < BSP_LED_STRIP_COUNT; i++) {
            ret |= led_strip_set_pixel_hsv(led_strip, i, hue, saturation, value);
        }
    } else if (index < BSP_LED_STRIP_COUNT) {
        ret = led_strip_set_pixel_hsv(led_strip, index, hue, saturation, value);
    } else {
        return ESP_ERR_INVALID_ARG;
    }

    if (ret == ESP_OK) {
        ret = led_strip_refresh(led_strip);
    }

    return ret;
}

esp_err_t bsp_led_set_colors(const uint32_t *colors)
{
    if (!led_strip || !colors) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;
    
    for (int i = 0; i < BSP_LED_STRIP_COUNT; i++) {
        uint8_t r = (colors[i] >> 16) & 0xFF;
        uint8_t g = (colors[i] >> 8) & 0xFF;
        uint8_t b = colors[i] & 0xFF;
        
        ret |= led_strip_set_pixel(led_strip, i, r, g, b);
        led_configs[i].color = colors[i];
    }

    if (ret == ESP_OK) {
        ret = led_strip_refresh(led_strip);
    }

    return ret;
}

esp_err_t bsp_led_set_all_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return bsp_led_set_rgb(BSP_LED_ALL_INDEX, r, g, b);
}

esp_err_t bsp_led_clear(uint8_t index)
{
    return bsp_led_set_rgb(index, 0, 0, 0);
}

esp_err_t bsp_led_clear_all()
{
    if (!led_strip) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return led_strip_clear(led_strip);
}

esp_err_t bsp_led_set_brightness(uint8_t index, uint8_t brightness)
{
    if (!led_strip) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = ESP_OK;
    
    if (index == BSP_LED_ALL_INDEX) {
        // Set brightness for all LEDs
        for (int i = 0; i < BSP_LED_STRIP_COUNT; i++) {
            led_configs[i].brightness = brightness;
            
            // Apply brightness to current color
            uint32_t color = led_configs[i].color;
            uint8_t r = ((color >> 16) & 0xFF) * brightness / 255;
            uint8_t g = ((color >> 8) & 0xFF) * brightness / 255;
            uint8_t b = (color & 0xFF) * brightness / 255;
            
            ret |= led_strip_set_pixel(led_strip, i, r, g, b);
        }
    } else if (index < BSP_LED_STRIP_COUNT) {
        // Set brightness for specific LED
        led_configs[index].brightness = brightness;
        
        // Apply brightness to current color
        uint32_t color = led_configs[index].color;
        uint8_t r = ((color >> 16) & 0xFF) * brightness / 255;
        uint8_t g = ((color >> 8) & 0xFF) * brightness / 255;
        uint8_t b = (color & 0xFF) * brightness / 255;
        
        ret = led_strip_set_pixel(led_strip, index, r, g, b);
    } else {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (ret == ESP_OK) {
        ret = led_strip_refresh(led_strip);
    }
    
    return ret;
}

static void led_effect_timer_callback(void *arg)
{
    if (!effect_running || !led_strip) {
        return;
    }

    switch (current_effect) {
        case BSP_LED_EFFECT_BREATHING: {
            float brightness = (sin(effect_step * 0.1) + 1.0) / 2.0;
            uint8_t level = (uint8_t)(brightness * 255);
            bsp_led_set_all_rgb(level, level, level);
            break;
        }
        
        case BSP_LED_EFFECT_RAINBOW: {
            for (int i = 0; i < BSP_LED_STRIP_COUNT; i++) {
                uint16_t hue = (effect_step * 10 + i * 60) % 360;
                led_strip_set_pixel_hsv(led_strip, i, hue, 255, 128);
            }
            led_strip_refresh(led_strip);
            break;
        }
        
        case BSP_LED_EFFECT_CHASE: {
            bsp_led_clear_all();
            uint8_t pos = effect_step % BSP_LED_STRIP_COUNT;
            bsp_led_set_rgb(pos, 255, 255, 255);
            break;
        }
        
        default:
            break;
    }
    
    effect_step++;
}

esp_err_t bsp_led_start_effect(bsp_led_effect_t effect, uint32_t color, uint16_t speed)
{
    if (effect >= BSP_LED_EFFECT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    bsp_led_stop_effect();

    current_effect = effect;
    effect_step = 0;
    effect_running = true;

    esp_timer_create_args_t timer_args = {
        .callback = led_effect_timer_callback,
        .arg = NULL,
        .name = "led_effect_timer"
    };
    
    esp_err_t ret = esp_timer_create(&timer_args, &led_effect_timer);
    if (ret != ESP_OK) {
        return ret;
    }

    return esp_timer_start_periodic(led_effect_timer, speed * 1000);
}

esp_err_t bsp_led_stop_effect()
{
    effect_running = false;
    
    if (led_effect_timer) {
        esp_timer_stop(led_effect_timer);
        esp_timer_delete(led_effect_timer);
        led_effect_timer = NULL;
    }
    
    return ESP_OK;
}

esp_err_t bsp_led_rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
    return bsp_led_set_all_rgb(r, g, b);
}

/* I2C Bus Management */
esp_err_t bsp_i2c_init(const bsp_i2c_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Invalid I2C configuration");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_i2c_bus_handle != NULL) {
        ESP_LOGW(TAG, "I2C bus already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing I2C bus on SDA=%d, SCL=%d", config->sda_io_num, config->scl_io_num);
    
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = config->sda_io_num,
        .scl_io_num = config->scl_io_num,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .intr_priority = 0,
    };
    
    esp_err_t ret = i2c_new_master_bus(&bus_config, &s_i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "I2C bus initialized successfully");

    /* Debug probe: verify BS8112A3 is responding on the bus before continuing */
    esp_err_t probe_ret = i2c_master_probe(s_i2c_bus_handle, BS8112A3_I2C_ADDR, 1000);
    if (probe_ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C probe 0x%02X success", BS8112A3_I2C_ADDR);
    } else {
        ESP_LOGE(TAG, "I2C probe 0x%02X failed: %s", BS8112A3_I2C_ADDR, esp_err_to_name(probe_ret));
    }

    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_get_bus_handle(void)
{
    return s_i2c_bus_handle;
}

esp_err_t bsp_i2c_deinit(void)
{
    if (s_i2c_bus_handle) {
        ESP_LOGI(TAG, "Deinitializing I2C bus");
        esp_err_t ret = i2c_del_master_bus(s_i2c_bus_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to deinitialize I2C bus: %s", esp_err_to_name(ret));
        }
        s_i2c_bus_handle = NULL;
        return ret;
    }
    return ESP_OK;
}


#if defined(ESP_HALE_BOARD_OPEN_SOURCE)

// Button
static button_handle_t bsp_button_handles[BSP_INPUT_MAX] = {NULL};
static bsp_button_callback_t global_button_callback = NULL;
static bool touch_hardware_available = false;  // Track touch hardware availability

/* Touch Buttons (BS8112A3) - Internal Functions */
/**
 * @brief Custom button initialization callback for individual touch buttons
 */
static esp_err_t bsp_touch_button_custom_init(void *param)
{
    uint32_t bit_position = (uint32_t)param;
    
    if (bit_position < 2 || bit_position > 7) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!touch_ic_bs8112a3_is_initialized()) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return ESP_OK;
}

/**
 * @brief Custom button deinitialization callback for individual touch buttons
 */
static esp_err_t bsp_touch_button_custom_deinit(void *param)
{
    uint32_t bit_position = (uint32_t)param;
    
    if (bit_position < 2 || bit_position > 7) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

/**
 * @brief Check if touch hardware is available and functional
 */
bool bsp_touch_hardware_available(void)
{
    return touch_hardware_available;
}

/**
 * @brief Custom button get key value function for touch buttons
 */
static uint8_t bsp_touch_custom_get_key_value(void *param)
{
    uint32_t bit_position = (uint32_t)param;
    
    if (!touch_hardware_available || bit_position < 2 || bit_position > 7) {
        return 0;
    }
    
    uint32_t touch_index = bit_position - 2;
    if (touch_index >= TOUCH_BUTTON_NUM) {
        return 0;
    }
    
    return touch_ic_bs8112a3_get_key_value(touch_index) ? 1 : 0;
}

/**
 * @brief Internal function to initialize touch buttons
 * 
 * Touch Button Physical Layout:
 *          4 (Top Left)    3 (Top Right)
 *                     \   /
 *                      \ /
 *       5 (Left) -------o------- 2 (Right)
 *                      / \
 *                     /   \
 *          6 (Bottom Left) 1 (Bottom Right)
 * 
 * LED-Touch Button Mapping:
 * - Touch Button 1 (Bottom Right) <-> LED 0 (Bottom Right)
 * - Touch Button 2 (Right)        <-> LED 1 (Right)
 * - Touch Button 3 (Top Right)    <-> LED 2 (Top Right)
 * - Touch Button 4 (Top Left)     <-> LED 3 (Top Left)
 * - Touch Button 5 (Left)         <-> LED 4 (Left)
 * - Touch Button 6 (Bottom Left)  <-> LED 5 (Bottom Left)
 */
static esp_err_t bsp_touch_button_init_internal(void)
{
    if (!s_i2c_bus_handle) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing touch buttons with interrupt mode");
    
    // Initialize touch IC with interrupt mode (using BSP_TOUCH_INT pin)
    // GPIO ISR service installation is now handled inside touch_ic_bs8112a3_init()
    touch_ic_bs8112a3_config_t touch_config = {
        .i2c_bus_handle = s_i2c_bus_handle,
        .device_address = BS8112A3_I2C_ADDR,
        .scl_speed_hz = BSP_I2C_CLK_SPEED,
        .interrupt_pin = GPIO_NUM_NC,  // Enable interrupt mode
    };
    
    esp_err_t ret = touch_ic_bs8112a3_init(&touch_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize touch IC");
        return ret;
    }
    
    // Register each touch button with bit position mapping
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_CUSTOM,
        .long_press_time = 1000,
        .short_press_time = 180,
        .custom_button_config = {
            .button_custom_init = bsp_touch_button_custom_init,
            .button_custom_get_key_value = bsp_touch_custom_get_key_value,
            .button_custom_deinit = bsp_touch_button_custom_deinit,
            .active_level = 1,
            .priv = NULL,
        },
    };
    
    uint8_t success_count = 0;
    
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        btn_cfg.custom_button_config.priv = (void *)(i + 2);
        bsp_button_source_t source = BSP_INPUT_TOUCH_1 + i;
        
        ret = bsp_button_register_source(source, &btn_cfg);
        if (ret == ESP_OK) {
            success_count++;
        }
    }
    
    if (success_count == 0) {
        ESP_LOGE(TAG, "No touch buttons registered successfully");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Touch buttons initialized: %d/%d functional (interrupt mode)", success_count, TOUCH_BUTTON_NUM);
    return ESP_OK;
}

/**
 * @brief Internal function to deinitialize touch buttons
 */
static esp_err_t bsp_touch_button_deinit_internal(void)
{
    // Unregister touch buttons
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        bsp_button_source_t source = BSP_INPUT_TOUCH_1 + i;
        if (bsp_button_handles[source] != NULL) {
            iot_button_delete(bsp_button_handles[source]);
            bsp_button_handles[source] = NULL;
        }
    }
    
    // Deinitialize touch IC
    esp_err_t ret = touch_ic_bs8112a3_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize touch IC");
    }
    
    return ret;
}

/* Button Management */
static void bsp_button_event_handler(void *button_handle, void *usr_data)
{
    bsp_button_source_t source = (bsp_button_source_t)usr_data;
    button_event_t btn_event = iot_button_get_event(button_handle);
    
    bsp_button_event_t bsp_event;
    switch (btn_event) {
        case BUTTON_PRESS_DOWN:      bsp_event = BSP_BUTTON_EVENT_PRESS_DOWN; break;
        case BUTTON_PRESS_UP:        bsp_event = BSP_BUTTON_EVENT_PRESS_UP; break;
        case BUTTON_SINGLE_CLICK:    bsp_event = BSP_BUTTON_EVENT_SHORT_PRESS; break;
        case BUTTON_LONG_PRESS_START: bsp_event = BSP_BUTTON_EVENT_LONG_PRESS; break;
        case BUTTON_DOUBLE_CLICK:    bsp_event = BSP_BUTTON_EVENT_DOUBLE_CLICK; break;
        default: return;
    }
    
    if (global_button_callback) {
        global_button_callback(source, bsp_event, usr_data);
    }
}

esp_err_t bsp_button_init(bsp_button_callback_t callback)
{
    if (!callback) {
        return ESP_ERR_INVALID_ARG;
    }
    
    global_button_callback = callback;
    ESP_LOGI(TAG, "Initializing button management system");
    
    // Initialize I2C bus if not already initialized
    if (s_i2c_bus_handle == NULL) {
        bsp_i2c_config_t i2c_config = {
            .sda_io_num = BSP_I2C_SDA,
            .scl_io_num = BSP_I2C_SCL,
            .clk_speed = BSP_I2C_CLK_SPEED,
        };
        
        esp_err_t ret = bsp_i2c_init(&i2c_config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2C bus");
            return ret;
        }
    }
    
    // Try to initialize touch buttons
    esp_err_t ret = bsp_touch_button_init_internal();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Touch hardware not available, continuing without touch");
        touch_hardware_available = false;
    } else {
        touch_hardware_available = true;
        ESP_LOGI(TAG, "Touch hardware initialized successfully");
    }
    
    ESP_LOGI(TAG, "Button management system initialized (touch: %s)", 
             touch_hardware_available ? "enabled" : "disabled");
    return ESP_OK;
}

esp_err_t bsp_button_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing button management system");
    
    // Deinitialize touch buttons
    esp_err_t ret = bsp_touch_button_deinit_internal();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize touch buttons");
    }
    
    // Clean up remaining button handles
    for (int i = 0; i < BSP_INPUT_MAX; i++) {
        if (bsp_button_handles[i] != NULL) {
            iot_button_delete(bsp_button_handles[i]);
            bsp_button_handles[i] = NULL;
        }
    }
    
    // Deinitialize I2C bus
    esp_err_t i2c_ret = bsp_i2c_deinit();
    if (i2c_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize I2C bus");
        if (ret == ESP_OK) {
            ret = i2c_ret;
        }
    }
    
    global_button_callback = NULL;
    touch_hardware_available = false;
    return ret;
}

esp_err_t bsp_button_register_source(bsp_button_source_t source, const button_config_t *config)
{
    if (source >= BSP_INPUT_MAX || bsp_button_handles[source] != NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    bsp_button_handles[source] = iot_button_create(config);
    if (bsp_button_handles[source] == NULL) {
        return ESP_FAIL;
    }
    
    iot_button_register_cb(bsp_button_handles[source], BUTTON_PRESS_DOWN, bsp_button_event_handler, (void*)source);
    iot_button_register_cb(bsp_button_handles[source], BUTTON_PRESS_UP, bsp_button_event_handler, (void*)source);
    iot_button_register_cb(bsp_button_handles[source], BUTTON_SINGLE_CLICK, bsp_button_event_handler, (void*)source);
    iot_button_register_cb(bsp_button_handles[source], BUTTON_LONG_PRESS_START, bsp_button_event_handler, (void*)source);
    iot_button_register_cb(bsp_button_handles[source], BUTTON_DOUBLE_CLICK, bsp_button_event_handler, (void*)source);
    
    return ESP_OK;
}
#endif

#define LCD_CMD_BITS         (8)
#define LCD_PARAM_BITS       (8)
#define LCD_LEDC_CH          (CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH)
#define LVGL_TICK_PERIOD_MS  (CONFIG_BSP_DISPLAY_LVGL_TICK)
#define LVGL_MAX_SLEEP_MS    (CONFIG_BSP_DISPLAY_LVGL_MAX_SLEEP)

static const ili9341_lcd_init_cmd_t vendor_specific_init[] = {
    {0x11, NULL, 0, 120},                                                                         // Sleep Out
    {0x36, (uint8_t []){0x00}, 1, 0},                                                             // Memory Data Access Control
    {0x3A, (uint8_t []){0x05}, 1, 0},                                                             // Interface Pixel Format (16-bit)
    {0xB2, (uint8_t []){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5, 0},                                     // Porch Setting
    {0xB7, (uint8_t []){0x05}, 1, 0},                                                             // Gate Control
    {0xBB, (uint8_t []){0x21}, 1, 0},                                                             // VCOM Setting
    {0xC0, (uint8_t []){0x2C}, 1, 0},                                                             // LCM Control
    {0xC2, (uint8_t []){0x01}, 1, 0},                                                             // VDV and VRH Command Enable
    {0xC3, (uint8_t []){0x15}, 1, 0},                                                             // VRH Set
    {0xC6, (uint8_t []){0x0F}, 1, 0},                                                             // Frame Rate Control
    {0xD0, (uint8_t []){0xA7}, 1, 0},                                                             // Power Control 1
    {0xD0, (uint8_t []){0xA4, 0xA1}, 2, 0},                                                       // Power Control 1
    {0xD6, (uint8_t []){0xA1}, 1, 0},                                                             // Gate output GND in sleep mode
    {0xE0, (uint8_t []){0xF0, 0x05, 0x0E, 0x08, 0x0A, 0x17, 0x39, 0x54, 
                        0x4E, 0x37, 0x12, 0x12, 0x31, 0x37}, 14, 0},                              // Positive Gamma Control
    {0xE1, (uint8_t []){0xF0, 0x10, 0x14, 0x0D, 0x0B, 0x05, 0x39, 0x44, 
                        0x4D, 0x38, 0x14, 0x14, 0x2E, 0x35}, 14, 0},                              // Negative Gamma Control
    {0xE4, (uint8_t []){0x23, 0x00, 0x00}, 3, 0},                                                 // Gate position control
    {0x21, NULL, 0, 0},                                                                           // Display Inversion On
    {0x29, NULL, 0, 0},                                                                           // Display On
    {0x2C, NULL, 0, 0},                                                                           // Memory Write
};

esp_err_t bsp_display_brightness_init(void)
{
    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    return ESP_OK;
}

esp_err_t bsp_display_backlight_off(void)
{
    return bsp_display_brightness_set(0);
}

esp_err_t bsp_display_backlight_on(void)
{
    return bsp_display_brightness_set(100);
}

esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io)
{
    esp_err_t ret = ESP_OK;
    assert(config != NULL && config->max_transfer_sz > 0);

    ESP_LOGD(TAG, "Initialize display");
    esp_lcd_panel_io_handle_t io_handle = NULL;

#ifdef CONFIG_BSP_LCD_INTERFACE_SPI
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_PIN_NUM_SCLK,
        .mosi_io_num = BSP_LCD_PIN_NUM_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = config->max_transfer_sz,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_PIN_NUM_DC,
        .cs_gpio_num = BSP_LCD_PIN_NUM_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 3,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, &io_handle), err, TAG, "New panel IO failed");
#else
    ESP_LOGD(TAG, "Initialize Parlio device");
    const esp_lcd_panel_io_parl_config_t io_config = {
        .clk_src = PARLIO_CLK_SRC_DEFAULT,
        .dc_gpio_num = BSP_LCD_PIN_NUM_DC,
        .clk_gpio_num = BSP_LCD_PIN_NUM_SCLK,
        .data_gpio_nums = {
            BSP_LCD_PIN_NUM_MOSI,
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 2
            GPIO_NUM_NC,
#endif
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 3
            GPIO_NUM_NC,
#endif
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 4
            GPIO_NUM_NC,
#endif
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 5
            GPIO_NUM_NC,
#endif
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 6
            GPIO_NUM_NC,
#endif
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 7
            GPIO_NUM_NC,
#endif
#if CONFIG_BSP_LCD_PARLIO_DATA_WIDTH >= 8
            GPIO_NUM_NC,
#endif
        },
        .data_width = CONFIG_BSP_LCD_PARLIO_DATA_WIDTH,
        .max_transfer_bytes = config->max_transfer_sz,
        .dma_burst_size = 32,
        .cs_gpio_num = BSP_LCD_PIN_NUM_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_cmd_level = 0,
            .dc_data_level = 1,
        },
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_parl(&io_config, &io_handle), err, TAG, "New panel IO failed");
#endif

    ESP_LOGD(TAG, "Install LCD driver");
    ili9341_vendor_config_t vendor_config = {
        .init_cmds = vendor_specific_init,
        .init_cmds_size = sizeof(vendor_specific_init) / sizeof(ili9341_lcd_init_cmd_t),
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_NUM_NC,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_config,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_ili9341(io_handle, &panel_config, ret_panel), err, TAG, "New panel failed");

    esp_lcd_panel_reset(*ret_panel);
    esp_lcd_panel_init(*ret_panel);

    esp_lcd_panel_swap_xy(*ret_panel, true);
    esp_lcd_panel_mirror(*ret_panel, false, true);
    esp_lcd_panel_set_gap(*ret_panel, 0, 0);

    *ret_io = io_handle;
    return ret;

err:
    if (*ret_panel) {
        esp_lcd_panel_del(*ret_panel);
    }
    if (io_handle) {
        esp_lcd_panel_io_del(io_handle);
    }
#ifdef CONFIG_BSP_LCD_INTERFACE_SPI
    spi_bus_free(BSP_LCD_SPI_NUM);
#endif
    return ret;
}

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static esp_lcd_touch_handle_t tp_handle = NULL;

static lv_display_t *bsp_display_lcd_init(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_handle_t panel_handle = NULL;
    const bsp_display_config_t bsp_disp_cfg = {
        .max_transfer_sz = BSP_LCD_DRAW_BUFF_SIZE * sizeof(uint16_t),
    };
    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(&bsp_disp_cfg, &panel_handle, &io_handle));

    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = cfg->buffer_size,
        .double_buffer = cfg->double_buffer,
        .hres = BSP_LCD_H_RES,
        .vres = BSP_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = cfg->flags.buff_dma,
            .buff_spiram = cfg->flags.buff_spiram,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = (BSP_LCD_BIGENDIAN ? true : false),
#endif
        }
    };

    return lvgl_port_add_disp(&disp_cfg);
}

static esp_err_t bsp_touch_init(void)
{
    if (s_i2c_bus_handle == NULL) {
        bsp_i2c_config_t i2c_config = {
            .sda_io_num = BSP_I2C_SDA,
            .scl_io_num = BSP_I2C_SCL,
            .clk_speed = BSP_I2C_CLK_SPEED,
        };
        ESP_RETURN_ON_ERROR(bsp_i2c_init(&i2c_config), TAG, "I2C init failed");
    }

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(s_i2c_bus_handle, &tp_io_config, &tp_io_handle), TAG, "");

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,
        .y_max = BSP_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp_handle), TAG, "");
    return ESP_OK;
}

lv_indev_t *bsp_display_indev_init(lv_display_t *disp)
{
    assert(disp != NULL);

    if (bsp_touch_init() != ESP_OK) {
        return NULL;
    }

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp_handle,
    };

    return lvgl_port_add_touch(&touch_cfg);
}

lv_display_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = {
            .task_priority = CONFIG_BSP_DISPLAY_LVGL_TASK_PRIORITY,
            .task_stack = 8*1024,
            .task_affinity = 0,
            .timer_period_ms = LVGL_TICK_PERIOD_MS,
            .task_max_sleep_ms = LVGL_MAX_SLEEP_MS,
        },
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
#if (CONFIG_BSP_DISPLAY_LVGL_BUFFER_IN_PSRAM && CONFIG_SPIRAM)
            .buff_dma = false,
            .buff_spiram = true,
#else
            .buff_dma = true,
            .buff_spiram = false,
#endif
        }
    };
    return bsp_display_start_with_config(&cfg);
}

lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    lv_display_t *disp;
    assert(cfg != NULL);
    BSP_ERROR_CHECK_RETURN_NULL(lvgl_port_init(&cfg->lvgl_port_cfg));
    BSP_NULL_CHECK(disp = bsp_display_lcd_init(cfg), NULL);

    bsp_display_indev_init(disp);

    return disp;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return NULL;
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void bsp_display_unlock(void)
{
    lvgl_port_unlock();
}

void bsp_display_rotate(lv_display_t *disp, lv_disp_rotation_t rotation)
{
    lv_disp_set_rotation(disp, rotation);
}
#endif
