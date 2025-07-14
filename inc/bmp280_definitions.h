/**
* @file	bmp280_definitions.h
* @brief BMP280 barometric pressure and temperature sensor C Driver
* @author Reza G. Ebrahimi <https://github.com/ebrezadev>
* @version 3.0

* @license MIT 
*
* MIT License
* 
* Copyright (c) 2025 Reza G. Ebrahimi
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* 
*/

#ifndef __BMP280_DEFS_H__
#define __BMP280_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bmp280_config.h"

#ifndef NULL
	#ifdef __cplusplus
		#define NULL 0
	#else
		#define NULL ((void *)0)
	#endif
#endif

/*constant definitions*/
static const uint8_t BMP280_DEFAULT_CHIP_ID = 0x58;
static const uint8_t BMP280_RESET_VALUE = 0xB6;

static const uint8_t BMP280_MEASURING_IN_PROGRESS = 0x01;
static const uint8_t BMP280_MEASURING_DONE = 0x00;

static const uint32_t SEA_LEVEL_PRESSURE = 101325;

/*data types*/

/**
 * @brief Register addresses for BMP280
 * 
 */
typedef enum 
{
	BMP280_REGISTER_ADDRESS_T1 = 0X88,
	BMP280_REGISTER_ADDRESS_T2 = 0X8A,
	BMP280_REGISTER_ADDRESS_T3 = 0X8C,
	BMP280_REGISTER_ADDRESS_P1 = 0X8E,
	BMP280_REGISTER_ADDRESS_P2 = 0X90,
	BMP280_REGISTER_ADDRESS_P3 = 0X92,
	BMP280_REGISTER_ADDRESS_P4 = 0X94,
	BMP280_REGISTER_ADDRESS_P5 = 0X96,
	BMP280_REGISTER_ADDRESS_P6 = 0X98,
	BMP280_REGISTER_ADDRESS_P7 = 0X9A,
	BMP280_REGISTER_ADDRESS_P8 = 0X9C,
	BMP280_REGISTER_ADDRESS_P9 = 0X9E,
	BMP280_REGISTER_ADDRESS_ID = 0XD0,
	BMP280_REGISTER_ADDRESS_RESET = 0XE0,
	BMP280_REGISTER_ADDRESS_STATUS = 0XF3,
	BMP280_REGISTER_ADDRESS_CONTROL_MEAS = 0XF4,
	BMP280_REGISTER_ADDRESS_CONFIG = 0XF5,
	BMP280_REGISTER_ADDRESS_PRESSURE_MSB = 0xF7,
	BMP280_REGISTER_ADDRESS_PRESSURE_LSB = 0xF8,
	BMP280_REGISTER_ADDRESS_PRESSURE_XLSB = 0xF9,
	BMP280_REGISTER_ADDRESS_TEMPERATURE_MSB = 0xFA,
	BMP280_REGISTER_ADDRESS_TEMPERATURE_LSB = 0xFB,
	BMP280_REGISTER_ADDRESS_TEMPERATURE_XLSB = 0xFC
} bmp280_register_address_t;

/**
 * @brief Starting bit addresses of bit-fields in registers in BMP280
 * 
 */
typedef enum
{
	BMP280_REGISTER_BIT_IM_UPDATE = 0X00,
	BMP280_REGISTER_BIT_MEASURING = 0X03,
	BMP280_REGISTER_BIT_MODE = 0X00,
	BMP280_REGISTER_BIT_OSRS_P = 0X02,
	BMP280_REGISTER_BIT_OSRS_T = 0X05,
	BMP280_REGISTER_BIT_SPI3W_EN = 0X00,
	BMP280_REGISTER_BIT_FILTER = 0X02,
	BMP280_REGISTER_BIT_T_SB = 0X05
} bmp280_register_bit_t;

/**
 * @brief bit-field lengths in BMP280
 * 
 */
typedef enum 
{
	BMP280_REGISTER_FIELD_LENGTH_IM_UPDATE = 0X01,
	BMP280_REGISTER_FIELD_LENGTH_MEASURING = 0X01,
	BMP280_REGISTER_FIELD_LENGTH_MODE = 0X02,
	BMP280_REGISTER_FIELD_LENGTH_OSRS_P = 0X03,
	BMP280_REGISTER_FIELD_LENGTH_OSRS_T = 0X03,
	BMP280_REGISTER_FIELD_LENGTH_SPI3W_EN = 0X01,
	BMP280_REGISTER_FIELD_LENGTH_FILTER = 0X03,
	BMP280_REGISTER_FIELD_LENGTH_T_SB = 0X03
} bmp280_register_field_length_t;

/**
 * @brief Oversampling rates for temperature and pressure
 */
typedef enum
{
	OVERSAMPLING_0X = 0X00,
	OVERSAMPLING_1X = 0X01,
	OVERSAMPLING_2X = 0X02,
	OVERSAMPLING_4X = 0X03,
	OVERSAMPLING_8X = 0X04,
	OVERSAMPLING_16X = 0X05
} bmp280_over_sampling_t;

/**
 * @brief Possible interfaces for the BMP280 device driver
 * 
 */
typedef enum {
	BMP280_I2C,
	BMP280_SPI,
	BMP280_OTHER
} bmp280_hardware_interface_t;

/**
 * @brief I2C address used for I2C interface
 * 
 */
typedef enum
{
	I2C_ADDRESS_NONE = 0,
	I2C_ADDRESS_1 = 0X76,
	I2C_ADDRESS_2 = 0X77
} bmp280_i2c_address_t;

/**
 * @brief Standby time period to be used in NORMAL mode
 * 
 */
typedef enum
{
	T_STANDBY_500US = 0X00,
	T_STANDBY_62500US,
	T_STANDBY_125MS,
	T_STANDBY_250MS,
	T_STANDBY_500MS,
	T_STANDBY_1S,
	T_STANDBY_2S,
	T_STANDBY_4S
} bmp280_standby_time_t;

/**
 * @brief Possible operation modes of BMP280
 * 
 */
typedef enum
{
	MODE_SLEEP = 0x00,
	MODE_FORCED = 0x01,
	MODE_NORMAL = 0x03
} bmp280_operation_mode_t;

/**
 * @brief Possible values for filter coefficient
 * 
 */
typedef enum
{
	FILTER_OFF = 0X00,
	FILTER_2X,
	FILTER_4X,
	FILTER_8X,
	FILTER_16X
} bmp280_iir_filter_t;

/**
 * @brief Struct of calibration parameters unique to each BMP280 sensor
 * 
 */
typedef struct
{
	uint16_t T1;
	int16_t T2;
	int16_t T3;
	uint16_t P1;
	int16_t P2;
	int16_t P3;
	int16_t P4;
	int16_t P5;
	int16_t P6;
	int16_t P7;
	int16_t P8;
	int16_t P9;
} bmp280_calibration_param_t;

/**
 * @brief Structure needed in case of using bmp280_get_all() function
 * 
 */
typedef struct
{
	float temperature;
	uint32_t pressure;
	
	#if BMP280_INCLUDE_ALTITUDE
	float altitude;
	#endif
} bmp280_sensors_data_t;

/*Default values section*/
static const bmp280_over_sampling_t BMP280_PRESSURE_OVERSAMPLING_DEFAULT = OVERSAMPLING_1X;
static const bmp280_over_sampling_t BMP280_TEMPERATURE_OVERSAMPLING_DEFAULT = OVERSAMPLING_1X;
static const bmp280_standby_time_t BMP280_STANDBY_TIME_DEFAULT = T_STANDBY_125MS;
static const bmp280_iir_filter_t BMP280_FILTER_DEFAULT = FILTER_OFF;
static const bmp280_operation_mode_t BMP280_MODE_DEFAULT = MODE_SLEEP;

/*Dependency pointers*/

/**
 * @brief The interface initializer
 * 
 * Implements the interface (or optionally chip power) initializer, whether I2c, SPI or test mock.
 * 
 * @return Returns 0 for no error
 * 
 */
typedef int (*bmp280_interface_init_fp)(void);


/**
 * @brief The interface de-initializer
 * 
 * Implements the interface (or optionally chip power) de-initializer, whether I2c, SPI or test mock.
 * 
 * @return Returns 0 for no error
 * 
 */
typedef int (*bmp280_interface_deinit_fp)(void);


/**
 * @brief The delay function
 * 
 * Implements a delay function in milliseconds.
 * 
 * @return Returns 0 for no error
 * 
 */
typedef int (*bmp280_delay_function_fp)(uint32_t delayMS);

#if BMP280_INCLUDE_ALTITUDE 

/**
 * @brief The power function
 * 
 * Implements a power math function used for altitude calculations.
 * 
 * @return Returns 0 for no error
 * 
 */
typedef int (*bmp280_power_function_fp) (float x, float y, float *result);

#endif

/**
 * @brief The write array function
 * 
 * Implements the interface write function
 * 
 * @param deviceAddress: Used in case of I2C interface
 * @param startRegisterAddress: The address of starting register
 * @param data: Pointer to the array of data
 * @param dataLength: Length of data array
 * @return Returns 0 for no error
 * 
 */
typedef int (*bmp280_write_array_fp)(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);


/**
 * @brief The read array function
 * 
 * Implements the interface read function
 * 
 * @param deviceAddress: Used in case of I2C interface
 * @param startRegisterAddress: The address of starting register
 * @param data: Pointer to the array of data
 * @param dataLength: Length of data array
 * @return Returns 0 for no error
 * 
 */
typedef int (*bmp280_read_array_fp)(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);


/**
 * @brief The dependency interface structure
 * 
 * Please define your interface functions and point these function-pointers to them.
 * 
 */
typedef struct
{
	bmp280_interface_init_fp bmp280_interface_init;
	bmp280_interface_deinit_fp bmp280_interface_deinit;
	bmp280_delay_function_fp bmp280_delay_function;
	bmp280_write_array_fp bmp280_write_array;
	bmp280_read_array_fp bmp280_read_array;

	#if BMP280_INCLUDE_ALTITUDE 
	bmp280_power_function_fp bmp280_power_function;
	#endif
} bmp280_dependency_t;


/**
 * @brief The handle to BMP280 instance
 * 
 * The handle to an instance of BMP280 sensor. Please set the correct dependency interface.
 * 
 */
typedef struct
{
	bmp280_operation_mode_t operation_mode;
	bmp280_i2c_address_t i2c_address;
	bmp280_calibration_param_t dig;
	int32_t t_fine;
	uint8_t poll_timeout_ms;
	bmp280_hardware_interface_t hardware_interface;
	bmp280_dependency_t dependency_interface;
} bmp280_handle_t;

#ifdef __cplusplus
}
#endif

#endif
