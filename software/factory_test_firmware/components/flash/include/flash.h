#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define NVS_PART_NAME      "nvs"
#define NVS_PART_NAMESPACE "test_result"

void flash_write_init(void);
void flash_write_state(char* key, char* value);
uint8_t flash_read_state(char* key);

#ifdef __cplusplus
}
#endif

