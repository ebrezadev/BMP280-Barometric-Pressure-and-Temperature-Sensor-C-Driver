/**
* @file	bmp280.h
* @brief BMP280 barometric pressure and temperature sensor C Driver
* @author Reza G. Ebrahimi <https://github.com/ebrezadev>
* @version 5.0
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

#ifndef __BMP280_H__
#define __BMP280_H__

#include "bmp280_definitions.h"
#include "bmp280_config.h"
#include "bmp280_error.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief BMP280 initializer
 * 
 * Checks for NULL handle, checks for NULL interface and address (for I2C), 
 * resets and initializes sensor, gets the calibration data for further 
 * calculations. also sets the default values. 
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param hw_interface: Defines which hardware interface is used: I2C, SPI or a software mock.
 * @param i2c_address: I2C address in case of I2C interface.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_init(
	bmp280_handle_t *handle, 
	bmp280_hardware_interface_t hw_interface, 
	bmp280_i2c_address_t i2c_address);


/**
 * @brief BMP280 deinitializer
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_deinit(bmp280_handle_t *handle);


/**
 * @brief BMP280 chip ID checker
 * 
 * Checks chip ID to see if it really is a BMP280 module with correct wiring and address.
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_check_id(bmp280_handle_t *bmp280_handle);


/**
 * @brief BMP280 soft reset
 * 
 * Soft resets BMP280 using special reset register
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_reset(bmp280_handle_t *handle);


/**
 * @brief BMP280 set mode
 * 
 * Setting bmp280 mode: MODE_SLEEP, MODE_FORCED, MODE_NORMAL
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param operationMode: The mode of operation: NORMAL_MODE, SLEEP_MODE, FORCED_MODE
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_set_mode(
	bmp280_handle_t *handle, 
	bmp280_operation_mode_t operationMode);


/**
 * @brief BMP280 set pressure oversampling
 * 
 * Setting pressure oversampling from 0 (skip) to 16x
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param osValue: Pressure oversampling value
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_set_pressure_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t osValue);


/**
 * @brief BMP280 set temperature oversampling
 * 
 * Setting temperature oversampling from 0 (skip) to 16x
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param osValue: Temperature oversampling value
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_set_temperature_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t osValue);


/**
 * @brief BMP280 set standby time
 * 
 * Setting standby time period in normal mode
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param standbyTime: standby time value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_set_standby_time(
	bmp280_handle_t *handle, 
	bmp280_standby_time_t standbyTime);


/**
 * @brief BMP280 set filter coefficient
 * 
 * Sets low pass internal filter coefficient
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param filterCoefficient: Filter coefficient value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_set_filter_coefficient(
	bmp280_handle_t *handle, 
	bmp280_iir_filter_t filterCoefficient);


/**
 * @brief BMP280 get temperature
 * 
 * Gets temperature in Centigrade
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param temperature: Pointer to temperature value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_get_temperature(
	bmp280_handle_t *handle, 
	float *temperature);


/**
 * @brief BMP280 get pressure
 * 
 * Gets pressure in Pascal
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param pressure: Pointer to pressure value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_get_pressure(
	bmp280_handle_t *handle, 
	uint32_t *pressure);

#if BMP280_INCLUDE_ALTITUDE 

/**
 * @brief BMP280 calculate altitude
 * 
 * Calculates altitude from barometric pressure without temperature as an argument
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param alt: Pointer to altitude value.
 * @param barometricPressure: Barometric pressure read from sensor
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_calculate_altitude_quick(
	bmp280_handle_t *handle, 
	float *alt, 
	uint32_t barometricPressure);

/**
 * @brief BMP280 calculate altitude
 * 
 * Calculates altitude from barometric pressure and temperature as arguments
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param alt: Pointer to altitude value.
 * @param barometricPressure: Barometric pressure read from sensor
 * @param ambientTemperatureInC: Temperature in Centigrade
 * @return 0 or ERROR_OK on success, other values on errors.
 */	
bmp280_error_code_t bmp280_calculate_altitude_hypsometric(
	bmp280_handle_t *handle, 
	float *alt, 
	uint32_t barometricPressure, 
	float ambientTemperatureInC);
#endif

#if BMP280_INCLUDE_ADDITIONAL_GETTERS

/**
 * @brief BMP280 get temperature and pressure (optional: altitude)
 * 
 * Returns a complete set of sensor readings and altitude calculation (quick)
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param alt: Pointer to altitude value.
 * @param data: An structure of temperature, pressure and (optional) altitude
 * @return 0 or ERROR_OK on success, other values on errors.
 */	
bmp280_error_code_t bmp280_get_all(
	bmp280_handle_t *handle, 
	bmp280_sensors_data_t *data);


/**
 * @brief BMP280 get mode of operation
 * 
 * Returns mode of operation: MODE_NORMAL, MODE_SLEEP or MODE_FORCED
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param mode: Pointer to mode of operation value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */	
bmp280_error_code_t bmp280_get_mode(
	bmp280_handle_t *handle, 
	bmp280_operation_mode_t *mode);


/**
 * @brief BMP280 get temperature oversampling
 * 
 * Returns the current temperature oversampling
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param tempOS: Pointer to temperature oversampling value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */	
bmp280_error_code_t bmp280_get_temperature_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t *tempOS);


/**
 * @brief BMP280 get pressure oversampling
 * 
 * Returns the current pressure oversampling
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param pressureOS: Pointer to pressure oversampling value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */	
bmp280_error_code_t bmp280_get_pressure_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t *pressureOS);


/**
 * @brief BMP280 get standby time
 * 
 * Returns the current standby time (for normal mode)
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param standby_time: Pointer to standby time value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_get_standby_time(
	bmp280_handle_t *handle, 
	bmp280_standby_time_t *standby_time);


/**
 * @brief BMP280 get filter coefficient
 * 
 * Returns the current IIR filter coefficient
 * 
 * @param handle: Pointer to the BMP280 instance handle structure.
 * @param filter_coeff: Pointer to filter coefficient value.
 * @return 0 or ERROR_OK on success, other values on errors.
 */
bmp280_error_code_t bmp280_get_filter_coefficient(
	bmp280_handle_t *handle, 
	bmp280_iir_filter_t *filter_coeff);
#endif

#ifdef __cplusplus
}
#endif

#endif
