/**
* Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bmi270_toy.h
* @date       2024-10-28
* @version    v2.108.0
*
*/

/* BMI270_toy offers specialized toy motion detection features */

/**
 * \ingroup bmi2xy
 * \defgroup bmi270_toy BMI270_TOY
 * @brief Sensor driver for BMI270_TOY sensor
 * 
 * This header file provides the interface definitions for the BMI270 Toy sensor driver.
 * The BMI270 Toy is a specialized version of the BMI270 sensor that includes
 * dedicated algorithms for toy motion detection, shake detection, rotation angle measurement,
 * and other interactive toy features.
 * 
 * Key features:
 * - 3-axis accelerometer and gyroscope
 * - Dedicated toy motion detection algorithms
 * - Shake detection with configurable sensitivity
 * - Rotation angle measurement
 * - Push detection for interactive toys
 * - Rolling motion detection
 * - Low-power operation modes
 * - Configurable interrupt generation
 * - I2C and SPI communication interfaces
 * 
 * @note This driver provides specialized APIs for toy motion detection
 *       with optimized algorithms and improved accuracy for interactive toy applications.
 */

#ifndef BMI270_TOY_H_
#define BMI270_TOY_H_

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

/*!             Header files
 ****************************************************************************/
#include "bmi2.h"
#include "i2c_bus.h"
#include "esp_err.h"

/***************************************************************************/

/*!               Macro definitions
 ****************************************************************************/

/*! @name BMI270 Toy Chip identifier */
#define BMI270_TOY_CHIP_ID                   UINT8_C(0x24)
#define BMI270_I2C_ADDRESS                   UINT8_C(0x68)

/*! @name BMI270 feature input start addresses */
#define BMI270_TOY_ROTATION_STRT_ADDR           UINT8_C(0x06)
#define BMI270_TOY_ANY_MOTION_STRT_ADDR         UINT8_C(0x0C)
#define BMI270_TOY_MULTI_TAP_STRT_ADDR          UINT8_C(0x00)
#define BMI270_TOY_LOW_G_STRT_ADDR              UINT8_C(0x0A)
#define BMI270_TOY_HIGH_G_STRT_ADDR             UINT8_C(0x00)
#define BMI270_TOY_MOTION_STRT_ADDR             UINT8_C(0x06)
#define BMI270_TOY_NO_MOTION_STRT_ADDR          UINT8_C(0x08)
#define BMI270_TOY_ROLLING_STRT_ADDR            UINT8_C(0x0C)
#define BMI270_TOY_PUSH_STRT_ADDR               UINT8_C(0x0E)
#define BMI270_TOY_GI_INS1_STRT_ADDR            UINT8_C(0x00)
#define BMI270_TOY_GI_INS2_STRT_ADDR            UINT8_C(0x00)
#define BMI270_TOY_SHAKE_STRT_ADDR              UINT8_C(0x00)


/*! @name BMI270 feature output start addresses */
#define BMI270_TOY_ROTATION_ANGLE_OUT_STRT_ADDR     UINT8_C(0x04)
#define BMI270_TOY_HIGH_G_OUT_STRT_ADDR             UINT8_C(0x08)


/*! @name Defines maximum number of pages */
#define BMI270_TOY_MAX_PAGE_NUM             UINT8_C(8)

/*! @name Defines maximum number of feature input configurations */
#define BMI270_TOY_MAX_FEAT_IN              UINT8_C(17)

/*! @name Defines maximum number of feature outputs */
#define BMI270_TOY_MAX_FEAT_OUT             UINT8_C(7)


/*! @name Mask definitions for feature interrupt mapping bits */
#define BMI270_TOY_INT_TOY_MOTION_MASK      UINT8_C(0x01)
#define BMI270_TOY_INT_HIGH_LOW_G_MASK      UINT8_C(0x02)
#define BMI270_TOY_INT_PUSH_MASK            UINT8_C(0x04)
#define BMI270_TOY_INT_TAP_MASK             UINT8_C(0x08)
#define BMI270_TOY_INT_GI_INS1_ROLLING_MASK UINT8_C(0x10)
#define BMI270_TOY_INT_NO_MOT_MASK          UINT8_C(0x20)
#define BMI270_TOY_INT_ANY_MOT_MASK         UINT8_C(0x40)
#define BMI270_TOY_INT_SHAKE_MASK           UINT8_C(0x80)

/*! @name Defines maximum number of feature interrupts */
#define BMI270_TOY_MAX_INT_MAP              UINT8_C(8)

/***************************************************************************/

/*!     BMI270_TOY User Interface function prototypes
 ****************************************************************************/

/**
 * @brief BMI270 I2C configuration structure
 *
 * This structure contains the configuration parameters for communicating with
 * the BMI270 sensor over I2C. It specifies the I2C port and the I2C address of
 * the BMI270 device.
 */
typedef struct {
    i2c_bus_handle_t i2c_handle;    /*!< I2C port used to connect to the BMI270 device */
    uint8_t     i2c_addr;           /*!< I2C address of the BMI270 device, can be 0x38 or 0x39 depending on the A0 pin */
} bmi270_toy_i2c_config_t;

/**
 * @brief Handle type for BMI270 sensor
 *
 * This is a pointer to a structure representing the BMI270 device. It is used
 * as a handle for interacting with the sensor.
 */
typedef struct bmi2_dev * bmi270_toy_handle_t;

/**
 * @brief Create and initialize a BMI270 sensor object
 *
 * This function initializes the BMI270 sensor and prepares it for use.
 * It configures the I2C interface and creates a handle for further communication.
 *
 * @param[in] i2c_conf Pointer to the I2C configuration structure
 * @param[out] handle_ret Pointer to a variable that will hold the created sensor handle
 * @return
 *      - ESP_OK: Successfully created the sensor object
 *      - ESP_ERR_INVALID_ARG: Invalid arguments were provided
 *      - ESP_FAIL: Failed to initialize the sensor
 */
esp_err_t bmi270_toy_sensor_create(const bmi270_toy_i2c_config_t *i2c_conf, bmi270_toy_handle_t *handle_ret);

/**
 * @brief Delete and release a BMI270 sensor object
 *
 * This function releases the resources allocated for the BMI270 sensor.
 * It should be called when the sensor is no longer needed.
 *
 * @param[in] handle Handle of the BMI270 sensor object
 * @return
 *      - ESP_OK: Successfully deleted the sensor object
 *      - ESP_ERR_INVALID_ARG: Invalid handle was provided
 *      - ESP_FAIL: Failed to delete the sensor object
 */
esp_err_t bmi270_toy_sensor_del(bmi270_toy_handle_t handle);

/**
 * \ingroup bmi270_toy
 * \defgroup bmi270_toyApiInit Initialization
 * @brief Initialize the sensor and device structure
 */

/*!
 * \ingroup bmi270_toyApiInit
 * \page bmi270_toy_api_bmi270_toy_init bmi270_toy_init
 * \code
 * int8_t bmi270_toy_init(struct bmi2_dev *dev);
 * \endcode
 * @details This API:
 *  1) updates the device structure with address of the configuration file.
 *  2) Initializes BMI270_TOY sensor.
 *  3) Writes the configuration file.
 *  4) Updates the feature offset parameters in the device structure.
 *  5) Updates the maximum number of pages, in the device structure.
 *
 * @param[in, out] dev      : Structure instance of bmi2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_toy_init(struct bmi2_dev *dev);

/*!
 * \ingroup bmi270_toyApiInit
 * \page bmi270_toy_api_bmi270_toy_map_feature_int bmi270_toy_map_feature_int
 * \code
 * int8_t bmi270_toy_map_feature_int(struct bmi2_dev *dev);
 * \endcode
 * @details This API maps/unmaps feature interrupts to that of interrupt pins.
 */
int8_t bmi270_toy_map_feature_int(struct bmi2_dev *dev);


int8_t enable_toy_any_motion(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_no_motion(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_high_g(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_low_g(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_tap(struct bmi2_dev *bmi2_dev, uint8_t single_tap_enable, uint8_t double_tap_enable, uint8_t triple_tap_enable);
int8_t enable_toy_rolling(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_rotation_angle(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_motion(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_generic_interrupt_ins1(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_generic_interrupt_ins2(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_push(struct bmi2_dev *bmi2_dev, uint8_t enable);
int8_t enable_toy_shake(struct bmi2_dev *bmi2_dev, uint8_t enable);


int8_t get_toy_high_g_direction(struct bmi2_dev *bmi2_dev, char *direction);
int8_t get_toy_rotation_angle(struct bmi2_dev *bmi2_dev, uint8_t * output_status, float * output_angle);

/******************************************************************************/
/*! @name       C++ Guard Macros                                      */
/******************************************************************************/
#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif /* BMI270_TOY_H_ */
