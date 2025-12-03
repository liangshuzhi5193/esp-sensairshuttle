idf_component_register(
    SRCS "bmi2.c" "bmi270.c" "common/common.c"
    INCLUDE_DIRS "." "common"
    REQUIRES driver i2c_bus
    PRIV_REQUIRES esp_common
)
