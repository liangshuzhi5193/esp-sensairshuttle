# BMI270 Triple Tap Detection Example - Main Component
#
# This component contains the main application code for the BMI270 triple tap
# detection example. It includes the main.c file and defines component dependencies.
#
# Copyright (c) 2025 Bosch Sensortec GmbH. All rights reserved.

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_netif esp_event
)

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})