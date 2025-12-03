#include "bmi270_bmm350.h"

/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file main.c
 * @brief BMI270 + BMM350 9-axis sensor data collection example
 * 
 * This example demonstrates how to use BMI270 (accelerometer + gyroscope) and BMM350 (magnetometer) sensors
 * to collect 9-axis data simultaneously through ESP32's I2C bus, without using BMI270's AUX interface.
 * 
 * Features:
 * - BMI270: 6-axis raw data (3-axis accelerometer + 3-axis gyroscope)
 * - BMM350: 3-axis magnetometer data (raw data + parsed direction data)
 * - Real-time 9-axis sensor data display
 * - Motion detection and direction recognition
 * 
 * @version 1.0.0
 * @date 2025-01-14
 */

#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bmi270.h"
#include "bmm350.h"
#include "bmm350_defs.h"
#include "../../../components/BMM350_SensorAPI/examples/common/common.h"
#include "i2c_bus.h"
#include "sdkconfig.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define I2C_MASTER_FREQ_HZ (100 * 1000)
#define SDO_PIN             9        /* SDO pin (keep LOW to I2C primary address) */

/* Global device handles */
static bmi270_handle_t bmi_handle = NULL;
static i2c_bus_handle_t i2c_bus;
static struct bmi2_dev *bmi2_dev = NULL;
static struct bmm350_dev bmm350_dev = { 0 };
static uint8_t bmm350_addr = 0x14; /* BMM350 I2C address */

/* Log tag */
static const char *TAG = "BMI270_BMM350";

/* Motion detection state */
static volatile bool s_motion_detected = false;
static volatile bool s_rotation_detected = false;

/* Magnetometer calibration data */
static float mag_offset_x = 0.0f;
static float mag_offset_y = 0.0f;
static float mag_offset_z = 0.0f;
static float mag_scale_x = 1.0f;
static float mag_scale_y = 1.0f;
static float mag_scale_z = 1.0f;
static bool mag_calibrated = false;

bool sensors_initialized = false;

/**
 * @brief Initialize I2C bus and BMI270 driver handle
 */
static esp_err_t i2c_sensor_bmi270_init(void)
{
    /* Configure I2C bus parameters */
    const i2c_config_t i2c_bus_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = CONFIG_I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    
    i2c_bus = i2c_bus_create(I2C_NUM_0, &i2c_bus_conf);
    if (i2c_bus == NULL) {
        ESP_LOGE(TAG, "I2C bus creation failed");
        return ESP_FAIL;
    }
    
    /* Create BMI270 driver handle */
    bmi270_i2c_config_t i2c_bmi270_conf = {
        .i2c_handle = i2c_bus,
        .i2c_addr = BMI270_I2C_ADDRESS,
    };
    if (bmi270_sensor_create(&i2c_bmi270_conf, &bmi_handle) != ESP_OK || bmi_handle == NULL) {
        ESP_LOGE(TAG, "BMI270 creation failed");
        return ESP_FAIL;
    }
    
    /* Get BMI270 underlying structure pointer */
    bmi2_dev = (struct bmi2_dev *)bmi_handle;
    return ESP_OK;
}

/**
 * @brief Initialize BMI270 chip
 */
static int8_t bmi270_init_and_config(void)
{
    int8_t rslt;
    
    /* Initialize BMI270 chip */
    rslt = bmi270_init(bmi2_dev);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 initialization failed: %d", rslt);
        return rslt;
    }
    ESP_LOGI(TAG, "BMI270 initialized successfully");
    
    /* Configure accelerometer and gyroscope */
    struct bmi2_sens_config sens_cfg[2] = {0};
    
    /* Configure accelerometer */
    sens_cfg[0].type = BMI2_ACCEL;
    sens_cfg[0].cfg.acc.odr = BMI2_ACC_ODR_100HZ;
    sens_cfg[0].cfg.acc.bwp = BMI2_ACC_NORMAL_AVG4;
    sens_cfg[0].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;
    sens_cfg[0].cfg.acc.range = BMI2_ACC_RANGE_4G;
    
    /* Configure gyroscope */
    sens_cfg[1].type = BMI2_GYRO;
    sens_cfg[1].cfg.gyr.odr = BMI2_GYR_ODR_100HZ;
    sens_cfg[1].cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;
    sens_cfg[1].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;
    sens_cfg[1].cfg.gyr.range = BMI2_GYR_RANGE_2000;
    
    rslt = bmi2_set_sensor_config(sens_cfg, 2, bmi2_dev);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 sensor configuration failed: %d", rslt);
        return rslt;
    }
    
    ESP_LOGI(TAG, "BMI270 sensor configuration completed");
    return BMI2_OK;
}

/**
 * @brief Initialize BMM350 magnetometer
 */
static int8_t bmm350_init_standalone(void)
{
    /* Try different I2C addresses */
    const uint8_t addr_candidates[] = { 0x14, 0x15 };
    int8_t rslt = BMM350_E_DEV_NOT_FOUND;

    for (size_t i = 0; i < sizeof(addr_candidates); ++i) {
        uint8_t addr = addr_candidates[i];
        bmm350_addr = addr;
        bmm350_dev.read = bmm350_i2c_read;
        bmm350_dev.write = bmm350_i2c_write;
        bmm350_dev.delay_us = bmm350_delay;
        bmm350_dev.intf_ptr = (void *)&bmm350_addr;

        /* Initialize BMM350 */
        rslt = bmm350_init(&bmm350_dev);
        ESP_LOGI(TAG, "BMM350 init result at address 0x%02X: %d, chip_id=0x%02X (expect 0x33)", 
                 addr, rslt, bmm350_dev.chip_id);

        if (bmm350_dev.chip_id == BMM350_CHIP_ID) {
            if (rslt != BMM350_OK) {
                (void)bmm350_soft_reset(&bmm350_dev);
                bmm350_delay(BMM350_SOFT_RESET_DELAY + 10000, &bmm350_dev);
            }

            /* Wait for PMU busy to clear */
            struct bmm350_pmu_cmd_status_0 pmu0 = { 0 };
            for (int t = 0; t < 10; ++t) {
                (void)bmm350_get_pmu_cmd_status_0(&pmu0, &bmm350_dev);
                if (pmu0.pmu_cmd_busy == 0) {
                    break;
                }
                bmm350_delay(5000, &bmm350_dev);
            }

            /* Configure ODR/averaging and enable XYZ axes, enter NORMAL mode */
            (void)bmm350_set_odr_performance(BMM350_DATA_RATE_100HZ, BMM350_AVERAGING_4, &bmm350_dev);
            (void)bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, &bmm350_dev);
            (void)bmm350_set_powermode(BMM350_NORMAL_MODE, &bmm350_dev);
            
            ESP_LOGI(TAG, "BMM350 configuration completed");
            return BMM350_OK;
        }
    }

    return rslt;
}

/**
 * @brief Enable BMI270 sensors
 */
static int8_t enable_bmi270_sensors(void)
{
    int8_t rslt;
    uint8_t sens_list[2] = { BMI2_ACCEL, BMI2_GYRO };
    rslt = bmi2_sensor_enable(sens_list, 2, bmi2_dev);
    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "BMI270 sensor enable failed: %d", rslt);
        return rslt;
    }
    ESP_LOGI(TAG, "BMI270 sensors enabled");
    return BMI2_OK;
}

/**
 * @brief Simple magnetometer calibration
 */
static void calibrate_magnetometer(void)
{
    if (mag_calibrated) return;
    
    ESP_LOGI(TAG, "Starting magnetometer calibration, please slowly rotate device 360 degrees...");
    
    float min_x = 1000.0f, max_x = -1000.0f;
    float min_y = 1000.0f, max_y = -1000.0f;
    float min_z = 1000.0f, max_z = -1000.0f;
    
    // Collect calibration data
    for (int i = 0; i < 100; i++) {
        struct bmm350_mag_temp_data mag_data = {0};
        int8_t rslt = bmm350_get_compensated_mag_xyz_temp_data(&mag_data, &bmm350_dev);
        if (rslt == BMM350_OK) {
            if (mag_data.x < min_x) min_x = mag_data.x;
            if (mag_data.x > max_x) max_x = mag_data.x;
            if (mag_data.y < min_y) min_y = mag_data.y;
            if (mag_data.y > max_y) max_y = mag_data.y;
            if (mag_data.z < min_z) min_z = mag_data.z;
            if (mag_data.z > max_z) max_z = mag_data.z;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Calculate offset and scale
    mag_offset_x = (min_x + max_x) / 2.0f;
    mag_offset_y = (min_y + max_y) / 2.0f;
    mag_offset_z = (min_z + max_z) / 2.0f;
    
    mag_scale_x = (max_x - min_x) / 2.0f;
    mag_scale_y = (max_y - min_y) / 2.0f;
    mag_scale_z = (max_z - min_z) / 2.0f;
    
    // Normalize scale factors
    float avg_scale = (mag_scale_x + mag_scale_y + mag_scale_z) / 3.0f;
    mag_scale_x = avg_scale / mag_scale_x;
    mag_scale_y = avg_scale / mag_scale_y;
    mag_scale_z = avg_scale / mag_scale_z;
    
    mag_calibrated = true;
    ESP_LOGI(TAG, "Magnetometer calibration completed");
    ESP_LOGI(TAG, "Offset: X=%.2f, Y=%.2f, Z=%.2f", mag_offset_x, mag_offset_y, mag_offset_z);
    ESP_LOGI(TAG, "Scale: X=%.2f, Y=%.2f, Z=%.2f", mag_scale_x, mag_scale_y, mag_scale_z);
}

/**
 * @brief Calculate magnetometer heading angle (improved version)
 */
static float calculate_heading(float mag_x, float mag_y)
{
    // Calculate raw angle
    float heading = atan2(mag_y, mag_x) * 180.0f / M_PI;
    
    // Convert to 0-360 degrees
    if (heading < 0) {
        heading += 360.0f;
    }
    
    // Magnetic declination correction (Beijing example, about -6 degrees, adjust according to actual location)
    float magnetic_declination = -6.0f;  // Beijing area magnetic declination
    heading += magnetic_declination;
    
    // Ensure angle is within 0-360 range
    if (heading < 0) {
        heading += 360.0f;
    } else if (heading >= 360.0f) {
        heading -= 360.0f;
    }
    
    return heading;
}

/**
 * @brief Get direction description
 */
static const char* get_direction_name(float heading)
{
    if (heading >= 337.5f || heading < 22.5f) return "North";
    if (heading >= 22.5f && heading < 67.5f) return "Northeast";
    if (heading >= 67.5f && heading < 112.5f) return "East";
    if (heading >= 112.5f && heading < 157.5f) return "Southeast";
    if (heading >= 157.5f && heading < 202.5f) return "South";
    if (heading >= 202.5f && heading < 247.5f) return "Southwest";
    if (heading >= 247.5f && heading < 292.5f) return "West";
    if (heading >= 292.5f && heading < 337.5f) return "Northwest";
    return "Unknown";
}

/**
 * @brief Collect and print 9-axis sensor data
 */
void print_9axis_sensor_data(void)
{
    int8_t rslt;
    struct bmi2_sens_data sensor_data = {0};
    struct bmm350_mag_temp_data mag_data = {0};
    
    /* Read BMI270 accelerometer and gyroscope data */
    rslt = bmi2_get_sensor_data(&sensor_data, bmi2_dev);
    if (rslt == BMI2_OK) {
        /* Parse accelerometer data */
        float acc_x_mg = (float)sensor_data.acc.x / 16384.0f;
        float acc_y_mg = (float)sensor_data.acc.y / 16384.0f;
        float acc_z_mg = (float)sensor_data.acc.z / 16384.0f;
        
        /* Parse gyroscope data */
        float gyro_x_dps = (float)sensor_data.gyr.x / 16.4f;
        float gyro_y_dps = (float)sensor_data.gyr.y / 16.4f;
        float gyro_z_dps = (float)sensor_data.gyr.z / 16.4f;
        
        /* Motion detection */
        static float last_acc_x = 0, last_acc_y = 0, last_acc_z = 0;
        float acc_diff_x = fabs(acc_x_mg - last_acc_x);
        float acc_diff_y = fabs(acc_y_mg - last_acc_y);
        float acc_diff_z = fabs(acc_z_mg - last_acc_z);
        
        if (acc_diff_x > 50 || acc_diff_y > 50 || acc_diff_z > 50) {
            ESP_LOGI(TAG, "Motion detected!");
            s_motion_detected = true;
        }
        
        /* Print BMI270 6-axis raw data */
        ESP_LOGI(TAG, "[BMI270] Acceleration(mg): X=%.1f, Y=%.1f, Z=%.1f | Gyroscope(dps): X=%.1f, Y=%.1f, Z=%.1f", 
                 acc_x_mg, acc_y_mg, acc_z_mg, gyro_x_dps, gyro_y_dps, gyro_z_dps);
        
        last_acc_x = acc_x_mg;
        last_acc_y = acc_y_mg;
        last_acc_z = acc_z_mg;
        
    } else {
        ESP_LOGE(TAG, "BMI270 data read failed: %d", rslt);
    }
    
    /* Read BMM350 magnetometer data */
    rslt = bmm350_get_compensated_mag_xyz_temp_data(&mag_data, &bmm350_dev);
    if (rslt == BMM350_OK) {
        // Apply calibration data
        float mag_x = (mag_data.x - mag_offset_x) * mag_scale_x;
        float mag_y = (mag_data.y - mag_offset_y) * mag_scale_y;
        float mag_z = (mag_data.z - mag_offset_z) * mag_scale_z;
        float temperature = mag_data.temperature;
        
        /* Calculate magnetic field strength */
        float mag_strength = sqrt(mag_x * mag_x + mag_y * mag_y + mag_z * mag_z);
        
        /* Calculate heading angle */
        float heading = calculate_heading(mag_x, mag_y);
        const char* direction = get_direction_name(heading);
        
        /* Print BMM350 3-axis magnetometer data */
        ESP_LOGI(TAG, "[BMM350] Raw magnetic field(uT): X=%.2f, Y=%.2f, Z=%.2f", mag_data.x, mag_data.y, mag_data.z);
        ESP_LOGI(TAG, "[BMM350] Calibrated magnetic field(uT): X=%.2f, Y=%.2f, Z=%.2f, Strength=%.2f, Temperature=%.2f°C", 
                 mag_x, mag_y, mag_z, mag_strength, temperature);
        ESP_LOGI(TAG, "[BMM350] Direction: %s (%.1f°)", direction, heading);
        
        /* Magnetic field quality check */
        if (mag_strength < 20.0f || mag_strength > 80.0f) {
            ESP_LOGW(TAG, "Warning: Abnormal magnetic field strength (%.2f uT), may affect direction accuracy", mag_strength);
        }
        
        /* Magnetic field change detection */
        static float last_mag_x = 0, last_mag_y = 0, last_mag_z = 0;
        float mag_diff_x = fabs(mag_x - last_mag_x);
        float mag_diff_y = fabs(mag_y - last_mag_y);
        float mag_diff_z = fabs(mag_z - last_mag_z);
        
        if (mag_diff_x > 10 || mag_diff_y > 10 || mag_diff_z > 10) {
            ESP_LOGI(TAG, "Magnetic field change detected: X=%.1f, Y=%.1f, Z=%.1f uT", mag_diff_x, mag_diff_y, mag_diff_z);
        }
        
        last_mag_x = mag_x;
        last_mag_y = mag_y;
        last_mag_z = mag_z;
        
    } else {
        ESP_LOGE(TAG, "BMM350 data read failed: %d", rslt);
    }
}

esp_err_t start_bmi270_bmm350_new(void)
{
    esp_err_t ret;
    
    /* Configure SDO pin */
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << SDO_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) return ret;
    gpio_set_level(SDO_PIN, 0);

    /* Initialize I2C and BMI270 */
    if (i2c_sensor_bmi270_init() != ESP_OK) return ESP_FAIL;
    if (bmi270_init_and_config() != BMI2_OK) return ESP_FAIL;
    
    /* Initialize BMM350 */
    if (bmm350_interface_init(&bmm350_dev) != BMM350_OK) return ESP_FAIL;
    if (bmm350_init_standalone() != BMM350_OK) return ESP_FAIL;

    if (enable_bmi270_sensors() != BMI2_OK) return ESP_FAIL;

    calibrate_magnetometer();

    sensors_initialized = true;
    return ESP_OK;
}

esp_err_t bmi270_bmm350_get_data(sensor_9axis_data_t *data)
{
    if (!sensors_initialized || !data) return ESP_FAIL;

    int8_t rslt;
    struct bmi2_sens_data bmi_data = {0};
    struct bmm350_mag_temp_data mag_data = {0};

    /* Read BMI270 */
    rslt = bmi2_get_sensor_data(&bmi_data, bmi2_dev);
    if (rslt != BMI2_OK) return ESP_FAIL;

    data->acc_x  = (float)bmi_data.acc.x / 16384.0f;
    data->acc_y  = (float)bmi_data.acc.y / 16384.0f;
    data->acc_z  = (float)bmi_data.acc.z / 16384.0f;
    data->gyro_x = (float)bmi_data.gyr.x / 16.4f;
    data->gyro_y = (float)bmi_data.gyr.y / 16.4f;
    data->gyro_z = (float)bmi_data.gyr.z / 16.4f;

    /* Read BMM350 */
    rslt = bmm350_get_compensated_mag_xyz_temp_data(&mag_data, &bmm350_dev);
    if (rslt != BMM350_OK) return ESP_FAIL;

    data->mag_x = (mag_data.x - mag_offset_x) * mag_scale_x;
    data->mag_y = (mag_data.y - mag_offset_y) * mag_scale_y;
    data->mag_z = (mag_data.z - mag_offset_z) * mag_scale_z;

    return ESP_OK;
}

void stop_bmi270_bmm350(void)
{
    if (bmi_handle) {
        bmi270_sensor_del(bmi_handle);
        bmi_handle = NULL;
    }
    if (i2c_bus) {
        i2c_bus_delete(&i2c_bus);
        i2c_bus = NULL;
    }
    bmm350_coines_deinit();
    sensors_initialized = false;
}