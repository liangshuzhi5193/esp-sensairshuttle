#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "flash.h"

nvs_handle_t my_handle;

void flash_write_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void flash_write_state(char* key, char* value)
{
    nvs_open_from_partition(NVS_PART_NAME, NVS_PART_NAMESPACE, NVS_READWRITE, &my_handle);
    ESP_ERROR_CHECK(nvs_set_str(my_handle, key, value));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
}

uint8_t flash_read_state(char* key)
{
    nvs_open_from_partition(NVS_PART_NAME, NVS_PART_NAMESPACE, NVS_READWRITE, &my_handle);
    char test[10] = {0};
    size_t length = sizeof(test);
    esp_err_t err = nvs_get_str(my_handle, key, test, &length);
    nvs_close(my_handle);
    if (err != ESP_OK || length == 0) {
        return 2;
    } else {
        if ( !strcmp(test, "pass")) {
            return 1;
        } else if ( !strcmp(test, "fail")) {
            return 2;
        } else {
            return 0;
        }
    }
}
