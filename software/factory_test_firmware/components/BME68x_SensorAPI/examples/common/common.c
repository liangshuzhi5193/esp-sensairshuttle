/**
 * Copyright (C) 2023 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

#include "bme68x.h"
#include "common.h"

static const char *TAG = "BME68X_ESP32";

/* Static device state */
static uint8_t dev_addr;
static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t i2c_dev = NULL;

/* I2C read mapped to ESP-IDF */
BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    (void)intf_ptr;
    esp_err_t ret;

    if (i2c_dev == NULL) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return BME68X_E_COM_FAIL;
    }

    ret = i2c_master_transmit_receive(i2c_dev, &reg_addr, 1, reg_data, len, 50);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(ret));
        return BME68X_E_COM_FAIL;
    }

    return BME68X_OK;
}

/* I2C write mapped to ESP-IDF */
BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    (void)intf_ptr;
    esp_err_t ret;

    if (i2c_dev == NULL) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return BME68X_E_COM_FAIL;
    }

    uint8_t *buf = malloc(len + 1);
    if (!buf) {
        ESP_LOGE(TAG, "No memory");
        return BME68X_E_COM_FAIL;
    }
    buf[0] = reg_addr;
    memcpy(&buf[1], reg_data, len);

    ret = i2c_master_transmit(i2c_dev, buf, len + 1, 50);
    free(buf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(ret));
        return BME68X_E_COM_FAIL;
    }
    return BME68X_OK;
}

/* SPI is not implemented for ESP32 in this example */
BME68X_INTF_RET_TYPE bme68x_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    ESP_LOGE(TAG, "SPI interface not implemented for ESP32");
    return BME68X_E_COM_FAIL;
}

BME68X_INTF_RET_TYPE bme68x_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    ESP_LOGE(TAG, "SPI interface not implemented for ESP32");
    return BME68X_E_COM_FAIL;
}

void bme68x_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    if (period < 1000) {
        esp_rom_delay_us(period);
    } else {
        vTaskDelay(pdMS_TO_TICKS((period + 999) / 1000));
    }
}

void bme68x_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BME68X_OK:
            break;
        case BME68X_E_NULL_PTR:
            ESP_LOGE(TAG, "API [%s] Error [%d] : Null pointer", api_name, rslt);
            break;
        case BME68X_E_COM_FAIL:
            ESP_LOGE(TAG, "API [%s] Error [%d] : Communication failure", api_name, rslt);
            break;
        case BME68X_E_INVALID_LENGTH:
            ESP_LOGE(TAG, "API [%s] Error [%d] : Incorrect length parameter", api_name, rslt);
            break;
        case BME68X_E_DEV_NOT_FOUND:
            ESP_LOGE(TAG, "API [%s] Error [%d] : Device not found", api_name, rslt);
            break;
        case BME68X_E_SELF_TEST:
            ESP_LOGE(TAG, "API [%s] Error [%d] : Self test error", api_name, rslt);
            break;
        case BME68X_W_NO_NEW_DATA:
            ESP_LOGW(TAG, "API [%s] Warning [%d] : No new data found", api_name, rslt);
            break;
        default:
            ESP_LOGE(TAG, "API [%s] Error [%d] : Unknown error code", api_name, rslt);
            break;
    }
}

int8_t bme68x_interface_init(struct bme68x_dev *bme, uint8_t intf)
{
    if (bme == NULL) {
        return BME68X_E_NULL_PTR;
    }

    if (intf == BME68X_I2C_INTF) {
        ESP_LOGI(TAG, "I2C Interface, dev_addr=0x%02X", BME68X_I2C_ADDR_LOW);

        if (i2c_bus == NULL) {
            ESP_LOGE(TAG, "I2C bus handle is NULL, please call bme68x_set_i2c_bus_handle first");
            return BME68X_E_COM_FAIL;
        }

        dev_addr = BME68X_I2C_ADDR_LOW;
        bme->read = bme68x_i2c_read;
        bme->write = bme68x_i2c_write;
        bme->intf = BME68X_I2C_INTF;

        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_7,
            .device_address  = dev_addr,
            .scl_speed_hz    = 100000,
            .flags.disable_ack_check = 0
        };

        vTaskDelay(pdMS_TO_TICKS(10));

        esp_err_t esp_rslt = i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev);
        if (esp_rslt != ESP_OK) {
            ESP_LOGE(TAG, "i2c_master_bus_add_device failed: %s", esp_err_to_name(esp_rslt));
            return BME68X_E_COM_FAIL;
        }
        ESP_LOGI(TAG, "i2c_dev handle created: %p", i2c_dev);

        esp_rslt = i2c_master_probe(i2c_bus, dev_addr, 200);
        ESP_LOGI(TAG, "post-add probe(0x%02X) -> %s", dev_addr, esp_err_to_name(esp_rslt));
        if (esp_rslt != ESP_OK) {
            return BME68X_E_DEV_NOT_FOUND;
        }
    } else if (intf == BME68X_SPI_INTF) {
        ESP_LOGE(TAG, "SPI Interface not implemented for ESP32");
        return BME68X_E_COM_FAIL;
    }

    bme->intf_ptr = &dev_addr;
    bme->delay_us = bme68x_delay_us;
    bme->amb_temp = 25;
    return BME68X_OK;
}

void bme68x_set_i2c_bus_handle(i2c_master_bus_handle_t bus_handle)
{
    i2c_bus = bus_handle;
    ESP_LOGI(TAG, "I2C bus handle set");
}

void bme68x_coines_deinit(void)
{
    ESP_LOGI(TAG, "BME68X ESP32 deinit");
    if (i2c_dev) {
        i2c_master_bus_rm_device(i2c_dev);
        i2c_dev = NULL;
    }
    if (i2c_bus) {
        i2c_del_master_bus(i2c_bus);
        i2c_bus = NULL;
    }
}
