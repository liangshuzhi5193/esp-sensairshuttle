idf_component_register(SRCS "audio_test.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bsp_esp_halo  # 添加这行
                    )

spiffs_create_partition_assets(
    audio
    "audio"
    FLASH_IN_PROJECT
    MMAP_FILE_SUPPORT_FORMAT ".wav"
)
