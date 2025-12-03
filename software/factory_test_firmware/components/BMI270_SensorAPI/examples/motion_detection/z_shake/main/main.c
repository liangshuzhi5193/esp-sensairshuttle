/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bmi270.h"
#include "common/common.h"

/*! Earth's gravity in m/s^2 */
#define GRAVITY_EARTH       (9.80665f)

/*! Macros to select the sensors                   */
#define ACCEL               UINT8_C(0x00)
#define GYRO                UINT8_C(0x01)

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

#define I2C_MASTER_NUM      I2C_NUM_0               /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ  100000                  /*!< I2C master clock frequency */

static bmi270_handle_t bmi_handle = NULL;
static i2c_bus_handle_t i2c_bus;

/* Forward declarations */
static int8_t configure_gesture_axis_z_sign(struct bmi2_dev *bmi2_dev, uint8_t z_sign);
static int8_t read_gesture_axis_registers(struct bmi2_dev *bmi2_dev);

/**
 * @brief i2c master initialization
 */
static void i2c_sensor_bmi270_init(void)
{
    const i2c_config_t i2c_bus_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000
    };
#if UE_SW_I2C
    i2c_bus = i2c_bus_create(I2C_NUM_SW_1, &i2c_bus_conf);
#else
    i2c_bus = i2c_bus_create(I2C_MASTER_NUM, &i2c_bus_conf);
#endif
    if (i2c_bus == NULL) {
        ESP_LOGE("BMI270", "i2c_bus create returned NULL");
        return;
    }

    bmi270_i2c_config_t i2c_bmi270_conf = {
        .i2c_handle = i2c_bus,
        .i2c_addr = BMI270_I2C_ADDRESS,
    };
    bmi270_sensor_create(&i2c_bmi270_conf, &bmi_handle);
    if (bmi_handle == NULL) {
        ESP_LOGE("BMI270", "BMI270 create returned NULL");
        return;
    }
    
    /* Configure gesture Z-axis sign - can be adjusted as needed */
    /* Z-axis sign: 0=positive, 1=negative */
    int8_t rslt = configure_gesture_axis_z_sign(bmi_handle, 0); // Use default Z-axis positive direction
    if (rslt != BMI2_OK) {
        ESP_LOGW("BMI270", "Gesture Z-axis sign configuration failed, using default configuration");
    }
}

/*!
 * @brief This function converts lsb to meter per second squared for 16 bit accelerometer at
 * range 2G, 4G, 8G or 16G.
 */
static float lsb_to_mps2(int16_t val, float g_range, uint8_t bit_width)
{
    float half_scale = (float)(1 << (bit_width - 1));  // 2^(bit_width-1)

    return (GRAVITY_EARTH * val * g_range) / half_scale;
}

/*!
 * @brief Directly configure BMI270 register axis configuration (only affects gesture feature)
 * @param bmi2_dev: BMI270 device pointer
 * @param z_sign: Z-axis sign (0=positive, 1=negative)
 * @return BMI2_OK for success, other values for failure
 */
static int8_t configure_gesture_axis_z_sign(struct bmi2_dev *bmi2_dev, uint8_t z_sign)
{
    int8_t rslt;
    uint8_t feat_config[BMI2_FEAT_SIZE_IN_BYTES] = { 0 };
    uint8_t axis_map_offset = 0x04;  // BMI270_AXIS_MAP_STRT_ADDR
    
    /* Get current feature configuration */
    rslt = bmi2_get_regs(BMI2_FEATURES_REG_ADDR, feat_config, BMI2_FEAT_SIZE_IN_BYTES, bmi2_dev);
    if (rslt != BMI2_OK) {
        ESP_LOGE("BMI270", "Failed to get feature configuration");
        return rslt;
    }
    
    /* Only modify Z-axis sign bit (bit 0 of second byte) */
    uint8_t reg2 = feat_config[axis_map_offset + 1];
    reg2 = (reg2 & 0xFE) | (z_sign & 0x01);  // Clear bit 0, then set new Z-axis sign
    feat_config[axis_map_offset + 1] = reg2;
    
    /* Write back to register */
    rslt = bmi2_set_regs(BMI2_FEATURES_REG_ADDR, feat_config, BMI2_FEAT_SIZE_IN_BYTES, bmi2_dev);
    if (rslt == BMI2_OK) {
        ESP_LOGI("BMI270", "Z-axis sign configuration successful: Z-axis %s", z_sign ? "negative" : "positive");
    } else {
        ESP_LOGE("BMI270", "Z-axis sign configuration failed");
    }
    
    return rslt;
}

/*!
 * @brief Read current BMI270 gesture axis mapping register values
 * @param bmi2_dev: BMI270 device pointer
 * @return BMI2_OK for success, other values for failure
 */
static int8_t read_gesture_axis_registers(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    uint8_t feat_config[BMI2_FEAT_SIZE_IN_BYTES] = { 0 };
    uint8_t axis_map_offset = 0x04;  // BMI270_AXIS_MAP_STRT_ADDR
    
    /* Get current feature configuration */
    rslt = bmi2_get_regs(BMI2_FEATURES_REG_ADDR, feat_config, BMI2_FEAT_SIZE_IN_BYTES, bmi2_dev);
    if (rslt != BMI2_OK) {
        ESP_LOGE("BMI270", "Failed to read feature configuration");
        return rslt;
    }
    
    /* Parse axis mapping registers */
    uint8_t reg1 = feat_config[axis_map_offset];
    uint8_t reg2 = feat_config[axis_map_offset + 1];
    
    uint8_t x_axis = reg1 & 0x03;
    uint8_t x_sign = (reg1 >> 2) & 0x01;
    uint8_t y_axis = (reg1 >> 3) & 0x03;
    uint8_t y_sign = (reg1 >> 5) & 0x01;
    uint8_t z_axis = (reg1 >> 6) & 0x03;
    uint8_t z_sign = reg2 & 0x01;
    
    ESP_LOGI("BMI270", "Current wrist coordinate axis register configuration:");
    ESP_LOGI("BMI270", "  Register 1 (0x%02X): 0x%02X", axis_map_offset, reg1);
    ESP_LOGI("BMI270", "  Register 2 (0x%02X): 0x%02X", axis_map_offset + 1, reg2);
    ESP_LOGI("BMI270", "  X-axis: %d%s", x_axis, x_sign ? "-" : "+");
    ESP_LOGI("BMI270", "  Y-axis: %d%s", y_axis, y_sign ? "-" : "+");
    ESP_LOGI("BMI270", "  Z-axis: %d%s", z_axis, z_sign ? "-" : "+");
    
    return BMI2_OK;
}

/*!
 * @brief Function to detect complete Z-axis flip sequence (positive->negative->positive->negative)
 * @param bmi2_dev: BMI270 device pointer
 * @return BMI2_OK for success, other values for failure
 */
static int8_t detect_z_axis_complete_flip(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    struct bmi2_sens_data sensor_data;
    float acc_z_prev = 0.0f;
    float acc_z_current = 0.0f;
    uint8_t flip_detected = 0;
    uint8_t sample_count = 0;
    const uint8_t max_samples = 150; // Maximum sample count
    
    /* Flip state machine */
    enum {
        WAITING_FOR_POSITIVE,      // Waiting for positive direction
        WAITING_FOR_NEGATIVE,      // Waiting for negative direction
        WAITING_FOR_POSITIVE_AGAIN, // Waiting for positive direction again
        WAITING_FOR_NEGATIVE_AGAIN  // Waiting for negative direction again
    } state = WAITING_FOR_POSITIVE;
    
    /* Configure accelerometer */
    uint8_t sensor_list[1] = { BMI2_ACCEL };
    rslt = bmi2_sensor_enable(sensor_list, 1, bmi2_dev);
    if (rslt != BMI2_OK) {
        ESP_LOGE("BMI270", "Failed to enable accelerometer");
        return rslt;
    }
    
    /* Set accelerometer configuration */
    struct bmi2_sens_config config;
    config.type = BMI2_ACCEL;
    rslt = bmi2_get_sensor_config(&config, 1, bmi2_dev);
    if (rslt == BMI2_OK) {
        config.cfg.acc.odr = BMI2_ACC_ODR_100HZ;  // 100Hz sampling rate
        config.cfg.acc.range = BMI2_ACC_RANGE_2G;  // 2G range
        config.cfg.acc.bwp = BMI2_ACC_NORMAL_AVG4; // 4x averaging
        config.cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;
        rslt = bmi2_set_sensor_config(&config, 1, bmi2_dev);
    }
    
    if (rslt != BMI2_OK) {
        ESP_LOGE("BMI270", "Failed to configure accelerometer");
        return rslt;
    }
    
    ESP_LOGI("BMI270", "Starting Z-axis complete flip sequence detection (positive->negative->positive->negative)...");
    ESP_LOGI("BMI270", "Please flip the device from Z-axis positive direction to negative, then back to positive, finally to negative");
    ESP_LOGI("BMI270", "Sample data format: [sample_count] Z-axis acceleration(m/s²) [status]");
    
    /* Get initial Z-axis value */
    rslt = bmi2_get_sensor_data(&sensor_data, bmi2_dev);
    if (rslt == BMI2_OK) {
        acc_z_prev = lsb_to_mps2(sensor_data.acc.z, (float)2, bmi2_dev->resolution);
        ESP_LOGI("BMI270", "Initial Z-axis value: %.2f m/s²", acc_z_prev);
    }
    
    /* Continuously monitor Z-axis changes */
    while (sample_count < max_samples && !flip_detected) {
        rslt = bmi2_get_sensor_data(&sensor_data, bmi2_dev);
        
        if (rslt == BMI2_OK && (sensor_data.status & BMI2_DRDY_ACC)) {
            acc_z_current = lsb_to_mps2(sensor_data.acc.z, (float)2, bmi2_dev->resolution);
            sample_count++;
            
            /* State machine processing */
            switch (state) {
                case WAITING_FOR_POSITIVE:
                    if (acc_z_current > 0.5f) {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Positive direction detected]", sample_count, acc_z_current);
                        state = WAITING_FOR_NEGATIVE;
                    } else {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Waiting for positive direction]", sample_count, acc_z_current);
                    }
                    break;
                    
                case WAITING_FOR_NEGATIVE:
                    if (acc_z_current < -0.5f) {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Negative direction detected]", sample_count, acc_z_current);
                        state = WAITING_FOR_POSITIVE_AGAIN;
                    } else {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Waiting for negative direction]", sample_count, acc_z_current);
                    }
                    break;
                    
                case WAITING_FOR_POSITIVE_AGAIN:
                    if (acc_z_current > 0.5f) {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Positive direction detected again]", sample_count, acc_z_current);
                        state = WAITING_FOR_NEGATIVE_AGAIN;
                    } else {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Waiting for positive direction again]", sample_count, acc_z_current);
                    }
                    break;
                    
                case WAITING_FOR_NEGATIVE_AGAIN:
                    if (acc_z_current < -0.5f) {
                        flip_detected = 1;
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Complete flip sequence detected!]", sample_count, acc_z_current);
                        ESP_LOGI("BMI270", "Z-axis flip sequence: positive->negative->positive->negative");
                        ESP_LOGI("BMI270", "Trigger action: Complete Z-axis flip event");
                        
                        /* Execute flip trigger action */
                        ESP_LOGI("BMI270", "=== Executing complete flip trigger action ===");
                        ESP_LOGI("BMI270", "1. Record flip timestamp");
                        ESP_LOGI("BMI270", "2. Send complete flip notification");
                        ESP_LOGI("BMI270", "3. Update device status");
                        ESP_LOGI("BMI270", "4. Execute user-defined callback function");
                        ESP_LOGI("BMI270", "=== Complete flip action execution finished ===");
                    } else {
                        ESP_LOGI("BMI270", "[%2d] %.2f m/s² [Waiting for negative direction again]", sample_count, acc_z_current);
                    }
                    break;
            }
            
            acc_z_prev = acc_z_current;
        }
        
        /* Delay 100ms */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    if (!flip_detected) {
        ESP_LOGW("BMI270", "No complete Z-axis flip sequence detected within %d samples", max_samples);
        ESP_LOGW("BMI270", "Current state: %s", 
               state == WAITING_FOR_POSITIVE ? "Waiting for positive direction" :
               state == WAITING_FOR_NEGATIVE ? "Waiting for negative direction" :
               state == WAITING_FOR_POSITIVE_AGAIN ? "Waiting for positive direction again" : "Waiting for negative direction again");
    }
    
    /* Disable accelerometer */
    uint8_t disable_list[1] = { BMI2_ACCEL };
    bmi2_sensor_disable(disable_list, 1, bmi2_dev);
    
    return BMI2_OK;
}

void app_main(void)
{
    ESP_LOGI("BMI270", "BMI270 Z-axis Complete Flip Detection Example");
    
    /* Initialize I2C and BMI270 sensor */
    i2c_sensor_bmi270_init();
    
    if (bmi_handle == NULL) {
        ESP_LOGE("BMI270", "Failed to initialize BMI270 sensor");
        return;
    }
    
    /* Read current axis configuration */
    read_gesture_axis_registers(bmi_handle);
    
    ESP_LOGI("BMI270", "=== Testing Z-axis complete flip sequence detection ===");
    ESP_LOGI("BMI270", "Please flip the device from Z-axis positive direction to negative, then back to positive, finally to negative");
    ESP_LOGI("BMI270", "After detecting the complete flip sequence, an action will be triggered");
    
    /* Start Z-axis complete flip detection */
    int8_t rslt = detect_z_axis_complete_flip(bmi_handle);
    if (rslt != BMI2_OK) {
        ESP_LOGE("BMI270", "Z-axis complete flip detection failed");
    }
    
    /* Clean up */
    bmi270_sensor_del(bmi_handle);
    i2c_bus_delete(&i2c_bus);
    
    ESP_LOGI("BMI270", "Example completed");
}