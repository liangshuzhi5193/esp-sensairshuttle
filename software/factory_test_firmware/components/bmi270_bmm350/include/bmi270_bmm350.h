#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"

typedef struct {
    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float mag_x;
    float mag_y;
    float mag_z;
} sensor_9axis_data_t;

extern bool sensors_initialized;

void print_9axis_sensor_data(void);
esp_err_t start_bmi270_bmm350_new(void);
esp_err_t bmi270_bmm350_get_data(sensor_9axis_data_t *data);
void stop_bmi270_bmm350(void);

#ifdef __cplusplus
}
#endif
