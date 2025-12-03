idf_component_register(SRCS "bme690.c"
                    INCLUDE_DIRS "include"
                    REQUIRES BME690_SensorAPI driver esp_common bsp_esp_halo
                    )
