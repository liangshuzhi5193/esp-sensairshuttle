file(GLOB_RECURSE SRC_UI ${CMAKE_SOURCE_DIR} "ui/*.c")

idf_component_register(SRCS "app_main.c" ${SRC_UI}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES bsp_esp_halo audio_test nvs_flash bme680 bmi270_bmm350 flash
                    )
