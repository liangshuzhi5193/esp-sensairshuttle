/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// 1. C 标准库头文件
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "lvgl.h"
#include "sdkconfig.h"
#include "ui/ui.h"
#include "ui/ui_events.h"
#include "audio_test.h"
#include "bme680.h"
#include "bmi270_bmm350.h"
#include "flash.h"

#define APP_DISP_DEFAULT_BRIGHTNESS 70

// sensor 1: temperature (°C)
// sensor 2: humidity (%)
// sensor 3: pressure (kPa)
// sensor 4: gas resistance (Ω)
#define SENSOR_TEMP_MIN -40
#define SENSOR_TEMP_MAX 85
#define SENSOR_HUMIDITY_MIN 0
#define SENSOR_HUMIDITY_MAX 100
#define SENSOR_PRESSURE_MIN 30
#define SENSOR_PRESSURE_MAX 110
#define SENSOR_GAS_MIN 0
#define SENSOR_GAS_MAX 1000000

static const char *TAG = "Factory Test Firmware";
static esp_codec_dev_handle_t spk_codec_dev;

static EventBits_t app_ui_event = 0;

// Check if all pending tests are completed
static bool are_all_devices_idle(void)
{
    const char* keys[] = {"touch", "display", "loudspeaker", "mic", "sensor1", "sensor2"};
    const size_t key_count = sizeof(keys) / sizeof(keys[0]);

    for (size_t i = 0; i < key_count; i++) {
        uint8_t state = flash_read_state((char*)keys[i]);
        if (state != 0) { 
            return false;
        }
    }

    return true; 
}

// Test if the sensor values are within the expected range
static bool sensor_bme680_test(bme680_data_t *data)
{
    bool pass = true; 
    
    data->pressure /= 1000;
    lv_label_set_text_fmt(ui_Label9, "%d", (int)SENSOR_TEMP_MIN);
    lv_label_set_text_fmt(ui_Label1, "%d", (int)SENSOR_TEMP_MAX);
    lv_label_set_text_fmt(ui_Label12, "%d", (int)SENSOR_HUMIDITY_MIN);
    lv_label_set_text_fmt(ui_Label13, "%d", (int)SENSOR_HUMIDITY_MAX);
    lv_label_set_text_fmt(ui_Label15, "%dk", (int)SENSOR_PRESSURE_MIN);
    lv_label_set_text_fmt(ui_Label16, "%dk", (int)SENSOR_PRESSURE_MAX);
    lv_label_set_text_fmt(ui_Label22, "%d", (int)SENSOR_GAS_MIN);
    lv_label_set_text_fmt(ui_Label23, "%dk", (int)SENSOR_GAS_MAX / 1000);

    lv_bar_set_range(ui_Bar1, (int32_t)SENSOR_TEMP_MIN, (int32_t)SENSOR_TEMP_MAX);
    lv_bar_set_range(ui_Bar2, (int32_t)SENSOR_HUMIDITY_MIN, (int32_t)SENSOR_HUMIDITY_MAX);
    lv_bar_set_range(ui_Bar3, (int32_t)SENSOR_PRESSURE_MIN, (int32_t)SENSOR_PRESSURE_MAX);
    lv_bar_set_range(ui_Bar4, (int32_t)SENSOR_GAS_MIN, (int32_t)SENSOR_GAS_MAX);

    lv_label_set_text_fmt(ui_Label10, "%.1f℃", data->temperature);
    lv_label_set_text_fmt(ui_Label3, "%.1f%%", data->humidity);
    lv_label_set_text_fmt(ui_Label17, "%.1fkPa", data->pressure);
    if (data->gas_resistance < 1000) {
        lv_label_set_text_fmt(ui_Label20, "%.1fΩ", data->gas_resistance);
    } else {
        lv_label_set_text_fmt(ui_Label20, "%.1fkΩ", data->gas_resistance / 1000);
    }

    lv_bar_set_value(ui_Bar1, (int32_t)data->temperature, LV_ANIM_OFF);
    lv_bar_set_value(ui_Bar2, (int32_t)data->humidity, LV_ANIM_OFF);
    lv_bar_set_value(ui_Bar3, (int32_t)data->pressure, LV_ANIM_OFF);
    lv_bar_set_value(ui_Bar4, (int32_t)data->gas_resistance, LV_ANIM_OFF);

    if (data->temperature > SENSOR_TEMP_MIN && data->temperature < SENSOR_TEMP_MAX) {
        lv_obj_set_style_text_color(ui_Label11, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label11, "通过");
    } else {
        lv_obj_set_style_text_color(ui_Label11, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label11, "失败");
        pass = false;
    }

    if (data->humidity > SENSOR_HUMIDITY_MIN && data->humidity < SENSOR_HUMIDITY_MAX) {
        lv_obj_set_style_text_color(ui_Label4, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label4, "通过");
    } else {
        lv_obj_set_style_text_color(ui_Label4, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label4, "失败");
        pass = false;
    }

    if (data->pressure > SENSOR_PRESSURE_MIN && data->pressure < SENSOR_PRESSURE_MAX) {
        lv_obj_set_style_text_color(ui_Label18, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label18, "通过");
    } else {
        lv_obj_set_style_text_color(ui_Label18, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label18, "失败");
        pass = false;
    }

    if (data->gas_resistance > SENSOR_GAS_MIN && data->gas_resistance < SENSOR_GAS_MAX) {
        lv_obj_set_style_text_color(ui_Label21, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label21, "通过");
    } else {
        lv_obj_set_style_text_color(ui_Label21, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_Label21, "失败");
        pass = false;
    }

    return pass; 
}

// Test if the accelerometer, gyroscope, and magnetometer values are within the expected range
static bool sensor_bmi270_bmm350_test(sensor_9axis_data_t *data)
{
    bool pass = true;
    lv_label_set_text_fmt(ui_AccXVal, "%.2f", data->acc_x);
    lv_label_set_text_fmt(ui_AccYVal, "%.2f", data->acc_y);
    lv_label_set_text_fmt(ui_AccZVal, "%.2f", data->acc_z);
    lv_label_set_text_fmt(ui_GyroXVal1, "%.2f", data->gyro_x);
    lv_label_set_text_fmt(ui_GyroYVal1, "%.2f", data->gyro_y);
    lv_label_set_text_fmt(ui_GyroZVal1, "%.2f", data->gyro_z);
    lv_label_set_text_fmt(ui_MagXVal, "%.2f", data->mag_x);
    lv_label_set_text_fmt(ui_MagYVal, "%.2f", data->mag_y);
    lv_label_set_text_fmt(ui_MagZVal, "%.2f", data->mag_z);
    if (data->acc_x == 0 && data->acc_y == 0 && data->acc_z == 0) {
        lv_obj_set_style_text_color(ui_AccStatus, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_AccStatus, "异常");
        pass = false;
    } else {
        lv_obj_set_style_text_color(ui_AccStatus, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_AccStatus, "正常");
    }
    if (data->gyro_x == 0 && data->gyro_y == 0 && data->gyro_z == 0) {
        lv_obj_set_style_text_color(ui_GyroStatus, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_GyroStatus, "异常");
        pass = false;
    } else {
        lv_obj_set_style_text_color(ui_GyroStatus, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_GyroStatus, "正常");
    }
    if (data->mag_x == 0 && data->mag_y == 0 && data->mag_z == 0) {
        lv_obj_set_style_text_color(ui_MagStatus, lv_color_hex(0xFF0303), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_MagStatus, "异常");   
        pass = false;
    } else {
        lv_obj_set_style_text_color(ui_MagStatus, lv_color_hex(0x175F11), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_MagStatus, "正常");
    }
    return pass;
}

void app_main(void)
{
    ui_event_init();
    audio_test_play_music();
    flash_write_init();
    lv_display_t *disp = bsp_display_start();
    if (disp == NULL) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }
    bme680_test();
    bsp_display_brightness_set(APP_DISP_DEFAULT_BRIGHTNESS);
    bsp_display_rotate(disp, LV_DISP_ROTATION_180);
    bsp_display_lock(0);
    if (are_all_devices_idle() == true) {
        ui_init(SCREEN_TOUCH);
        ESP_LOGI(TAG, "Touch Screen");
        set_first_test(true);
    } else {
        ui_init(SCREEN_FINAL);
        ESP_LOGI(TAG, "Final Screen");
        Check_result(NULL);
        set_first_test(false);
    }
    bsp_display_unlock();
    while(1) {
        app_ui_event = ui_event_wait_events();
        if (app_ui_event & APP_UI_EVENT_SPEAKER_START) {
            ESP_LOGI(TAG, "Start Speaker");
            init_loudspeaker();
        }
        if (app_ui_event & APP_UI_EVENT_CHANGE_SPEAKER) {
            ESP_LOGI(TAG, "Change Speaker");
            audio_change();
        }
        if (app_ui_event & APP_UI_EVENT_SPEAKER_STOP) {
            ESP_LOGI(TAG, "Speaker stop");
            audio_stop();
        }
        if (app_ui_event & APP_UI_EVENT_MIC_START) {
            ESP_LOGI(TAG, "Init Microphone");
            init_mic();
        }
        if (app_ui_event & APP_UI_EVENT_CHANGE_MIC) {
            ESP_LOGI(TAG, "Change Microphone");
            change_mic();
            ESP_LOGI(TAG, "Recording is finished");
            lv_obj_add_flag(ui_Button2, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_add_flag(ui_Image3, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_add_flag(ui_Arc1, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_clear_flag(ui_MicPanel, LV_OBJ_FLAG_HIDDEN); 
        }
        if(app_ui_event & APP_UI_EVENT_SENSOR_BME680_TEST) {
            bme680_data_t data;
            bme680_get_data(&data); 
            if (sensor_bme680_test(&data) == true) {
                flash_write_state("sensor1","pass");
            } else {
                flash_write_state("sensor1","fail");
            }
        }
        if (app_ui_event & APP_UI_EVENT_SENSOR_BMI270_BMM350_TEST) {
            sensor_9axis_data_t data;
            if (get_sensors_initialized() == false) {
                if (start_bmi270_bmm350_new() == ESP_OK) {
                    change_sensors_initialized(true);
                }
            }
            while (get_at_sensor2_screen()) {
                if (get_sensors_initialized()) {
                    lv_label_set_text(ui_Sensor2Tip, "查看传感器读数是否正常");
                    if (bmi270_bmm350_get_data(&data) == ESP_OK) {
                        ESP_LOGI(TAG, "Acc: %.2f %.2f %.2f", data.acc_x, data.acc_y, data.acc_z);
                        ESP_LOGI(TAG, "Gyro: %.2f %.2f %.2f", data.gyro_x, data.gyro_y, data.gyro_z);
                        ESP_LOGI(TAG, "Mag: %.2f %.2f %.2f", data.mag_x, data.mag_y, data.mag_z);
                    }
                } else {
                    lv_label_set_text(ui_Sensor2Tip, "传感器初始化失败");
                    data.acc_x = 0.0f;
                    data.acc_y = 0.0f;
                    data.acc_z = 0.0f;
                    data.gyro_x = 0.0f;
                    data.gyro_y = 0.0f;
                    data.gyro_z = 0.0f;
                    data.mag_x = 0.0f;
                    data.mag_y = 0.0f;
                    data.mag_z = 0.0f;
                }
                if (sensor_bmi270_bmm350_test(&data) == true) {
                    flash_write_state("sensor2","pass");
                } else {
                    flash_write_state("sensor2","fail");
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
    }
}
