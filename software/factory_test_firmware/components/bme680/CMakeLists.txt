idf_component_register(SRCS "bme680.c"
                    INCLUDE_DIRS "include"
                    REQUIRES BME68x_SensorAPI driver esp_common bsp_esp_halo
                    )
