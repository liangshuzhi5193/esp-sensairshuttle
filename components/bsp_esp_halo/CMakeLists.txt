idf_component_register(SRCS "bsp_esp_halo.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "priv_include"
                    REQUIRES esp_codec_dev espressif__button
                    PRIV_REQUIRES esp_timer esp_lcd 
                    )
