// C / FreeRTOS (FreeRTOS.h must be before task.h)
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP / Drivers
#include "bmi2_defs.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "bmi270_circle.h"
#include "common/common.h"
// Wi‑Fi / ESP‑NOW
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_netif.h"
#include "esp_event.h"

/**
 * @file main.c
 * @brief BMI270 Triple Tap Detection Example for ESP32
 *
 * This example demonstrates how to use the BMI270 Circle sensor for triple tap detection
 * on ESP32 platforms. It shows how to initialize the BMI270 sensor, configure it for
 * triple tap detection, and handle interrupts using ESP32 GPIO.
 *
 * Key Features:
 * - Triple tap detection using BMI270 Circle firmware
 * - GPIO interrupt handling for efficient event detection
 * - Support for multiple ESP32 development boards
 * - Configurable tap sensitivity and timing parameters
 * - 3-second cooling period to prevent false triggers
 *
 * Hardware Requirements:
 * - ESP32 development board (ESP32-S3, ESP32-C5, etc.)
 * - BMI270 Circle sensor
 * - I2C connection between ESP32 and BMI270
 *
 * @author Bosch Sensortec GmbH
 * @date 2025
 * @version 1.0
 */

// Configure GPIO pins based on selected development board
#ifdef CONFIG_BOARD_ESP_SPOT_C5
#define I2C_INT_IO              3      // Interrupt pin for ESP-SPOT-C5
#define I2C_MASTER_SCL_IO       26     // I2C SCL pin for ESP-SPOT-C5
#define I2C_MASTER_SDA_IO       25     // I2C SDA pin for ESP-SPOT-C5
#define UE_SW_I2C               1      // Use software I2C for ESP-SPOT-C5
#elif defined(CONFIG_BOARD_ESP_SPOT_S3)
#define I2C_INT_IO              5      // Interrupt pin for ESP-SPOT-S3
#define I2C_MASTER_SCL_IO       1      // I2C SCL pin for ESP-SPOT-S3
#define I2C_MASTER_SDA_IO       2      // I2C SDA pin for ESP-SPOT-S3
#define UE_SW_I2C               1      // Use software I2C for ESP-SPOT-S3
#elif defined(CONFIG_BOARD_ESP_ASTOM_S3)
#define I2C_INT_IO              16     // Interrupt pin for ESP-ASTOM-S3
#define I2C_MASTER_SCL_IO       0      // I2C SCL pin for ESP-ASTOM-S3
#define I2C_MASTER_SDA_IO       45     // I2C SDA pin for ESP-ASTOM-S3
#define UE_SW_I2C               1      // Use software I2C for ESP-ASTOM-S3
#elif defined(CONFIG_BOARD_ESP_ECHOEAR_S3)
#define I2C_INT_IO              21     // Interrupt pin for ESP-ECHOEAR-S3
#define I2C_MASTER_SCL_IO       1      // I2C SCL pin for ESP-ECHOEAR-S3
#define I2C_MASTER_SDA_IO       2      // I2C SDA pin for ESP-ECHOEAR-S3
#define UE_SW_I2C               0      // Use hardware I2C for ESP-ECHOEAR-S3
#else
// Custom board configuration - use Kconfig values
#define I2C_INT_IO              CONFIG_I2C_INT_IO
#define I2C_MASTER_SCL_IO       CONFIG_I2C_MASTER_SCL_IO
#define I2C_MASTER_SDA_IO       CONFIG_I2C_MASTER_SDA_IO
#define UE_SW_I2C               CONFIG_UE_SW_I2C
#endif

// I2C configuration constants
#define I2C_MASTER_NUM          I2C_NUM_0      // I2C port number
#define I2C_MASTER_FREQ_HZ      (100 * 1000)   // I2C frequency: 100kHz

// Global handles for BMI270 and I2C bus
static bmi270_circle_handle_t bmi_handle = NULL;    // BMI270 sensor handle
static i2c_bus_handle_t i2c_bus;                    // I2C bus handle
static volatile bool interrupt_status = false;       // Flag set by ISR when interrupt occurs

// ESPNOW channel must match receiver (led_strip example uses channel 1)
#define EXAMPLE_ESPNOW_CHANNEL      1

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} example_rgb_msg_t;

static const uint8_t s_broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t s_color_mode = 0; // 0..4: pink, orange, green, cyan, off

static void example_init_wifi_and_espnow(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(EXAMPLE_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_now_init());

    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, s_broadcast_mac, 6);
    peer.channel = EXAMPLE_ESPNOW_CHANNEL;
    peer.ifidx = ESP_IF_WIFI_STA;
    peer.encrypt = false;
    (void)esp_now_del_peer(peer.peer_addr); // ensure clean state
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
}

static void example_send_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    example_rgb_msg_t msg = { .r = r, .g = g, .b = b };
    esp_err_t err = esp_now_send(s_broadcast_mac, (const uint8_t *)&msg, sizeof(msg));
    if (err != ESP_OK) {
        ESP_LOGE("MAIN", "ESP-NOW send failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI("MAIN", "Sent RGB: %u,%u,%u", r, g, b);
    }
}

static void example_send_color_by_mode(uint8_t mode)
{
    switch (mode) {
        case 0: // pink
            example_send_rgb(255, 20, 147);
            break;
        case 1: // orange
            example_send_rgb(255, 165, 0);
            break;
        case 2: // green
            example_send_rgb(32, 255, 0);
            break;
        case 3: // cyan
            example_send_rgb(0, 255, 255);
            break;
        case 4: // off
        default:
            example_send_rgb(0, 0, 0);
            break;
    }
}

/**
 * @brief GPIO interrupt service routine for BMI270 INT pin
 *
 * This ISR is called when an interrupt is detected on the configured GPIO pin.
 * It sets the interrupt_status flag to notify the main loop that an interrupt
 * has occurred. The ISR should be as short as possible to avoid blocking
 * other interrupts.
 *
 * @param arg GPIO number (cast from void*)
 */
static void IRAM_ATTR gpio_isr_edge_handler(void* arg)
{
    (void)arg;
    interrupt_status = true;
}

/**
 * @brief Initialize I2C bus and BMI270 sensor
 *
 * This function configures the I2C bus and creates the BMI270 sensor handle.
 * It sets up the I2C communication interface and initializes the BMI270 Circle
 * sensor with the specified I2C address.
 *
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
static esp_err_t i2c_sensor_bmi270_init(void)
{
    // I2C bus configuration
    const i2c_config_t i2c_bus_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    // Create I2C bus (software or hardware based on board)
#if UE_SW_I2C
    i2c_bus = i2c_bus_create(I2C_NUM_SW_1, &i2c_bus_conf);
#else
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &i2c_bus_conf);
#endif

    if (!i2c_bus) {
        ESP_LOGE("MAIN", "I2C bus create failed");
        return ESP_FAIL;
    }

    // BMI270 sensor configuration
    bmi270_circle_i2c_config_t i2c_bmi270_conf = {
        .i2c_handle = i2c_bus,
        .i2c_addr = BMI270_I2C_ADDRESS,
    };

    // Create BMI270 sensor handle
    if (bmi270_circle_sensor_create(&i2c_bmi270_conf, &bmi_handle) != ESP_OK || bmi_handle == NULL) {
        ESP_LOGE("MAIN", "BMI270_CIRCLE create failed");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

/**
 * @brief Configure BMI270 for tap detection and set INT1 pin
 *
 * This function sets up the sensor configuration for tap detection and configures
 * the INT1 pin for interrupt output. It configures the accelerometer and tap
 * detection parameters according to the BMI270 Circle firmware specifications.
 *
 * @param bmi2_dev Pointer to BMI2 device structure
 * @return Result code from BMI2 API
 */
static int8_t set_feature_config(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    struct bmi2_sens_config config[2] = { {0} };
    struct bmi2_int_pin_config pin_config = { 0 };

    /* Configure the type of tap detection feature. */
    config[0].type = BMI2_TAP;      // Tap detection feature
    config[1].type = BMI2_ACCEL;    // Accelerometer feature

    /* Get default configurations for the type of feature selected. */
    rslt = bmi270_circle_get_sensor_config(config, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    // Configure INT1 pin for interrupt output
    rslt = bmi2_get_int_pin_config(&pin_config, bmi2_dev);
    bmi2_error_codes_print_result(rslt);
    if (rslt == BMI2_OK) {
        pin_config.pin_type = BMI2_INT1;
        pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
        pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;    // Low-level interrupt
        pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
        pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
        pin_config.int_latch = BMI2_INT_NON_LATCH;
        rslt = bmi2_set_int_pin_config(&pin_config, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
    }

    if (rslt == BMI2_OK)
    {
        /* User can change the following configuration parameters as required. */
        config[1].cfg.acc.odr = BMI2_ACC_ODR_50HZ; // Set accelerometer ODR to 50Hz

        /* Set new configurations. */
        rslt = bmi270_circle_set_sensor_config(config, 2, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
        
        if (rslt == BMI2_OK) {
            ESP_LOGI("MAIN", "Basic configuration set successfully");
        } else {
            ESP_LOGE("MAIN", "Failed to set basic configuration");
        }
    }

    return rslt;
}

/**
 * @brief Enable tap detection interrupt and handle detection events
 *
 * This function enables the tap detection feature, configures interrupts, and enters
 * a loop to handle tap detection events. When a tap is detected, it waits 3 seconds
 * before re-enabling the interrupt to avoid repeated triggers.
 *
 * The function performs the following steps:
 * 1. Enable accelerometer
 * 2. Configure tap detection features
 * 3. Enable triple tap detection
 * 4. Map feature interrupts to GPIO
 * 5. Enter event loop to handle interrupts
 *
 * @param bmi2_dev Pointer to BMI2 device structure
 * @return Result code from BMI2 API
 */
static int8_t bmi270_enable_tap_int(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    uint8_t sens_list[2] = { BMI2_ACCEL, BMI2_TRIPLE_TAP };  // Only enable accelerometer and triple tap
    uint16_t int_status = 0;
    uint8_t tap_output;
    struct bmi2_sens_int_config sens_int = { 
        .type = BMI2_TAP, 
        .hw_int_pin = BMI2_INT1 
    };

    /* First enable accelerometer */
    uint8_t accel_list[1] = { BMI2_ACCEL };
    rslt = bmi270_circle_sensor_enable(accel_list, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK)
    {
        /* Set feature configurations for tap detection. */
        rslt = set_feature_config(bmi2_dev);
        bmi2_error_codes_print_result(rslt);

        if (rslt == BMI2_OK)
        {
            /* Enable tap features using circle firmware - only triple tap */
            rslt = bmi270_circle_sensor_enable(sens_list, 2, bmi2_dev);
            bmi2_error_codes_print_result(rslt);
            
            if (rslt == BMI2_OK) {
                ESP_LOGI("MAIN", "Triple tap feature enabled successfully");
            } else {
                ESP_LOGE("MAIN", "Failed to enable triple tap feature");
            }

            if (rslt == BMI2_OK)
            {
                /* Map the feature interrupt for tap detection. */
                rslt = bmi270_circle_map_feat_int(&sens_int, 1, bmi2_dev);
                bmi2_error_codes_print_result(rslt);
                ESP_LOGI("MAIN", "Tap the board to detect triple tap");

                /* Main loop to handle tap detection interrupts. */
                while (rslt == BMI2_OK) {
                    if (interrupt_status) {
                        interrupt_status = false;
                        ESP_LOGI("MAIN", "Interrupt detected!");
                        
                        /* Clear the buffer. */
                        int_status = 0;

                        /* Get the interrupt status of tap detection. */
                        rslt = bmi2_get_int_status(&int_status, bmi2_dev);
                        bmi2_error_codes_print_result(rslt);

                        /* Check the interrupt status for tap detection. */
                        if (int_status & BMI270_CIRCLE_TAP_MASK)
                        {
                            ESP_LOGI("MAIN", "Tap interrupt detected!");
                            rslt = bmi2_get_regs(BMI270_CIRCLE_TAP_STATUS_REG, &tap_output, 1, bmi2_dev);
                            bmi2_error_codes_print_result(rslt);
                            ESP_LOGI("MAIN", "Tap output: 0x%02X", tap_output);

                            /* Check for triple tap only */
                            if (tap_output & BMI270_CIRCLE_TRIPLE_TAP_MASK)
                            {
                                ESP_LOGI("MAIN", "Triple Tap Detected!");
                                // 每次三击切换颜色模式（粉/橙/绿/青/关）并发送到 C6
                                s_color_mode = (uint8_t)((s_color_mode + 1) % 5);
                                ESP_LOGI("MAIN", "Switch color mode -> %u", s_color_mode);
                                example_send_color_by_mode(s_color_mode);
                            }
                            else
                            {
                                ESP_LOGI("MAIN", "Other tap detected (not triple)");
                            }

                            // 1. Remove interrupt handler to avoid repeated triggers
                            gpio_isr_handler_remove(I2C_INT_IO);
                            ESP_LOGI("MAIN", "Waiting 1 seconds before next detection...");
                            // 2. Delay 3 seconds before re-enabling interrupt
                            vTaskDelay(pdMS_TO_TICKS(1000));
                            // 3. Re-add interrupt handler
                            gpio_isr_handler_add(I2C_INT_IO, gpio_isr_edge_handler, (void*) I2C_INT_IO);
                            ESP_LOGI("MAIN", "Ready for next tap detection...");
                        }
                    } else {
                        vTaskDelay(pdMS_TO_TICKS(100)); // Polling delay
                    }
                }
            }
        }
    }

    return rslt;
}

/**
 * @brief Main application entry point
 *
 * This function initializes GPIO, I2C, and BMI270, then enables tap detection.
 * It also handles cleanup after the detection loop finishes.
 *
 * The main function performs the following steps:
 * 1. Configure GPIO for interrupt detection
 * 2. Install GPIO interrupt service
 * 3. Initialize I2C and BMI270 sensor
 * 4. Enable tap detection interrupt
 * 5. Clean up resources when done
 */
void app_main(void) {
    // Init Wi‑Fi (STA) and ESP-NOW for RGB message sending
    example_init_wifi_and_espnow();

    // Configure GPIO for BMI270 interrupt
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE, // Falling edge trigger to match low-level sensor interrupt
        .pin_bit_mask = (1ULL << I2C_INT_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&io_conf);

    // Install and add ISR service for GPIO interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(I2C_INT_IO, gpio_isr_edge_handler, (void*) I2C_INT_IO);

    // Initialize I2C and BMI270 sensor
    if (i2c_sensor_bmi270_init() != ESP_OK) {
        ESP_LOGE("MAIN", "BMI270 initialization failed");
        return;
    }

    // Enable tap detection interrupt
    int8_t rslt = bmi270_enable_tap_int(bmi_handle);
    if (rslt != BMI2_OK) {
        ESP_LOGE("MAIN", "BMI270 enable tap interrupt failed");
        bmi2_error_codes_print_result(rslt);
    }

    // Remove ISR handler and uninstall service
    gpio_isr_handler_remove(I2C_INT_IO);
    gpio_uninstall_isr_service();

    // Delete I2C bus and BMI270 handle
    i2c_bus_delete(&i2c_bus);
    bmi270_circle_sensor_del(bmi_handle);

    ESP_LOGI("MAIN", "BMI270 tap detection interrupt example finished.");
}
