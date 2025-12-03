#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void bme680_test(void);
void bme680_get_data(float *temp, float *press, float *hum, float *gas);

#ifdef __cplusplus
}
#endif

