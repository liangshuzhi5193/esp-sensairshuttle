# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Add the root component directory to the component search path
set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/tools/unit-test-app/components" "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(main)