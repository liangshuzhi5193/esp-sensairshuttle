idf_component_register(SRCS "led_strip_example_main.c" "led_strip_encoder.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi nvs_flash driver esp_netif esp_event)
