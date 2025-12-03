idf_component_register(
    SRCS "bmi270_bmm350.c" "../BMM350_SensorAPI/examples/common/common.c"
    INCLUDE_DIRS "include" "../BMM350_SensorAPI/examples/common"
    REQUIRES espressif__i2c_bus BMM350_SensorAPI bsp_esp_halo
    )

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})