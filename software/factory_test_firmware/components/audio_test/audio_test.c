/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "bsp_esp_halo.h"
#include "iot_button.h"
#include "button_gpio.h"
#include "audio_test.h"
#include "mmap_generate_audio.h"    

#define CHUNK_SIZE 4096

/** 
 * @brief Audio player state
 */
typedef enum {
    PLAYER_STATE_IDLE = 0,    /** Player is idle */
    PLAYER_STATE_PLAYING,     /** Player is playing */
    PLAYER_STATE_PAUSED,      /** Player is paused */
} player_state_t;

static const char *TAG = "music_player";
static mmap_assets_handle_t asset_audio = NULL;
static SemaphoreHandle_t audio_sem = NULL;
static SemaphoreHandle_t record_done_sem;

static player_state_t player_state = PLAYER_STATE_IDLE;
static uint32_t current_audio_index = 0;
static bool should_play_next = false;

#define SAMPLE_RATE 16000
#define MAX_RECORD_TIME (3.0f)  // Maximum recording time in seconds

static TaskHandle_t mic_play_task_handle = NULL;
static TaskHandle_t audio_record_task_handle = NULL;
static bool is_recording = false;
static uint32_t recorded_samples = 0;
static esp_codec_dev_handle_t spk_codec_dev = NULL;

static void audio_mmap_init()
{
    const mmap_assets_config_t config = {
        .partition_label = "audio",
        .max_files = MMAP_AUDIO_FILES,
        .checksum = MMAP_AUDIO_CHECKSUM,
        .flags = {
            .mmap_enable = true,
            .app_bin_check = true,
        },
    };

    mmap_assets_new(&config, &asset_audio);
    ESP_LOGI(TAG, "stored_files:%d", mmap_assets_get_stored_files(asset_audio));
}

static void play_audio_from_mmap(void *asset, uint32_t mmap_id, esp_codec_dev_handle_t codec_dev)
{
    void *audio = (void *)mmap_assets_get_mem(asset, mmap_id);
    uint32_t len = mmap_assets_get_size(asset, mmap_id);

    uint8_t *buf = (uint8_t *)malloc(CHUNK_SIZE);
    if (!buf) {
        ESP_LOGE(TAG, "Failed to malloc audio buffer");
        return;
    }

    uint8_t *p = (uint8_t *)audio;
    uint32_t remaining = len;

    while (remaining > 0 && player_state == PLAYER_STATE_PLAYING) {
        uint32_t copy_len = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
        memcpy(buf, p, copy_len);
        esp_codec_dev_write(codec_dev, buf, copy_len);
        p += copy_len;
        remaining -= copy_len;
    }

    free(buf);
    
    // Set state to PAUSED after playback completes
    if (player_state == PLAYER_STATE_PLAYING) {
        player_state = PLAYER_STATE_PAUSED;
        ESP_LOGI(TAG, "Audio playback completed, state changed to PAUSED");
    }
}

static void mic_play_task(void *arg)
{
    uint8_t *audio_buffer = (uint8_t *)arg;
    uint32_t ulNotificationValue;
    while (1) {
        // Wait for notification to start playback
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Start playback, recorded samples: %ld", recorded_samples);
            // Play recorded audio            
            esp_codec_dev_write(spk_codec_dev, audio_buffer, recorded_samples * sizeof(uint16_t));
            ESP_LOGI(TAG, "Playback finished");
            // Notify main task that playback is done
            xSemaphoreGive(record_done_sem);
        }
    }

    vTaskDelete(NULL);
}

static void audio_record_task(void *arg)
{
    uint8_t *audio_buffer = (uint8_t *)arg;
    uint32_t ulNotificationValue;
    esp_codec_dev_handle_t mic_codec_dev = bsp_audio_codec_microphone_init();
    if (mic_codec_dev == NULL) {
        ESP_LOGE(TAG, "Failed to init microphone codec");
        vTaskDelete(NULL);
        return;
    }
    esp_codec_dev_sample_info_t mic_fs = {
        .sample_rate = SAMPLE_RATE,
        .channel = 2,
        .bits_per_sample = 16,
    };
    esp_codec_dev_open(mic_codec_dev, &mic_fs);
    
    while(1) {
        // Wait for notification to start recording
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Start recording");
            recorded_samples = 0;

            // Record until button is released or max time reached
            while (is_recording && recorded_samples < (MAX_RECORD_TIME * SAMPLE_RATE)) {
                // Read one second of audio at a time
                uint32_t samples_to_read = SAMPLE_RATE / 10;  //every 100ms
                if (recorded_samples + samples_to_read > (MAX_RECORD_TIME * SAMPLE_RATE)) {
                    samples_to_read = (MAX_RECORD_TIME * SAMPLE_RATE) - recorded_samples;
                }
                
                esp_codec_dev_read(mic_codec_dev, 
                                 audio_buffer + (recorded_samples * sizeof(uint16_t)), 
                                 samples_to_read * sizeof(uint16_t));
                recorded_samples += samples_to_read;
                
                ESP_LOGI(TAG, "Recorded %ld samples", recorded_samples);
            }
            
            ESP_LOGI(TAG, "Recording finished, total samples: %ld", recorded_samples);
            // Notify main task that recording is done
            is_recording = false;
            xTaskNotify(mic_play_task_handle, 0x01, eSetBits);
        }
    }
}

static void audio_play_task(void *arg)
{
    audio_mmap_init();

    spk_codec_dev = bsp_audio_codec_speaker_init();

    esp_codec_dev_sample_info_t fs = {
        .sample_rate        = 16000,
        .channel            = 1,
        .channel_mask       = 0,
        .bits_per_sample    = 16,
        .mclk_multiple      = 0,
    };

    esp_codec_dev_open(spk_codec_dev, &fs);
    esp_codec_dev_set_out_vol(spk_codec_dev, 60);
    if(AUDIO_PA_CTL_GPIO != GPIO_NUM_NC) {
        gpio_set_level(AUDIO_PA_CTL_GPIO, 1);
    }

    ESP_LOGI(TAG, "================ Starting audio loop ================");

    while (1) {
        // Wait for semaphore from button callback
        if (xSemaphoreTake(audio_sem, portMAX_DELAY) == pdTRUE) {
            switch (player_state) {
                case PLAYER_STATE_PLAYING:
                    if (should_play_next) {
                        current_audio_index = (current_audio_index + 1) % MMAP_AUDIO_FILES;
                        ESP_LOGI(TAG, "Playing next audio file: %ld", current_audio_index);
                    }
                    ESP_LOGI(TAG, "Playing audio file: %ld", current_audio_index);
                    play_audio_from_mmap(asset_audio, current_audio_index, spk_codec_dev);
                    break;
                    
                case PLAYER_STATE_PAUSED:
                    ESP_LOGI(TAG, "Audio paused");
                    break;
                    
                default:
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}

void audio_test_play_music(void)
{
    audio_sem = xSemaphoreCreateBinary();
    if (audio_sem == NULL) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return;
    }
    uint8_t *audio_buffer = malloc(MAX_RECORD_TIME * SAMPLE_RATE * sizeof(uint16_t));
    if (audio_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate audio buffer");
        vTaskDelete(NULL);
        return;
    }
    record_done_sem = xSemaphoreCreateBinary();

    xTaskCreate(audio_play_task, "audio_play_task", 1024 * 5, NULL, 15, NULL);
    xTaskCreate(mic_play_task, "mic_play_task", 1024 * 5, audio_buffer, 15, &mic_play_task_handle);
    xTaskCreate(audio_record_task, "audio_record_task", 1024 * 5, audio_buffer, 10, &audio_record_task_handle);
}

void audio_change(){
    switch (player_state) {
        case PLAYER_STATE_IDLE:
            player_state = PLAYER_STATE_PLAYING;
            should_play_next = false;
            break;
            
        case PLAYER_STATE_PLAYING:
            player_state = PLAYER_STATE_PAUSED;
            break;
            
        case PLAYER_STATE_PAUSED:
            player_state = PLAYER_STATE_PLAYING;
            should_play_next = true;
            break;
    }
    xSemaphoreGive(audio_sem);
}

void audio_stop(void)
{   
    player_state = PLAYER_STATE_IDLE;
    should_play_next = false;
    xSemaphoreGive(audio_sem);
}

void init_mic()
{
    esp_codec_dev_set_out_vol(spk_codec_dev, 80);
}

void change_mic()
{   
    if (is_recording==false) {
        is_recording = true;
        recorded_samples = 0;
        xTaskNotify(audio_record_task_handle, 0x01, eSetBits);
    }
    if (xSemaphoreTake(record_done_sem, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI(TAG, "Recording is done, returning from change_mic()");
    }
    is_recording = false;
}

void init_loudspeaker()
{
    esp_codec_dev_set_out_vol(spk_codec_dev, 60);
}