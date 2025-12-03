# BMI270 Triple Tap Detection Example
#
# This example demonstrates how to use the BMI270 Circle sensor for triple tap detection
# on ESP32 platforms. It shows how to initialize the BMI270 sensor, configure it for
# triple tap detection, and handle interrupts using ESP32 GPIO.
#
# Copyright (c) 2025 Bosch Sensortec GmbH. All rights reserved.

cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/tools/unit-test-app/components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(main)