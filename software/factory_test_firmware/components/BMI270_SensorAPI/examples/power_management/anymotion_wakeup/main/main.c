#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "bmi270.h"
#include "common/common.h"

static const char *TAG = "DEEPSLEEP_WAKEUP";

#if CONFIG_IDF_TARGET_ESP32S3
    #define POWER_CTRL_GPIO  GPIO_NUM_4
    #define BOOT_PIN         GPIO_NUM_0   // BOOT button (pull-up, low when pressed)
    #define KEY_PIN          GPIO_NUM_12  // User button (pull-down, high when pressed)
    #define LED_PIN          GPIO_NUM_11  // LED indicator
    #define VBAT_ADC_CHANNEL ADC_CHANNEL_9  // IO10
    #define IMU_INT_PIN      GPIO_NUM_5     // S3 IMU interrupt
    #define I2C_MASTER_SCL_IO       1
    #define I2C_MASTER_SDA_IO       2
#elif CONFIG_IDF_TARGET_ESP32C5
    #define POWER_CTRL_GPIO  GPIO_NUM_2
    #define BOOT_PIN         GPIO_NUM_28  // BOOT button (pull-up, low when pressed)
    #define KEY_PIN          GPIO_NUM_5   // User button (pull-down, high when pressed)
    #define LED_PIN          GPIO_NUM_27  // LED indicator
    #define VBAT_ADC_CHANNEL ADC_CHANNEL_3  // IO4
    #define IMU_INT_PIN      GPIO_NUM_3     // C5 IMU interrupt
    #define I2C_MASTER_SCL_IO       26
    #define I2C_MASTER_SDA_IO       25
#endif

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle;
static bool do_calibration = false;

static void init_adc() {
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, VBAT_ADC_CHANNEL, &chan_config));

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle);
    if (ret == ESP_OK) {
        do_calibration = true;
        ESP_LOGI(TAG, "ADC calibration succeeded");
    }
#endif
}

static int read_vbat_voltage() {
    int raw_value = 0, voltage = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, VBAT_ADC_CHANNEL, &raw_value));
    if (do_calibration) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, raw_value, &voltage));
        voltage = voltage * 3 / 2;
        ESP_LOGI(TAG, "Battery voltage: %d mV", voltage);
    } else {
        ESP_LOGI(TAG, "ADC Raw: %d", raw_value);
    }
    return voltage;
}


static bmi270_handle_t bmi_handle = NULL;
static i2c_bus_handle_t i2c_bus = NULL;

// BMI270 initialization function
static void i2c_sensor_bmi270_init(void)
{
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    i2c_bus = i2c_bus_create(I2C_NUM_0, &i2c_conf);

    bmi270_i2c_config_t bmi_conf = {
        .i2c_handle = i2c_bus,
        .i2c_addr = BMI270_I2C_ADDRESS,
    };
    bmi270_sensor_create(&bmi_conf, &bmi_handle);
}

// Configure BMI270 any motion interrupt
static void bmi270_configure_any_motion(void)
{
    uint8_t sensors[] = {BMI2_ACCEL, BMI2_ANY_MOTION};
    bmi270_sensor_enable(sensors, 2, bmi_handle);

    struct bmi2_sens_config config = {.type = BMI2_ANY_MOTION};
    bmi270_get_sensor_config(&config, 1, bmi_handle);
    config.cfg.any_motion.duration = 4;  // 80ms
    config.cfg.any_motion.threshold = 0x68;  // ~50mg
    bmi270_set_sensor_config(&config, 1, bmi_handle);

    struct bmi2_int_pin_config pin_config = {
        .pin_type = BMI2_INT1,
        .pin_cfg[0] = {
            .lvl = BMI2_INT_ACTIVE_HIGH,
            .output_en = BMI2_INT_OUTPUT_ENABLE,
            .od = BMI2_INT_PUSH_PULL,
            .input_en = BMI2_INT_INPUT_DISABLE,
        },
        .int_latch = BMI2_INT_NON_LATCH,
    };
    bmi2_set_int_pin_config(&pin_config, bmi_handle);

    struct bmi2_sens_int_config int_config = {
        .type = BMI2_ANY_MOTION,
        .hw_int_pin = BMI2_INT1
    };
    bmi270_map_feat_int(&int_config, 1, bmi_handle);
}

void configure_gpio_for_deepsleep() {
    ESP_LOGI(TAG, "Configuring GPIOs for deep sleep wakeup...");

    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << KEY_PIN) | (1ULL << IMU_INT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&input_conf);

    gpio_config_t boot_conf = {
        .pin_bit_mask = (1ULL << BOOT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&boot_conf);

    gpio_config_t power_conf = {
        .pin_bit_mask = (1ULL << POWER_CTRL_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&power_conf);
    gpio_set_level(POWER_CTRL_GPIO, 1);
    gpio_hold_en(POWER_CTRL_GPIO);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_conf);
    gpio_set_level(LED_PIN, 1);

    ESP_LOGI(TAG, "GPIOs configured.");
}

void enter_deep_sleep() {
    ESP_LOGI(TAG, "Setting up wakeup sources...");

    esp_sleep_enable_ext1_wakeup((1ULL << KEY_PIN) | (1ULL << IMU_INT_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_ext0_wakeup(BOOT_PIN, 0);

    ESP_LOGI(TAG, "Entering deep sleep...");
    esp_deep_sleep_start();
}

void main_task() {
    TickType_t last_activity = xTaskGetTickCount();
    const TickType_t sleep_timeout = pdMS_TO_TICKS(10000);

    while (1)
    {
        if (gpio_get_level(BOOT_PIN) == 0) {
            ESP_LOGI(TAG, "BOOT pressed");
            gpio_set_level(LED_PIN, 0);
        }

        if (gpio_get_level(KEY_PIN) == 1) {
            ESP_LOGI(TAG, "KEY pressed");
            gpio_set_level(LED_PIN, 1);
            last_activity = xTaskGetTickCount();
        }

        if (gpio_get_level(IMU_INT_PIN) == 1) {
            ESP_LOGI(TAG, "IMU motion detected");
            gpio_set_level(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_set_level(LED_PIN, 1);
            last_activity = xTaskGetTickCount();
        }

        if ((xTaskGetTickCount() - last_activity) > sleep_timeout) {
            ESP_LOGI(TAG, "No activity, going to sleep");
            enter_deep_sleep();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main() {
    ESP_LOGI(TAG, "System starting...");

    configure_gpio_for_deepsleep();
    i2c_sensor_bmi270_init();
    bmi270_configure_any_motion();

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wake_pin_mask = esp_sleep_get_ext1_wakeup_status();
        if (wake_pin_mask & (1ULL << KEY_PIN)) {
            ESP_LOGI(TAG, "Wakeup from KEY_PIN");
        }
        if (wake_pin_mask & (1ULL << IMU_INT_PIN)) {
            ESP_LOGI(TAG, "Wakeup from IMU_INT_PIN");
        }
    }
    else if (cause == ESP_SLEEP_WAKEUP_EXT0) {
        ESP_LOGI(TAG, "Wakeup from EXT0");
    }

    xTaskCreate(main_task, "button_led_task", 2048, NULL, 5, NULL);
}
