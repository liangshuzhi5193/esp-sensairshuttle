#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "bsp_esp_halo.h"
#include "common.h"
#include "bme680.h"
#include "bme68x.h"
#include "bme68x_defs.h"

#define SHUTTLE_V0_4_BOARD 1

#if SHUTTLE_V0_4_BOARD
    #define I2C_MASTER_SCL_IO       3 
    #define I2C_MASTER_SDA_IO       2
    #define I2C_MASTER_NUM          I2C_NUM_0
    #define I2C_MASTER_FREQ_HZ      (100 * 1000)

    #define BME680_SDO_PIN          9
#endif

static const char *TAG = "BME680_TEST";
static i2c_master_bus_handle_t i2c_bus = NULL;

static struct bme68x_dev bme;
static int8_t rslt;
static struct bme68x_conf conf;
static struct bme68x_heatr_conf heatr_conf;
static struct bme68x_data data;
static uint8_t n_data = 0;
static uint16_t sample_count = 1;

static bool bme_initialized = false;   // 避免重复初始化

static esp_err_t init_hardware(void)
{
    esp_err_t ret;

    /* SDO low -> 0x76 */
    gpio_config_t sdo_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << BME680_SDO_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    ret = gpio_config(&sdo_conf);
    if (ret != ESP_OK) return ret;
    gpio_set_level(BME680_SDO_PIN, 0);

    i2c_bus = bsp_i2c_get_bus_handle();
    return ESP_OK;
}

void bme680_test(void)
{
    ESP_LOGI(TAG, "=== BME680 ESP32 Test ===");

    if (init_hardware() != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization failed");
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    /* Set I2C bus handle */
    bme68x_set_i2c_bus_handle(i2c_bus);
    ESP_LOGI(TAG, "Scanning I2C bus...");

    ESP_LOGI(TAG, "Scan complete.");
    /* Interface/init */
    rslt = bme68x_interface_init(&bme, BME68X_I2C_INTF);
    bme68x_check_rslt("bme68x_interface_init", rslt);
    if (rslt != BME68X_OK) return;

    rslt = bme68x_init(&bme);
    bme68x_check_rslt("bme68x_init", rslt);
    if (rslt != BME68X_OK) return;
    ESP_LOGI(TAG, "Chip ID: 0x%02x", bme.chip_id);

    /* Config */
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_16X;
    conf.os_temp = BME68X_OS_16X;
    rslt = bme68x_set_conf(&conf, &bme);
    bme68x_check_rslt("bme68x_set_conf", rslt);
    if (rslt != BME68X_OK) return;

    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
    bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
    if (rslt != BME68X_OK) return;
    
    ESP_LOGI(TAG, "Sample, Temperature(deg C), Pressure(Pa), Humidity(%%), Gas resistance(ohm)");

    bme_initialized = true;
}

void bme680_get_data(bme680_data_t *received_data)
{
    if (!bme_initialized) {
        if (received_data) {
            received_data->temperature = -40.0f;
            received_data->pressure = 0.0f;
            received_data->humidity = 0.0f;
            received_data->gas_resistance = 0.0f;
        }
        return;
    }
    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);

    uint32_t del_us = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);
    bme.delay_us(del_us, bme.intf_ptr);

    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_data, &bme);
    bme68x_check_rslt("bme68x_get_data", rslt);

    if (n_data) {
        ESP_LOGI(TAG, "Reading #%d:", sample_count);
        ESP_LOGI(TAG, "  Temperature: %.2f °C", data.temperature);
        ESP_LOGI(TAG, "  Pressure: %.2f Pa", data.pressure);
        ESP_LOGI(TAG, "  Humidity: %.2f %%", data.humidity);
        ESP_LOGI(TAG, "  Gas Resistance: %.2f Ω", data.gas_resistance);
        sample_count++;
        if (received_data) {
            received_data->temperature = data.temperature;
            received_data->pressure = data.pressure;
            received_data->humidity = data.humidity;
            received_data->gas_resistance = data.gas_resistance;
        }
    } else {
        ESP_LOGW(TAG, "No valid data read from BME68X!");
        if (received_data) {
            received_data->temperature = -40.0f;
            received_data->pressure = 0.0f;
            received_data->humidity = 0.0f;
            received_data->gas_resistance = 0.0f;
        }
    }
}


