idf_component_register(
    SRCS "bsp_esp_bosch_sensor_hub.c"  ${SRC_VER}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver  esp_wifi
    PRIV_REQUIRES fatfs esp_lcd
)
