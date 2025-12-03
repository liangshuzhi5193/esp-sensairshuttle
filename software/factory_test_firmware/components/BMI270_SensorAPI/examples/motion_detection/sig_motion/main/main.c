#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bmi270.h"
#include "common/common.h"

/**
 * @file main.c
 * @brief Example for BMI270 significant motion detection with ESP32
 *
 * This file demonstrates how to initialize the BMI270 sensor, configure it for significant motion detection,
 * and handle interrupts using ESP32 GPIO. The code includes I2C initialization, sensor configuration,
 * interrupt service routines, and main application logic.
 */

// Configure GPIO pins based on selected development board
#ifdef CONFIG_BOARD_ESP_SPOT_C5
#define I2C_INT_IO              3
#define I2C_MASTER_SCL_IO       26
#define I2C_MASTER_SDA_IO       25
#define UE_SW_I2C               1
#elif defined(CONFIG_BOARD_ESP_SPOT_S3)
#define I2C_INT_IO              5
#define I2C_MASTER_SCL_IO       1
#define I2C_MASTER_SDA_IO       2
#define UE_SW_I2C               1
#elif defined(CONFIG_BOARD_ESP_ASTOM_S3)
#define I2C_INT_IO              16
#define I2C_MASTER_SCL_IO       0
#define I2C_MASTER_SDA_IO       45
#define UE_SW_I2C               1
#elif defined(CONFIG_BOARD_ESP_ECHOEAR_S3)
#define I2C_INT_IO              21
#define I2C_MASTER_SCL_IO       1
#define I2C_MASTER_SDA_IO       2
#define UE_SW_I2C               0
#else
#define I2C_INT_IO              CONFIG_I2C_INT_IO
#define I2C_MASTER_SCL_IO       CONFIG_I2C_MASTER_SCL_IO
#define I2C_MASTER_SDA_IO       CONFIG_I2C_MASTER_SDA_IO
#define UE_SW_I2C               CONFIG_UE_SW_I2C
#endif

#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      (100 * 1000)

// Global handles for BMI270 and I2C bus
static bmi270_handle_t bmi_handle = NULL;
static i2c_bus_handle_t i2c_bus;
static volatile bool interrupt_status = false; // Flag set by ISR when interrupt occurs

/**
 * @brief GPIO interrupt service routine for BMI270 INT pin
 *
 * This ISR sets the interrupt_status flag when an interrupt is detected on the configured GPIO.
 * @param arg GPIO number (cast from void*)
 */
static void IRAM_ATTR gpio_isr_edge_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    interrupt_status = true;
    esp_rom_printf("GPIO[%u] intr, val: %d\n", gpio_num, gpio_get_level(gpio_num));
}

/**
 * @brief Initialize I2C bus and BMI270 sensor
 *
 * This function configures the I2C bus and initializes the BMI270 sensor.
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
static esp_err_t i2c_sensor_bmi270_init(void)
{
    const i2c_config_t i2c_bus_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
#if UE_SW_I2C
    i2c_bus = i2c_bus_create(I2C_NUM_SW_1, &i2c_bus_conf);
#else
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &i2c_bus_conf);
#endif
    if (!i2c_bus) {
        ESP_LOGE("MAIN", "I2C bus create failed");
        return ESP_FAIL;
    }

    bmi270_i2c_config_t i2c_bmi270_conf = {
        .i2c_handle = i2c_bus,
        .i2c_addr = BMI270_I2C_ADDRESS,
    };

    if (bmi270_sensor_create(&i2c_bmi270_conf, &bmi_handle) != ESP_OK || bmi_handle == NULL) {
        ESP_LOGE("MAIN", "BMI270 create failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Configure BMI270 for significant motion detection and set INT1 pin
 *
 * This function sets up the sensor configuration for significant motion detection and configures the INT1 pin
 * for interrupt output.
 * @param bmi2_dev Pointer to BMI2 device structure
 * @return Result code from BMI2 API
 */
static int8_t set_feature_config(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    struct bmi2_sens_config config;
    struct bmi2_int_pin_config pin_config = { 0 };

    config.type = BMI2_SIG_MOTION;

    rslt = bmi270_get_sensor_config(&config, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    rslt = bmi2_get_int_pin_config(&pin_config, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        /* Configure significant motion detection parameters */
        config.cfg.sig_motion.block_size = 0x01; 
        
        ESP_LOGI("MAIN", "Setting sig_motion block_size to 0x01 for medium sensitivity");
        
        rslt = bmi270_set_sensor_config(&config, 1, bmi2_dev);
        bmi2_error_codes_print_result(rslt);

        pin_config.pin_type = BMI2_INT1;
        pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
        pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;
        pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
        pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
        pin_config.int_latch = BMI2_INT_NON_LATCH;

        rslt = bmi2_set_int_pin_config(&pin_config, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
    }

    return rslt;
}

/**
 * @brief Enable significant motion detection interrupt and handle detection events
 *
 * This function enables the significant motion detection feature, configures interrupts, and enters a loop
 * to handle significant motion detection events.
 * @param bmi2_dev Pointer to BMI2 device structure
 * @return Result code from BMI2 API
 */
static int8_t bmi270_enable_sig_motion_int(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    uint8_t sens_list[2] = { BMI2_ACCEL, BMI2_SIG_MOTION };
    uint16_t int_status = 0;
    struct bmi2_sens_int_config sens_int = { 
        .type = BMI2_SIG_MOTION, 
        .hw_int_pin = BMI2_INT1 
    };

    rslt = bmi270_sensor_enable(sens_list, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        rslt = set_feature_config(bmi2_dev);
        bmi2_error_codes_print_result(rslt);

        if (rslt == BMI2_OK) {
            rslt = bmi270_map_feat_int(&sens_int, 1, bmi2_dev);
            bmi2_error_codes_print_result(rslt);
            ESP_LOGI("MAIN", "Move the device for 3 seconds in any direction");

            while (rslt == BMI2_OK) {
                if (interrupt_status) {
                    interrupt_status = false;
                    
                    rslt = bmi2_get_int_status(&int_status, bmi2_dev);
                    bmi2_error_codes_print_result(rslt);
                    
                    if (int_status & BMI270_INT_SIG_MOT_MASK) {
                        ESP_LOGI("MAIN", "Significant motion interrupt is generated!");
                        
                        gpio_isr_handler_remove(I2C_INT_IO);
                        ESP_LOGI("MAIN", "Waiting 3 seconds before next detection...");
                        vTaskDelay(pdMS_TO_TICKS(3000)); // wait 3 seconds
                        
                        gpio_isr_handler_add(I2C_INT_IO, gpio_isr_edge_handler, (void*) I2C_INT_IO);
                        ESP_LOGI("MAIN", "=== READY FOR NEXT DETECTION ===");
                        ESP_LOGI("MAIN", "Ready for next significant motion detection...");
                    }
                } else {
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        }
    }
    return rslt;
}

/**
 * @brief Main application entry point
 *
 * This function initializes GPIO, I2C, and BMI270, then enables significant motion detection.
 * It also handles cleanup after the detection loop finishes.
 */
void app_main(void) {
    // Print current configuration information
    ESP_LOGI("MAIN", "=== BMI270 Significant Motion Detection Configuration ===");
#ifdef CONFIG_BOARD_ESP_SPOT_C5
    ESP_LOGI("MAIN", "Selected Board: ESP SPOT C5");
#elif defined(CONFIG_BOARD_ESP_SPOT_S3)
    ESP_LOGI("MAIN", "Selected Board: ESP SPOT S3");
#elif defined(CONFIG_BOARD_ESP_ASTOM_S3)
    ESP_LOGI("MAIN", "Selected Board: ESP ASTOM S3");
#elif defined(CONFIG_BOARD_ESP_ECHOEAR_S3)
    ESP_LOGI("MAIN", "Selected Board: ESP ECHOEAR S3");
#elif defined(CONFIG_BOARD_CUSTOM)
    ESP_LOGI("MAIN", "Selected Board: Other Boards (Custom)");
#else
    ESP_LOGI("MAIN", "Selected Board: Default (ECHOEAR_S3)");
#endif
    ESP_LOGI("MAIN", "GPIO Configuration:");
    ESP_LOGI("MAIN", "  - Interrupt GPIO: %d", I2C_INT_IO);
    ESP_LOGI("MAIN", "  - I2C SCL GPIO: %d", I2C_MASTER_SCL_IO);
    ESP_LOGI("MAIN", "  - I2C SDA GPIO: %d", I2C_MASTER_SDA_IO);
#ifdef CONFIG_UE_SW_I2C
    ESP_LOGI("MAIN", "  - I2C Type: Software I2C");
#else
    ESP_LOGI("MAIN", "  - I2C Type: Hardware I2C");
#endif
    ESP_LOGI("MAIN", "================================");

    // Configure GPIO for BMI270 interrupt
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,  // Changed to falling edge trigger to match BMI2_INT_ACTIVE_LOW
        .pin_bit_mask = (1ULL << I2C_INT_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Changed to pull-up to ensure high level when idle
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
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

    // Enable significant motion detection interrupt
    int8_t rslt = bmi270_enable_sig_motion_int(bmi_handle);
    bmi2_error_codes_print_result(rslt);
    if (rslt != BMI2_OK) {
        ESP_LOGE("MAIN", "BMI270 enable significant motion interrupt failed");
    }

    // Remove ISR handler and uninstall service
    gpio_isr_handler_remove(I2C_INT_IO);
    gpio_uninstall_isr_service();

    // Delete I2C bus and BMI270 handle
    bmi270_sensor_del(bmi_handle);
    i2c_bus_delete(&i2c_bus);

    ESP_LOGI("MAIN", "BMI270 significant motion detection interrupt example finished.");
}
