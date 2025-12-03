#include <stdio.h>
#include <stdbool.h>
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bmi270_toy.h"
#include "common/common.h"

#ifdef CONFIG_BOARD_ESP_SPOT_C5 //ESP-IDF v5.5-dev-1655-gc5865270b5
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

// I2C Configuration
#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      (100 * 1000)

static bmi270_toy_handle_t bmi_handle = NULL;
static i2c_bus_handle_t i2c_bus;
static volatile bool interrupt_status = false;

static void IRAM_ATTR gpio_isr_edge_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    interrupt_status = true;
    // esp_rom_printf("GPIO[%u] intr, val: %d\n", gpio_num, gpio_get_level(gpio_num));
}

static esp_err_t i2c_sensor_bmi270_toy_init(void)
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

    bmi270_toy_i2c_config_t i2c_bmi270_conf = {
        .i2c_handle = i2c_bus,
        .i2c_addr = BMI270_I2C_ADDRESS,
    };

    if (bmi270_toy_sensor_create(&i2c_bmi270_conf, &bmi_handle) != ESP_OK || bmi_handle == NULL) {
        ESP_LOGE("MAIN", "BMI270 TOY create failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static int8_t set_accel_gyro_config(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    struct bmi2_sens_config config[2];
    struct bmi2_int_pin_config pin_config = { 0 };

    config[BMI2_ACCEL].type = BMI2_ACCEL;
    config[BMI2_GYRO].type = BMI2_GYRO;

    rslt = bmi2_get_int_pin_config(&pin_config, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    rslt = bmi2_get_sensor_config(config, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        /* Set Output Data Rate */
        config[BMI2_ACCEL].cfg.acc.odr = BMI2_ACC_ODR_200HZ;
        config[BMI2_ACCEL].cfg.acc.range = BMI2_ACC_RANGE_16G;
        config[BMI2_ACCEL].cfg.acc.bwp = BMI2_ACC_NORMAL_AVG4;
        config[BMI2_ACCEL].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;

        config[BMI2_GYRO].cfg.gyr.odr = BMI2_GYR_ODR_200HZ;
        config[BMI2_GYRO].cfg.gyr.range = BMI2_GYR_RANGE_2000;
        config[BMI2_GYRO].cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;
        config[BMI2_GYRO].cfg.gyr.noise_perf = BMI2_POWER_OPT_MODE;
        config[BMI2_GYRO].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;

        /* Interrupt pin configuration */
        pin_config.pin_type = BMI2_INT1;
        pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
        pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_LOW;
        pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
        pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
        pin_config.int_latch = BMI2_INT_LATCH;

        rslt = bmi2_set_int_pin_config(&pin_config, bmi2_dev);
        bmi2_error_codes_print_result(rslt);

        rslt = bmi2_set_sensor_config(config, 2, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
    }

    return rslt;
}

static int8_t set_feature_interrupt(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    uint8_t data = BMI270_TOY_INT_PUSH_MASK | BMI270_TOY_INT_GI_INS1_ROLLING_MASK;
    struct bmi2_int_pin_config pin_config = { 0 };

    // Map push and rolling interrupt to INT1 (following toy_demo.c style)
    rslt = bmi2_set_regs(BMI2_INT1_MAP_FEAT_ADDR, &data, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        /* Interrupt pin configuration */
        pin_config.pin_type = BMI2_INT1;
        pin_config.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
        pin_config.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;
        pin_config.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
        pin_config.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
        pin_config.int_latch = BMI2_INT_LATCH;

        rslt = bmi2_set_int_pin_config(&pin_config, bmi2_dev);
        bmi2_error_codes_print_result(rslt);
    }

    return rslt;
}

static void adjust_high_g_threshold_for_on_table(struct bmi2_dev *bmi2_dev)
{
    uint8_t data[2], page;
    int8_t rslt;

    uint8_t aps_stat = bmi2_dev->aps_status;
    if (aps_stat == BMI2_ENABLE) {
        bmi2_set_adv_power_save(BMI2_DISABLE, bmi2_dev);
    }

    page = 4;
    rslt = bmi2_set_regs(0x2f, &page, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);
    
    // Decrease high g threshold for more sensitive detection
    uint16_t high_g_threshold = 0x0800;
    data[0] = high_g_threshold & 0xff;
    data[1] = (high_g_threshold >> 8) & 0x7f;
    rslt = bmi2_set_regs(0x30, data, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    if (aps_stat == BMI2_ENABLE) {
        bmi2_set_adv_power_save(BMI2_ENABLE, bmi2_dev);
    }
}

static void adjust_high_g_axis_by_rolling(struct bmi2_dev *bmi2_dev)
{
    uint8_t data[2], page;
    int8_t rslt;

    uint8_t aps_stat = bmi2_dev->aps_status;
    if (aps_stat == BMI2_ENABLE) {
        bmi2_set_adv_power_save(BMI2_DISABLE, bmi2_dev);
    }

    enable_toy_high_g(bmi2_dev, BMI2_DISABLE);
    enable_toy_push(bmi2_dev, BMI2_DISABLE);
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t axes;
    uint8_t select_x = 1;
    uint8_t select_y = 1;
    uint8_t select_z = 1;
    rslt = bmi2_get_regs(0x1e, &axes, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);
    axes = axes >> 5;
    if (axes == 1 || axes == 2) {
        select_x = 0;
    }
    else if (axes == 3 || axes == 4) {
        select_y = 0;
    }
    else if (axes == 5 || axes == 6) {
        select_z = 0;
    }

    uint8_t high_g_axis_selection = (select_x << 4) | (select_y << 5) | (select_z << 6);

    page = 4;
    rslt = bmi2_set_regs(0x2f, &page, 1, bmi2_dev);
    bmi2_error_codes_print_result(rslt);
    rslt = bmi2_get_regs(0x32, data, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);
    // set the axis of gravity to 0
    data[1] = (data[1] & 0x8f) | high_g_axis_selection;
    rslt = bmi2_set_regs(0x32, data, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    enable_toy_high_g(bmi2_dev, BMI2_ENABLE);
    enable_toy_push(bmi2_dev, BMI2_ENABLE);
    vTaskDelay(pdMS_TO_TICKS(20));

    if (aps_stat == BMI2_ENABLE) {
        bmi2_set_adv_power_save(BMI2_ENABLE, bmi2_dev);
    }
}

static int8_t bmi270_toy_enable_push_int(struct bmi2_dev *bmi2_dev)
{
    int8_t rslt;
    uint8_t sens_list[2] = { BMI2_ACCEL, BMI2_GYRO };
    uint8_t int_status = 0;

    // disable aps mode
    rslt = bmi2_set_adv_power_save(BMI2_DISABLE, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    // Set accel/gyro config
    rslt = set_accel_gyro_config(bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    // Set feature interrupt
    rslt = set_feature_interrupt(bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    // Enable sensors
    rslt = bmi2_sensor_enable(sens_list, 2, bmi2_dev);
    bmi2_error_codes_print_result(rslt);

    // Enable push feature
    rslt = enable_toy_push(bmi2_dev, BMI2_ENABLE);
    bmi2_error_codes_print_result(rslt);
    ESP_LOGI("MAIN", "Push feature enabled, result: %d", rslt);

    // Enable rolling to get the direction of gravity
    rslt = enable_toy_rolling(bmi2_dev, BMI2_ENABLE);
    bmi2_error_codes_print_result(rslt);
    ESP_LOGI("MAIN", "Rolling feature enabled, result: %d", rslt);

    // Delay 500ms to get initial direction
    vTaskDelay(pdMS_TO_TICKS(500));

    adjust_high_g_axis_by_rolling(bmi2_dev);
    adjust_high_g_threshold_for_on_table(bmi2_dev);

    if (rslt == BMI2_OK) {
        ESP_LOGI("MAIN", "Move the sensor to get push interrupt...");

        while (rslt == BMI2_OK) {
            if (interrupt_status) {
                interrupt_status = false;
                
                /* Clear buffer. */
                int_status = 0;

                rslt = bmi2_get_regs(BMI2_INT_STATUS_0_ADDR, &int_status, 1, bmi2_dev);
                bmi2_error_codes_print_result(rslt);

                if (int_status & BMI270_TOY_INT_PUSH_MASK) {
                    ESP_LOGI("MAIN", "Push generated, ");
                    char direction[10] = {0};
                    rslt = get_toy_high_g_direction(bmi2_dev, direction);
                    bmi2_error_codes_print_result(rslt);
                    ESP_LOGI("MAIN", "direction: %s", direction);
                }
                if (int_status & BMI270_TOY_INT_GI_INS1_ROLLING_MASK) {
                    ESP_LOGI("MAIN", "Rolling generated");
                    adjust_high_g_axis_by_rolling(bmi2_dev);
                }
            } else {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }

    return rslt;
}

void app_main(void) {
    // Print current configuration information
    ESP_LOGI("MAIN", "=== BMI270 TOY Push Detection Configuration ===");
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

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = (1ULL << I2C_INT_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(I2C_INT_IO, gpio_isr_edge_handler, (void*) I2C_INT_IO);

    if (i2c_sensor_bmi270_toy_init() != ESP_OK) {
        ESP_LOGE("MAIN", "BMI270 TOY initialization failed");
        return;
    }

    int8_t rslt = bmi270_toy_enable_push_int(bmi_handle);
    bmi2_error_codes_print_result(rslt);
    if (rslt != BMI2_OK) {
        ESP_LOGE("MAIN", "BMI270 TOY enable push interrupt failed");
    }

    gpio_isr_handler_remove(I2C_INT_IO);
    gpio_uninstall_isr_service();

    bmi270_toy_sensor_del(bmi_handle);
    i2c_bus_delete(&i2c_bus);

    ESP_LOGI("MAIN", "BMI270 TOY push interrupt example finished.");
}
