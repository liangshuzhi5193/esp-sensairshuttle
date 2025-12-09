cmake_minimum_required(VERSION 3.5)

# Set application version BEFORE including project.cmake
set(PROJECT_VER "V1.0.0")

# Set extra component dirs BEFORE including project.cmake
set(EXTRA_COMPONENT_DIRS "../../components")

# Minimal components
set(COMPONENTS main)

# Include ESP-IDF CMake system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project name
project(factory_test_firmware)

