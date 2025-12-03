/**
* Copyright (c) 2025  Bosch Sensortec GmbH. All rights reserved.
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
* @file       bmi270_circle.h
* @date       2025-06-23
* @version    v2.122.0
*
*/

/* BMI270_legacy offers same low-power features as in BMI160 */

/**
 * \ingroup bmi2xy
 * \defgroup bmi270_circle BMI270_CIRCLE
 * @brief Sensor driver for BMI270_CIRCLE sensor
 * 
 * This header file provides the interface definitions for the BMI270 Circle sensor driver.
 * The BMI270 Circle is a specialized version of the BMI270 sensor that includes
 * dedicated algorithms for circular gesture recognition and enhanced motion detection.
 * 
 * Key features:
 * - 3-axis accelerometer and gyroscope
 * - Dedicated circular gesture recognition algorithms
 * - Enhanced motion detection capabilities
 * - Low-power operation modes
 * - Configurable interrupt generation
 * - I2C and SPI communication interfaces
 * 
 * @note This driver provides specialized APIs for circular gesture detection
 *       with optimized algorithms and improved accuracy.
 */

#ifndef BMI270_CIRCLE_H_
#define BMI270_CIRCLE_H_

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

/*! @name BMI270_CIRCLE chip identifier */
#define BMI270_CIRCLE_CHIP_ID                         UINT8_C(0x24)
#define BMI270_I2C_ADDRESS                            UINT8_C(0x68)
/*! @name BMI270_CIRCLE feature input start addresses */
#define BMI270_CIRCLE_CONFIG_ID_STRT_ADDR             UINT8_C(0x00)
#define BMI270_CIRCLE_MAX_BURST_LEN_STRT_ADDR         UINT8_C(0x02)
#define BMI270_CIRCLE_CRT_GYRO_SELF_TEST_STRT_ADDR    UINT8_C(0x03)
#define BMI270_CIRCLE_ABORT_STRT_ADDR                 UINT8_C(0x03)
#define BMI270_CIRCLE_AXIS_MAP_STRT_ADDR              UINT8_C(0x04)
#define BMI270_CIRCLE_GYRO_SELF_OFF_STRT_ADDR         UINT8_C(0x05)
#define BMI270_CIRCLE_NVM_PROG_PREP_STRT_ADDR         UINT8_C(0x05)
#define BMI270_CIRCLE_ANY_MOT_STRT_ADDR               UINT8_C(0x0C)
#define BMI270_CIRCLE_NO_MOT_STRT_ADDR                UINT8_C(0x00)
#define BMI270_CIRCLE_CIRCLE_GEST_STRT_ADDR           UINT8_C(0x04)
#define BMI270_CIRCLE_GEN_INT_STRT_ADDR               UINT8_C(0x00)
#define BMI270_CIRCLE_GYRO_USERGAIN_UPDATE_STRT_ADDR  UINT8_C(0x06)
#define BMI270_CIRCLE_MULTITAP_DETECT_STRT_ADDR       UINT8_C(0x00)

/*! @name BMI270_CIRCLE feature output start addresses */
#define BMI270_CIRCLE_GYR_USER_GAIN_OUT_STRT_ADDR     UINT8_C(0x08)
#define BMI270_CIRCLE_GYRO_CROSS_SENSE_STRT_ADDR      UINT8_C(0x0C)
#define BMI270_CIRCLE_NVM_VFRM_OUT_STRT_ADDR          UINT8_C(0x0E)

/*! @name Defines maximum number of pages */
#define BMI270_CIRCLE_MAX_PAGE_NUM                    UINT8_C(5)

/*! @name Defines maximum number of feature input configurations */
#define BMI270_CIRCLE_MAX_FEAT_IN                     UINT8_C(23)

/*! @name Defines maximum number of feature outputs */
#define BMI270_CIRCLE_MAX_FEAT_OUT                    UINT8_C(8)

/*! @name Mask definitions for feature interrupt status bits */
#define BMI270_CIRCLE_CIRCLE_GEST_DIR_STATUS_MASK     UINT8_C(0x07)
#define BMI270_CIRCLE_CIRCLE_GEST_DETECT_STATUS_MASK  UINT8_C(0x08)
#define BMI270_CIRCLE_SINGLE_TAP_MASK                 UINT8_C(0x01)
#define BMI270_CIRCLE_DOUBLE_TAP_MASK                 UINT8_C(0x02)
#define BMI270_CIRCLE_TRIPLE_TAP_MASK                 UINT8_C(0x04)
#define BMI270_CIRCLE_MESSAGE_MASK                    UINT8_C(0x0F)
#define BMI270_CIRCLE_AXIS_REMAP_ERR_MASK             UINT8_C(0x20)
#define BMI270_CIRCLE_ODR_50HZ_ERR_MASK               UINT8_C(0x40)
#define BMI270_CIRCLE_ODR_HIGH_ERR_MASK               UINT8_C(0x80)

/*! @name Status register for tap */
#define BMI270_CIRCLE_TAP_STATUS_REG                  UINT8_C(0x20)

/*! @name Status register for circle gesture */
#define BMI270_CIRCLE_CIRCLE_GES_STATUS_REG           UINT8_C(0x1E)

/*! @name Internal status register */
#define BMI270_CIRCLE_CIRCLE_INTERNAL_STATUS_REG      UINT8_C(0x21)

/*! @name Mask definitions for feature interrupt mapping bits */
#define BMI270_CIRCLE_CIRCLE_GESTURE_MASK             UINT8_C(0x01)
#define BMI270_CIRCLE_GEN_INT_MASK                    UINT8_C(0x02)
#define BMI270_CIRCLE_TAP_MASK                        UINT8_C(0x08)
#define BMI270_CIRCLE_SINGLETAP_MASK                  UINT8_C(0x08)
#define BMI270_CIRCLE_DOUBLETAP_MASK                  UINT8_C(0x08)
#define BMI270_CIRCLE_TRIPLETAP_MASK                  UINT8_C(0x08)
#define BMI270_CIRCLE_NO_MOT_MASK                     UINT8_C(0x20)
#define BMI270_CIRCLE_ANY_MOT_MASK                    UINT8_C(0x40)

/*! @name Defines maximum number of feature interrupts */
#define BMI270_CIRCLE_MAX_INT_MAP                     UINT8_C(8)

/*! @name Mask definitions for BMI2 circle gesture detection feature configuration */
#define BMI270_CIRCLE_GEST_DET_EN_MASK                UINT16_C(0x0001)
#define BMI270_CIRCLE_GEST_DET_AXIS_SEL_MASK          UINT16_C(0x0003)
#define BMI270_CIRCLE_GEST_DET_THRES_MASK             UINT16_C(0x1FFC)
#define BMI270_CIRCLE_GEST_DET_THRES_DET_MASK         UINT16_C(0x0FFF)
#define BMI270_CIRCLE_GEST_DET_WAIT_TIMEOUT_MASK      UINT16_C(0x1000)
#define BMI270_CIRCLE_GEST_DET_MIN_GEST_DET_MASK      UINT16_C(0x000F)
#define BMI270_CIRCLE_GEST_DET_MAX_GEST_DET_MASK      UINT16_C(0x01F0)
#define BMI270_CIRCLE_GEST_DET_QUITE_TIME_MASK        UINT16_C(0X3E00)

/*! @name Bit position definition for BMI2 circle configuration for circle gesture detector variant */
#define BMI270_CIRCLE_GEST_DET_EN_POS                 UINT16_C(0x0001)
#define BMI270_CIRCLE_GEST_DET_AXIS_SEL_POS           UINT16_C(0x0000)
#define BMI270_CIRCLE_GEST_DET_THRES_POS              UINT16_C(0x0002)
#define BMI270_CIRCLE_GEST_DET_THRES_DET_POS          UINT16_C(0x0000)
#define BMI270_CIRCLE_GEST_DET_WAIT_TIMEOUT_POS       UINT16_C(0x000C)
#define BMI270_CIRCLE_GEST_DET_MIN_GEST_DET_POS       UINT16_C(0x0000)
#define BMI270_CIRCLE_GEST_DET_MAX_GEST_DET_POS       UINT16_C(0x0004)
#define BMI270_CIRCLE_GEST_DET_QUITE_TIME_POS         UINT16_C(0X0009)

/*! @name Macros for circle gesture directions */
#define BMI270_CIRCLE_GES_NO_DIR                      UINT8_C(0X00)
#define BMI270_CIRCLE_GES_CLOCKWISE_DIR               UINT8_C(0X01)
#define BMI270_CIRCLE_GES_ANTI_CLOCKWISE_DIR          UINT8_C(0X02)

/*! @name Mask definitions for BMI2 circle gesture detection feature configuration */
#define BMI270_CIRCLE_GEN_INT_EN_MASK                 UINT16_C(0x0001)
#define BMI270_CIRCLE_GEN_INT_SLOPE_THRE_MASK         UINT16_C(0x0FFF)
#define BMI270_CIRCLE_GEN_INT_COMB_SEL_MASK           UINT16_C(0x1000)
#define BMI270_CIRCLE_GEN_INT_AXIS_SEL_MASK           UINT16_C(0xE000)
#define BMI270_CIRCLE_GEN_INT_HYSTERESIS_MASK         UINT16_C(0x03FF)
#define BMI270_CIRCLE_GEN_INT_CRITERION_SEL_MASK      UINT16_C(0x0400)
#define BMI270_CIRCLE_GEN_INT_ACC_REF_UP_MASK         UINT16_C(0x1800)
#define BMI270_CIRCLE_GEN_INT_DURATION_MASK           UINT16_C(0X1FFF)
#define BMI270_CIRCLE_GEN_INT_WAIT_TIME_MASK          UINT16_C(0XE000)
#define BMI270_CIRCLE_GEN_INT_QUITE_TIME_MASK         UINT16_C(0X1FFF)
#define BMI270_CIRCLE_GEN_INT_REF_ACC_X_MASK          UINT16_C(0XFFFF)
#define BMI270_CIRCLE_GEN_INT_REF_ACC_Y_MASK          UINT16_C(0XFFFF)
#define BMI270_CIRCLE_GEN_INT_REF_ACC_Z_MASK          UINT16_C(0XFFFF)

/*! @name Bit position definition for BMI2 circle configuration for circle gesture detector variant */
#define BMI270_CIRCLE_GEN_INT_EN_POS                  UINT16_C(0x0000)
#define BMI270_CIRCLE_GEN_INT_SLOPE_THRE_POS          UINT16_C(0x0000)
#define BMI270_CIRCLE_GEN_INT_COMB_SEL_POS            UINT16_C(0x000C)
#define BMI270_CIRCLE_GEN_INT_AXIS_SEL_POS            UINT16_C(0x000D)
#define BMI270_CIRCLE_GEN_INT_HYSTERESIS_POS          UINT16_C(0x0000)
#define BMI270_CIRCLE_GEN_INT_CRITERION_SEL_POS       UINT16_C(0x000A)
#define BMI270_CIRCLE_GEN_INT_ACC_REF_UP_POS          UINT16_C(0x000B)
#define BMI270_CIRCLE_GEN_INT_DURATION_POS            UINT16_C(0X0000)
#define BMI270_CIRCLE_GEN_INT_WAIT_TIME_POS           UINT16_C(0X000D)
#define BMI270_CIRCLE_GEN_INT_QUITE_TIME_POS          UINT16_C(0X0000)
#define BMI270_CIRCLE_GEN_INT_REF_ACC_X_POS           UINT16_C(0X0000)
#define BMI270_CIRCLE_GEN_INT_REF_ACC_Y_POS           UINT16_C(0X0000)
#define BMI270_CIRCLE_GEN_INT_REF_ACC_Z_POS           UINT16_C(0X0000)

/*! @name Mask definitions for BMI270 circle tap feature configuration */
#define BMI270_CIRCLE_TAP_SENSITIVITY_MASK            UINT16_C(0x00C0)
#define BMI270_CIRCLE_TAP_SINGLE_TAP_EN_MASK          UINT16_C(0x01)
#define BMI270_CIRCLE_TAP_DOUBLE_TAP_EN_MASK          UINT16_C(0x02)
#define BMI270_CIRCLE_TAP_TRIPLE_TAP_EN_MASK          UINT16_C(0x04)
#define BMI270_CIRCLE_TAP_DATA_REG_EN_MASK            UINT16_C(0x08)
#define BMI270_CIRCLE_TAP_AXIS_SEL_MASK               UINT16_C(0x0003)

/*! @name Position definitions for BMI270 circle tap feature configuration */
#define BMI270_CIRCLE_TAP_SENSITIVITY_POS             UINT16_C(0x0006)
#define BMI270_CIRCLE_TAP_SINGLE_TAP_EN_POS           UINT16_C(0x00)
#define BMI270_CIRCLE_TAP_DOUBLE_TAP_EN_POS           UINT16_C(0x01)
#define BMI270_CIRCLE_TAP_TRIPLE_TAP_EN_POS           UINT16_C(0x02)
#define BMI270_CIRCLE_TAP_DATA_REG_EN_POS             UINT16_C(0x000B)
#define BMI270_CIRCLE_TAP_AXIS_SEL_POS                UINT16_C(0x0000)

/***************************************************************************/

/*!     BMI270_CIRCLE User Interface function prototypes
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
} bmi270_circle_i2c_config_t;

/**
 * @brief Handle type for BMI270 sensor
 *
 * This is a pointer to a structure representing the BMI270 device. It is used
 * as a handle for interacting with the sensor.
 */
typedef struct bmi2_dev * bmi270_circle_handle_t;

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
esp_err_t bmi270_circle_sensor_create(const bmi270_circle_i2c_config_t *i2c_conf, bmi270_circle_handle_t *handle_ret);

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
esp_err_t bmi270_circle_sensor_del(bmi270_circle_handle_t handle);

/**
 * \ingroup bmi270_circle
 * \defgroup bmi270_circleApiInit Initialization
 * @brief Initialize the sensor and device structure
 */

/*!
 * \ingroup bmi270_circleApiInit
 * \page bmi270_circle_api_bmi270_circle_init bmi270_circle_init
 * \code
 * int8_t bmi270_circle_init(struct bmi2_dev *dev);
 * \endcode
 * @details This API:
 *  1) updates the device structure with address of the configuration file.
 *  2) Initializes BMI270_CIRCLE sensor.
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
int8_t bmi270_circle_init(struct bmi2_dev *dev);

/**
 * \ingroup bmi270_circle
 * \defgroup bmi270_circleApiSensor Feature Set
 * @brief Enable / Disable features of the sensor
 */

/*!
 * \ingroup bmi270_circleApiSensor
 * \page bmi270_circle_api_bmi270_circle_sensor_enable bmi270_circle_sensor_enable
 * \code
 * int8_t bmi270_circle_sensor_enable(const uint8_t *sens_list, uint8_t n_sens, struct bmi2_dev *dev);
 * \endcode
 * @details This API selects the sensors/features to be enabled.
 *
 * @param[in]       sens_list   : Pointer to select the sensor/feature.
 * @param[in]       n_sens      : Number of sensors selected.
 * @param[in, out]  dev         : Structure instance of bmi2_dev.
 *
 * @note Sensors/features that can be enabled.
 *
 *@verbatim
 *    sens_list                |  Values
 * ----------------------------|-----------
 * BMI2_ACCEL                  |  0
 * BMI2_GYRO                   |  1
 * BMI2_AUX                    |  2
 * BMI2_SIG_MOTION             |  3
 * BMI2_ANY_MOTION             |  4
 * BMI2_NO_MOTION              |  5
 * BMI2_STEP_DETECTOR          |  6
 * BMI2_STEP_COUNTER           |  7
 * BMI2_STEP_ACTIVITY          |  8
 * BMI2_GYRO_GAIN_UPDATE       |  9
 * BMI2_ORIENTATION            |  14
 * BMI2_HIGH_G                 |  15
 * BMI2_LOW_G                  |  16
 * BMI2_FLAT                   |  17
 * BMI2_SINGLE_TAP             |  25
 * BMI2_DOUBLE_TAP             |  26
 * BMI2_TRIPLE_TAP             |  27
 * BMI2_TEMP                   |  31
 *@endverbatim
 *
 * @note :
 * example  uint8_t sens_list[2]  = {BMI2_ACCEL, BMI2_GYRO};
 *           uint8_t n_sens        = 2;
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_sensor_enable(const uint8_t *sens_list, uint8_t n_sens, struct bmi2_dev *dev);

/*!
 * \ingroup bmi270_circleApiSensor
 * \page bmi270_circle_api_bmi270_circle_sensor_disable bmi270_circle_sensor_disable
 * \code
 * int8_t bmi270_circle_sensor_disable(const uint8_t *sens_list, uint8_t n_sens, struct bmi2_dev *dev);
 * \endcode
 * @details This API selects the sensors/features to be disabled.
 *
 * @param[in]       sens_list   : Pointer to select the sensor/feature.
 * @param[in]       n_sens      : Number of sensors selected.
 * @param[in, out]  dev         : Structure instance of bmi2_dev.
 *
 * @note Sensors/features that can be disabled.
 *
 *@verbatim
 *    sens_list                |  Values
 * ----------------------------|-----------
 * BMI2_ACCEL                  |  0
 * BMI2_GYRO                   |  1
 * BMI2_AUX                    |  2
 * BMI2_SIG_MOTION             |  3
 * BMI2_ANY_MOTION             |  4
 * BMI2_NO_MOTION              |  5
 * BMI2_STEP_DETECTOR          |  6
 * BMI2_STEP_COUNTER           |  7
 * BMI2_STEP_ACTIVITY          |  8
 * BMI2_GYRO_GAIN_UPDATE       |  9
 * BMI2_ORIENTATION            |  14
 * BMI2_HIGH_G                 |  15
 * BMI2_LOW_G                  |  16
 * BMI2_FLAT                   |  17
 * BMI2_SINGLE_TAP             |  25
 * BMI2_DOUBLE_TAP             |  26
 * BMI2_TRIPLE_TAP             |  27
 * BMI2_TEMP                   |  31
 *@endverbatim
 *
 * @note :
 * example  uint8_t sens_list[2]  = {BMI2_ACCEL, BMI2_GYRO};
 *           uint8_t n_sens        = 2;
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_sensor_disable(const uint8_t *sens_list, uint8_t n_sens, struct bmi2_dev *dev);

/**
 * \ingroup bmi270_circle
 * \defgroup bmi270_circleApiSensorC Sensor Configuration
 * @brief Enable / Disable feature configuration of the sensor
 */

/*!
 * \ingroup bmi270_circleApiSensorC
 * \page bmi270_circle_api_bmi270_circle_set_sensor_config bmi270_circle_set_sensor_config
 * \code
 * int8_t bmi270_circle_set_sensor_config(struct bmi2_sens_config *sens_cfg, uint8_t n_sens, struct bmi2_dev *dev);
 * \endcode
 * @details This API sets the sensor/feature configuration.
 *
 * @param[in]       sens_cfg     : Structure instance of bmi2_sens_config.
 * @param[in]       n_sens       : Number of sensors selected.
 * @param[in, out]  dev          : Structure instance of bmi2_dev.
 *
 * @note Sensors/features that can be configured
 *
 *@verbatim
 *    sens_list                |  Values
 * ----------------------------|-----------
 * BMI2_SIG_MOTION             |  3
 * BMI2_ANY_MOTION             |  4
 * BMI2_NO_MOTION              |  5
 * BMI2_STEP_DETECTOR          |  6
 * BMI2_STEP_COUNTER           |  7
 * BMI2_STEP_ACTIVITY          |  8
 * BMI2_ORIENTATION            |  14
 * BMI2_HIGH_G                 |  15
 * BMI2_LOW_G                  |  16
 * BMI2_FLAT                   |  17
 * BMI2_TAP                    |  28
 *@endverbatim
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_set_sensor_config(struct bmi2_sens_config *sens_cfg, uint8_t n_sens, struct bmi2_dev *dev);

/*!
 * \ingroup bmi270_circleApiSensorC
 * \page bmi270_circle_api_bmi270_circle_get_sensor_config bmi270_circle_get_sensor_config
 * \code
 * int8_t bmi270_circle_get_sensor_config(struct bmi2_sens_config *sens_cfg, uint8_t n_sens, struct bmi2_dev *dev);
 * \endcode
 * @details This API gets the sensor/feature configuration.
 *
 * @param[in]       sens_cfg     : Structure instance of bmi2_sens_config.
 * @param[in]       n_sens       : Number of sensors selected.
 * @param[in, out]  dev          : Structure instance of bmi2_dev.
 *
 * @note Sensors/features whose configurations can be read.
 *
 *@verbatim
 *  sens_list                  |  Values
 * ----------------------------|-----------
 * BMI2_SIG_MOTION             |  3
 * BMI2_ANY_MOTION             |  4
 * BMI2_NO_MOTION              |  5
 * BMI2_STEP_DETECTOR          |  6
 * BMI2_STEP_COUNTER           |  7
 * BMI2_STEP_ACTIVITY          |  8
 * BMI2_ORIENTATION            |  14
 * BMI2_HIGH_G                 |  15
 * BMI2_LOW_G                  |  16
 * BMI2_FLAT                   |  17
 * BMI2_TAP                    |  28
 *@endverbatim
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_get_sensor_config(struct bmi2_sens_config *sens_cfg, uint8_t n_sens, struct bmi2_dev *dev);

/**
 * \ingroup bmi270_circle
 * \defgroup bmi270_circleApiSensorD Feature Sensor Data
 * @brief Get feature sensor data
 */

/*!
 * \ingroup bmi270_circleApiSensorD
 * \page bmi270_circle_api_bmi270_circle_get_feature_data bmi270_circle_get_feature_data
 * \code
 * int8_t bmi270_circle_get_feature_data(struct bmi2_feat_sensor_data *feature_data, uint8_t n_sens, struct bmi2_dev *dev);
 * \endcode
 * @details This API gets the feature data.
 *
 * @param[out] feature_data   : Structure instance of bmi2_feat_sensor_data.
 * @param[in]  n_sens         : Number of sensors selected.
 * @param[in]  dev            : Structure instance of bmi2_dev.
 *
 * @note Sensors/features whose data can be read
 *
 *@verbatim
 *  sens_list           |  Values
 * ---------------------|-----------
 * BMI2_STEP_COUNTER    |  7
 * BMI2_STEP_ACTIVITY   |  8
 * BMI2_ORIENTATION     |  14
 * BMI2_HIGH_G          |  15
 * BMI2_NVM_STATUS      |  38
 * BMI2_VFRM_STATUS     |  39
 *@endverbatim
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_get_feature_data(struct bmi2_feat_sensor_data *feature_data, uint8_t n_sens, struct bmi2_dev *dev);

/**
 * \ingroup bmi270_circle
 * \defgroup bmi270_circleApiGyroUG Gyro User Gain
 * @brief Set / Get Gyro User Gain of the sensor
 */

/*!
 * \ingroup bmi270_circleApiGyroUG
 * \page bmi270_circle_api_bmi270_circle_update_gyro_user_gain bmi270_circle_update_gyro_user_gain
 * \code
 * int8_t bmi270_circle_update_gyro_user_gain(const struct bmi2_gyro_user_gain_config *user_gain, struct bmi2_dev *dev);
 * \endcode
 * @details This API updates the gyroscope user-gain.
 *
 * @param[in] user_gain      : Structure that stores user-gain configurations.
 * @param[in] dev            : Structure instance of bmi2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_update_gyro_user_gain(const struct bmi2_gyro_user_gain_config *user_gain, struct bmi2_dev *dev);

/*!
 * \ingroup bmi270_circleApiGyroUG
 * \page bmi270_circle_api_bmi270_circle_read_gyro_user_gain bmi270_circle_read_gyro_user_gain
 * \code
 * int8_t bmi270_circle_read_gyro_user_gain(struct bmi2_gyro_user_gain_data *gyr_usr_gain, const struct bmi2_dev *dev);
 * \endcode
 * @details This API reads the compensated gyroscope user-gain values.
 *
 * @param[out] gyr_usr_gain   : Structure that stores gain values.
 * @param[in]  dev            : Structure instance of bmi2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_read_gyro_user_gain(struct bmi2_gyro_user_gain_data *gyr_usr_gain, struct bmi2_dev *dev);

/*!
 * \ingroup bmi270_circleApiInt
 * \page bmi270_circle_api_bmi270_circle_map_feat_int bmi270_circle_map_feat_int
 * \code
 * int8_t bmi270_circle_map_feat_int(const struct bmi2_sens_int_config *sens_int, uint8_t n_sens, struct bmi2_dev *dev)
 * \endcode
 * @details This API maps/unmaps feature interrupts to that of interrupt pins.
 *
 * @param[in] sens_int     : Structure instance of bmi2_sens_int_config.
 * @param[in] n_sens       : Number of interrupts to be mapped.
 * @param[in] dev          : Structure instance of bmi2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bmi270_circle_map_feat_int(const struct bmi2_sens_int_config *sens_int, uint8_t n_sens, struct bmi2_dev *dev);

/******************************************************************************/
/*! @name       C++ Guard Macros                                      */
/******************************************************************************/
#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif /* _BMI270_CIRCLE_H_ */
