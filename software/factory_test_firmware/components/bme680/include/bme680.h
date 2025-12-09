#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature;
    float pressure;
    float humidity;
    float gas_resistance;
} bme680_data_t;

void bme680_test(void);
void bme680_get_data(bme680_data_t *received_data);

#ifdef __cplusplus
}
#endif

