#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature;
    float pressure;
    float humidity;
    float gas_resistance;
} bme690_data_t;

void bme690_test(void);
void bme690_get_data(bme690_data_t *received_data);

#ifdef __cplusplus
}
#endif

