/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"
// Wi‑Fi / ESP‑NOW
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_check.h"
#include "esp_now.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_idf_version.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      8

#define EXAMPLE_LED_NUMBERS         25
#define EXAMPLE_CHASE_SPEED_MS      10
// ESPNOW channel must match sender
#define EXAMPLE_ESPNOW_CHANNEL      1

static const char *TAG = "example";

static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} example_rgb_msg_t;

static QueueHandle_t s_rgb_queue = NULL;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
static void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
#else
static void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
#endif
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    (void)recv_info;
#else
    (void)mac_addr;
#endif
    if (len < 3 || s_rgb_queue == NULL) {
        return;
    }
    example_rgb_msg_t msg = { .r = data[0], .g = data[1], .b = data[2] };
    (void)xQueueSend(s_rgb_queue, &msg, 0);
}

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
    ESP_ERROR_CHECK(esp_now_register_recv_cb(example_espnow_recv_cb));
    ESP_LOGI(TAG, "ESP-NOW ready on channel %d", EXAMPLE_ESPNOW_CHANNEL);
}

/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}
rmt_encoder_handle_t led_encoder = NULL;
rmt_channel_handle_t led_chan = NULL;

void led_set_color(uint32_t red, uint32_t green, uint32_t blue)
{
    for (int j = 0; j < EXAMPLE_LED_NUMBERS; j++) {
        led_strip_pixels[j * 3 + 0] = green;
        led_strip_pixels[j * 3 + 1] = blue;
        led_strip_pixels[j * 3 + 2] = red;
    }
    // Flush RGB values to LEDs
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}

void app_main(void)
{
    s_rgb_queue = xQueueCreate(8, sizeof(example_rgb_msg_t));
    if (s_rgb_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create RGB queue");
    }

    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    example_init_wifi_and_espnow();
    ESP_LOGI(TAG, "Waiting for RGB via ESP-NOW...");

        led_set_color(10, 10, 10);
            example_rgb_msg_t msg;
    while (1) {
        if (xQueueReceive(s_rgb_queue, &msg, portMAX_DELAY) == pdTRUE) {
            led_set_color(msg.r, msg.g, msg.b);
        }
    }
}
