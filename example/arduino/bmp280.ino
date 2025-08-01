/**
* @file	bmp280.c
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

#include "bmp280.h"
#include <stdio.h>

#define BMP280_CHECK_AND_RETURN_ERROR(error) \
    do { \
        if (error != BMP280_ERROR_OK) \
		{ \
			return error; \
		} \
    } while(0) 


#if BMP280_INCLUDE_EXCLUSION_HOOK 
#define BMP280_LOCK(handle) \
	do { \
		if(handle->dependency_interface.bmp280_interface_exclusion.bmp280_interface_lock(handle->dependency_interface.bmp280_interface_exclusion.mutex_handle) != 0) \
		{ \
			return BMP280_ERROR_EXCLUSION_LOCK; \
		} \
	} while(0)
#define BMP280_UNLOCK(handle) \
	do { \
		if(handle->dependency_interface.bmp280_interface_exclusion.bmp280_interface_unlock(handle->dependency_interface.bmp280_interface_exclusion.mutex_handle) != 0) \
		{ \
			return BMP280_ERROR_EXCLUSION_UNLOCK; \
		} \
	} while(0)
#else
#define BMP280_LOCK(handle) ;
#define BMP280_UNLOCK(handle) ;
#endif


/*Static functions are used internally*/

/*Sets a field of bits inside a BMP280 register*/
static bmp280_error_code_t bmp280_set_bits_in_register(bmp280_handle_t *handle, bmp280_register_address_t registerAddress, uint8_t fieldData, bmp280_register_bit_t fieldStartBitAddress, bmp280_register_field_length_t fieldLength);
static bmp280_error_code_t bmp280_get_bits_in_register(bmp280_handle_t *handle, bmp280_register_address_t registerAddress, uint8_t *fieldData, bmp280_register_bit_t fieldStartBitAddress, bmp280_register_field_length_t fieldLength);
/*Gets calibration data. Must be done once in startup.*/
static bmp280_error_code_t bmp280_get_calibration(bmp280_handle_t *handle);
/*Gets raw temperature data. needs further processing.*/
static bmp280_error_code_t bmp280_raw_temperature_data(bmp280_handle_t *handle, int32_t *raw_data);
/*Gets raw pressure data. needs further processing.*/
static bmp280_error_code_t bmp280_raw_pressure_data(bmp280_handle_t *handle, int32_t *raw_data);
static bmp280_error_code_t bmp280_read_calibration_word_unsigned(bmp280_handle_t *handle, bmp280_register_address_t startRegisterAddress, uint16_t *result);
static bmp280_error_code_t bmp280_read_calibration_word_signed(bmp280_handle_t *handle, bmp280_register_address_t startRegisterAddress, int16_t *result);


bmp280_error_code_t bmp280_init(
	bmp280_handle_t *handle,
	bmp280_hardware_interface_t hw_interface,
	bmp280_i2c_address_t i2c_address)
{
	bmp280_error_code_t error;

	/*check for NULL handle*/
	if(handle == NULL)
	{
		return BMP280_ERROR_NULL_HANDLE;
	}

	#if BMP280_INCLUDE_EXCLUSION_HOOK
	if(handle->dependency_interface.bmp280_interface_exclusion.mutex_handle == NULL)
	{
		return BMP280_ERROR_NULL_MUTEX_HANDLE;
	}
	#endif

	/*check for NULL or undefined dependencies*/
	if(handle->dependency_interface.bmp280_delay_function == NULL ||
		handle->dependency_interface.bmp280_interface_init == NULL ||
		handle->dependency_interface.bmp280_interface_deinit == NULL ||

		#if BMP280_INCLUDE_ALTITUDE
		handle->dependency_interface.bmp280_power_function == NULL ||
		#endif

		#if BMP280_INCLUDE_EXCLUSION_HOOK
		handle->dependency_interface.bmp280_interface_exclusion.bmp280_interface_lock == NULL ||
		handle->dependency_interface.bmp280_interface_exclusion.bmp280_interface_unlock == NULL ||
		#endif

		handle->dependency_interface.bmp280_read_array == NULL ||
		handle->dependency_interface.bmp280_write_array == NULL)
	{
		return BMP280_ERROR_NULL_INTERFACE;
	}

	handle->hardware_interface = hw_interface;

	/*check if the hardware interface is set to I2C*/
	if(hw_interface == BMP280_I2C)
	{
		handle->i2c_address = i2c_address;
	}
	else
	{
		handle->i2c_address = BMP280_I2C_ADDRESS_NONE;
	}

	/*initialize the hardware interface*/
	BMP280_LOCK(handle);
	if(handle->dependency_interface.bmp280_interface_init((uint8_t)(handle->i2c_address)) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_INIT;
	}
	BMP280_UNLOCK(handle);

	/*check if the driver can connect to sensor hardware by checking its ID*/
	error = bmp280_check_id(handle);
	
	if(error != BMP280_ERROR_OK)
	{
		handle->i2c_address = BMP280_I2C_ADDRESS_NONE;
		return error;
	}

	error = bmp280_reset(handle);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_get_calibration(handle);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, BMP280_MODE_DEFAULT);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_pressure_oversampling(handle, BMP280_PRESSURE_OVERSAMPLING_DEFAULT);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_temperature_oversampling(handle, BMP280_TEMPERATURE_OVERSAMPLING_DEFAULT);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_standby_time(handle, BMP280_STANDBY_TIME_DEFAULT);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_filter_coefficient(handle, BMP280_FILTER_DEFAULT);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	return BMP280_ERROR_OK;
}

bmp280_error_code_t bmp280_deinit(bmp280_handle_t *handle)
{
	bmp280_error_code_t error = bmp280_reset(handle);

	BMP280_CHECK_AND_RETURN_ERROR(error);

	BMP280_LOCK(handle);
	if(handle->dependency_interface.bmp280_interface_deinit((uint8_t)handle->i2c_address) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_DEINIT;
	}
	BMP280_UNLOCK(handle);

	handle->operation_mode = BMP280_MODE_SLEEP;

	return BMP280_ERROR_OK;
}

/*checks chip id to see if it really is a bmp280 module with correct wiring and address*/
bmp280_error_code_t bmp280_check_id(bmp280_handle_t *handle)
{
	uint8_t data;

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, BMP280_REGISTER_ADDRESS_ID, &data, 1) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}
	BMP280_UNLOCK(handle);

	if (data == BMP280_DEFAULT_CHIP_ID)
	{
		return BMP280_ERROR_OK;
	}
	else
	{
		return BMP280_ERROR_SENSOR_ID;
	}
}

/*soft resets bm280 using special reset register*/
bmp280_error_code_t bmp280_reset(bmp280_handle_t *handle)
{
	uint8_t data = BMP280_RESET_VALUE;

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_write_array((uint8_t)handle->i2c_address, BMP280_REGISTER_ADDRESS_RESET, &data, 1) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_WRITE;
	}
	BMP280_UNLOCK(handle);

	if (handle->dependency_interface.bmp280_delay_function(BMP280_STARTUP_DELAY_IN_MS) != 0)
	{
		return BMP280_ERROR_INTERFACE_DELAY;
	}

	handle->operation_mode = BMP280_MODE_SLEEP;

	return BMP280_ERROR_OK;
}

/*setting bmp280 mode, NORMAL_MODE, FORCED_MODE, SLEEP_MODE*/
bmp280_error_code_t bmp280_set_mode(
	bmp280_handle_t *handle,
	bmp280_operation_mode_t operationMode)
{
	bmp280_error_code_t error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, operationMode, BMP280_REGISTER_BIT_MODE, BMP280_REGISTER_FIELD_LENGTH_MODE);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_delay_function(50) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_DELAY;
	}
	BMP280_UNLOCK(handle);

	handle->operation_mode = operationMode;

	return BMP280_ERROR_OK;
}

/*setting pressure oversampling from 0 (skip) to 16x*/
bmp280_error_code_t bmp280_set_pressure_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t osValue)
{
	bmp280_operation_mode_t operation_mode = handle->operation_mode;

	/*In order to change configuration, the BMP280 sensor must be in SLEEP mode*/
	bmp280_error_code_t error = bmp280_set_mode(handle, BMP280_MODE_SLEEP);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, (uint8_t)osValue, BMP280_REGISTER_BIT_OSRS_P, BMP280_REGISTER_FIELD_LENGTH_OSRS_P);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, operation_mode);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	return BMP280_ERROR_OK;
}

/*setting temperature oversampling from 0 (skip) to 16x*/
bmp280_error_code_t bmp280_set_temperature_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t osValue)
{
	bmp280_operation_mode_t operation_mode = handle->operation_mode;

	bmp280_error_code_t error = bmp280_set_mode(handle, BMP280_MODE_SLEEP);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, (uint8_t)osValue, BMP280_REGISTER_BIT_OSRS_T, BMP280_REGISTER_FIELD_LENGTH_OSRS_T);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, operation_mode);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	return BMP280_ERROR_OK;
}

/*sets standby time between measurements in normal mode. lower standby time means higher power consumption*/
bmp280_error_code_t bmp280_set_standby_time(
	bmp280_handle_t *handle,
	bmp280_standby_time_t standbyTime)
{
	bmp280_operation_mode_t operation_mode = handle->operation_mode;

	bmp280_error_code_t error = bmp280_set_mode(handle, BMP280_MODE_SLEEP);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, (uint8_t)standbyTime, BMP280_REGISTER_BIT_T_SB, BMP280_REGISTER_FIELD_LENGTH_T_SB);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, operation_mode);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	return BMP280_ERROR_OK;
}

/*sets low pass internal filter coefficient for bmp280. used in noisy environments*/
bmp280_error_code_t bmp280_set_filter_coefficient(
	bmp280_handle_t *handle,
	bmp280_iir_filter_t filterCoefficient)
{
	bmp280_operation_mode_t operation_mode = handle->operation_mode;

	bmp280_error_code_t error = bmp280_set_mode(handle, BMP280_MODE_SLEEP);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, (uint8_t)filterCoefficient, BMP280_REGISTER_BIT_FILTER, BMP280_REGISTER_FIELD_LENGTH_FILTER);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, operation_mode);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	return BMP280_ERROR_OK;
}

/*reads temperature value from internal bmp280 registers in centigrade*/
bmp280_error_code_t bmp280_get_temperature(
	bmp280_handle_t *handle,
	float *temperature)
{
	int32_t adc_T;

	bmp280_error_code_t error = bmp280_raw_temperature_data(handle, &adc_T);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	int32_t var1, var2;

	var1 = ((((adc_T >> 3) - ((int32_t)handle->dig.T1 << 1))) * ((int32_t)handle->dig.T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)handle->dig.T1)) * ((adc_T >> 4) - ((int32_t)handle->dig.T1))) >> 12) * ((int32_t)handle->dig.T3)) >> 14;

	handle->t_fine = var1 + var2;
	int32_t temp_temperature = (handle->t_fine * 5 + 128) >> 8;
	*temperature = ((float)temp_temperature) / 100.0;

	return BMP280_ERROR_OK;
}

/*reads pressure value from internal bmp280 registers in pascal*/
bmp280_error_code_t bmp280_get_pressure(
	bmp280_handle_t *handle,
	uint32_t *pressure)
{
	int32_t adc_P;

	bmp280_error_code_t error = bmp280_raw_pressure_data(handle, &adc_P);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	int32_t var1, var2;

	var1 = (((int32_t)handle->t_fine) / 2) - (int32_t)64000;
	var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)handle->dig.P6);
	var2 = var2 + ((var1 * ((int32_t)handle->dig.P5)) * 2);
	var2 = (var2 / 4) + (((int32_t)handle->dig.P4) * 65536);
	var1 = (((handle->dig.P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8) + ((((int32_t)handle->dig.P2) * var1) / 2)) / 262144;
	var1 = ((((32768 + var1)) * ((int32_t)handle->dig.P1)) / 32768);
	*pressure = (uint32_t)(((int32_t)(1048576 - adc_P) - (var2 / 4096)) * 3125);

	/* Avoid exception caused by division with zero */
	if (var1 != 0)
	{
		/* Check for overflows against UINT32_MAX/2; if pres is left-shifted by 1 */
		if (*pressure < 0x80000000)
		{
			*pressure = (*pressure << 1) / ((uint32_t)var1);
		}
		else
		{
			*pressure = (*pressure / (uint32_t)var1) * 2;
		}
		var1 = (((int32_t)handle->dig.P9) * ((int32_t)(((*pressure / 8) * (*pressure / 8)) / 8192))) / 4096;
		var2 = (((int32_t)(*pressure / 4)) * ((int32_t)handle->dig.P8)) / 8192;
		*pressure = (uint32_t)((int32_t)*pressure + ((var1 + var2 + handle->dig.P7) / 16));
	}
	else
	{
		*pressure = 0;
	}

	return BMP280_ERROR_OK;
}

#if BMP280_INCLUDE_ALTITUDE
/*calculates altitude from barometric pressure without temperature as an argument*/
bmp280_error_code_t bmp280_calculate_altitude_quick(
	bmp280_handle_t *handle,
	float *alt,
	uint32_t barometricPressure)
{
	float temp_result;

	if (handle->dependency_interface.bmp280_power_function(barometricPressure, 0.190284, &temp_result) != 0)
	{
		return BMP280_ERROR_INTERFACE_MATH_POW;
	}

	*alt = 44307.69396 * (1 - 0.111555816 * temp_result); /*calculating altitude from barometric formula*/

	return BMP280_ERROR_OK;
}

/*calculates altitude from barometric pressure and temperature as arguments*/
bmp280_error_code_t bmp280_calculate_altitude_hypsometric(
	bmp280_handle_t *handle,
	float *alt,
	uint32_t barometricPressure,
	float ambientTemperatureInC)
{
	float temp_result;
	
	if (handle->dependency_interface.bmp280_power_function((float)SEA_LEVEL_PRESSURE / (float)barometricPressure, (float)1 / 5.257, &temp_result) != 0)
	{
		return BMP280_ERROR_INTERFACE_MATH_POW;
	}

	*alt = ((ambientTemperatureInC + 273.15) * (temp_result - 1)) / 0.0065;

	return BMP280_ERROR_OK;
}
#endif

#if BMP280_INCLUDE_ADDITIONAL_GETTERS
/*returns a complete set of sensor readings and altitude calculation (quick). if bmp280 is in sleep mode, gets data by setting bmp280 to forced mode then back to sleep mode*/
bmp280_error_code_t bmp280_get_all(
	bmp280_handle_t *handle,
	bmp280_sensors_data_t *data)
{
	bmp280_error_code_t error;

	if (handle->operation_mode == BMP280_MODE_FORCED)
	{
		error = bmp280_set_mode(handle, BMP280_MODE_FORCED);
		BMP280_CHECK_AND_RETURN_ERROR(error);
	}

	error = bmp280_get_temperature(handle, &data->temperature);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	error = bmp280_get_pressure(handle, &(data->pressure));
	BMP280_CHECK_AND_RETURN_ERROR(error);

	#if BMP280_INCLUDE_ALTITUDE
	error = bmp280_calculate_altitude_quick(handle, &data->altitude, data->pressure);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	#endif
	
	return BMP280_ERROR_OK;
}

/*returns bmp280 mode of operation: sleep, normal or forced*/
bmp280_error_code_t bmp280_get_mode(
	bmp280_handle_t *handle,
	bmp280_operation_mode_t *mode)
{
	uint8_t readMode;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, &readMode, BMP280_REGISTER_BIT_MODE, BMP280_REGISTER_FIELD_LENGTH_MODE);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	switch (readMode)
	{
	case 0X00:
		*mode = BMP280_MODE_SLEEP;
		break;
	case 0X01:
		*mode = BMP280_MODE_FORCED;
		break;
	case 0X02:
		*mode = BMP280_MODE_FORCED;
		break;
	case 0X03:
		*mode = BMP280_MODE_NORMAL;
		break;
	default:
		return BMP280_ERROR_MODE_UNKNOWN;
	}

	return BMP280_ERROR_OK;
}

/*returns the current temperature oversampling*/
bmp280_error_code_t bmp280_get_temperature_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t *tempOS)
{
	uint8_t readOS;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, &readOS, BMP280_REGISTER_BIT_OSRS_T, BMP280_REGISTER_FIELD_LENGTH_OSRS_T);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	if (readOS > BMP280_OVERSAMPLING_16X)
	{
		*tempOS = BMP280_OVERSAMPLING_16X;
	}
	else
	{
		*tempOS = (bmp280_over_sampling_t)readOS;
	}

	return BMP280_ERROR_OK;
}

/*returns the current pressure oversampling*/
bmp280_error_code_t bmp280_get_pressure_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t *pressureOS)
{
	uint8_t readOS;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, &readOS, BMP280_REGISTER_BIT_OSRS_P, BMP280_REGISTER_FIELD_LENGTH_OSRS_P);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	if (readOS > BMP280_OVERSAMPLING_16X)
	{
		*pressureOS = BMP280_OVERSAMPLING_16X;
	}
	else
	{
		*pressureOS = (bmp280_over_sampling_t)readOS;
	}

	return BMP280_ERROR_OK;
}

/*returns the current standby time (for normal mode)*/
bmp280_error_code_t bmp280_get_standby_time(
	bmp280_handle_t *handle,
	bmp280_standby_time_t *standby_time)
{
	uint8_t readTime;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, &readTime, BMP280_REGISTER_BIT_T_SB, BMP280_REGISTER_FIELD_LENGTH_T_SB);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	*standby_time = (bmp280_standby_time_t)readTime;

	return BMP280_ERROR_OK;
}

/*returns the current IIR filter coefficient*/
bmp280_error_code_t bmp280_get_filter_coefficient(
	bmp280_handle_t *handle,
	bmp280_iir_filter_t *filter_coeff)
{
	uint8_t readFilter;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, &readFilter, BMP280_REGISTER_BIT_FILTER, BMP280_REGISTER_FIELD_LENGTH_FILTER);
	BMP280_CHECK_AND_RETURN_ERROR(error);

	if(readFilter > (int) BMP280_FILTER_16X)
	{
		return BMP280_ERROR_FILTER_COEFF_UNKNOWN;
	}

	*filter_coeff = (bmp280_iir_filter_t)readFilter;

	return BMP280_ERROR_OK;
}

#endif

/*raw reading of temperature registers, uncompensated*/
static bmp280_error_code_t bmp280_raw_temperature_data(bmp280_handle_t *handle, int32_t *raw_data)
{
	uint8_t temp[3];

	/*reset the timeout value before the loop*/
	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	/*Polling and waiting for new data with a timeout*/
	for( uint8_t measuring_status = BMP280_MEASURING_IN_PROGRESS; measuring_status == BMP280_MEASURING_IN_PROGRESS; )
	{
		bmp280_error_code_t error;

		error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_STATUS, &measuring_status, BMP280_REGISTER_BIT_MEASURING, BMP280_REGISTER_FIELD_LENGTH_MEASURING);
		BMP280_CHECK_AND_RETURN_ERROR(error);

		if(measuring_status == BMP280_MEASURING_IN_PROGRESS)
		{
			if(handle->poll_timeout_ms < BMP280_MEASURING_POLL_PERIOD_IN_MS)
			{
				return BMP280_ERROR_SENSOR_POLL_TIMEOUT;
			}

			handle->poll_timeout_ms -= BMP280_MEASURING_POLL_PERIOD_IN_MS;

			if(handle->dependency_interface.bmp280_delay_function(BMP280_MEASURING_POLL_PERIOD_IN_MS) != 0)
			{
				return BMP280_ERROR_INTERFACE_DELAY;
			}
		}
	}


	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, BMP280_REGISTER_ADDRESS_TEMPERATURE_MSB, temp, 3) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}
	BMP280_UNLOCK(handle);

	*raw_data = (int32_t)((((uint32_t)temp[0]) << 12) + (((uint32_t)temp[1]) << 4) + (((uint32_t)temp[2]) >> 4));

	return BMP280_ERROR_OK;
}

/*raw reading of pressure registers, uncompensated*/
static bmp280_error_code_t bmp280_raw_pressure_data(bmp280_handle_t *handle, int32_t *raw_data)
{
	uint8_t pressure[3];

	/*reset the timeout value before the loop*/
	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	/*Polling and waiting for new data with a timeout*/
	for( uint8_t measuring_status = BMP280_MEASURING_IN_PROGRESS; measuring_status == BMP280_MEASURING_IN_PROGRESS; )
	{
		bmp280_error_code_t error;

		error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_STATUS, &measuring_status, BMP280_REGISTER_BIT_MEASURING, BMP280_REGISTER_FIELD_LENGTH_MEASURING);
		BMP280_CHECK_AND_RETURN_ERROR(error);

		if(measuring_status == BMP280_MEASURING_IN_PROGRESS)
		{
			if(handle->poll_timeout_ms < BMP280_MEASURING_POLL_PERIOD_IN_MS)
			{
				return BMP280_ERROR_SENSOR_POLL_TIMEOUT;
			}

			handle->poll_timeout_ms -= BMP280_MEASURING_POLL_PERIOD_IN_MS;

			if(handle->dependency_interface.bmp280_delay_function(BMP280_MEASURING_POLL_PERIOD_IN_MS) != 0)
			{
				return BMP280_ERROR_INTERFACE_DELAY;
			}
		}
	}
	
	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, BMP280_REGISTER_ADDRESS_PRESSURE_MSB, pressure, 3) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}
	BMP280_UNLOCK(handle);

	*raw_data = (int32_t)((((uint32_t)pressure[0]) << 12) + (((uint32_t)pressure[1]) << 4) + (((uint32_t)pressure[2]) >> 4));

	return BMP280_ERROR_OK;
}

/*extracting calibration data in chip's "non volatile memory". we need to do this only once for each chip*/
static bmp280_error_code_t bmp280_get_calibration(bmp280_handle_t *handle)
{
	int16_t result_signed;
	uint16_t result_unsigned;

	bmp280_error_code_t error;

	/*Getting calibration data byte by byte and saving it to the handle*/
	error = bmp280_read_calibration_word_unsigned(handle, BMP280_REGISTER_ADDRESS_T1, &result_unsigned);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.T1 = result_unsigned;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_T2, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.T2 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_T3, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.T3 = result_signed;

	error = bmp280_read_calibration_word_unsigned(handle, BMP280_REGISTER_ADDRESS_P1, &result_unsigned);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P1 = result_unsigned;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P2, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P2 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P3, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P3 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P4, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P4 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P5, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P5 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P6, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P6 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P7, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P7 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P8, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P8 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P9, &result_signed);
	BMP280_CHECK_AND_RETURN_ERROR(error);
	handle->dig.P9 = result_signed;

	return BMP280_ERROR_OK;
}

/*only used in get_calibration function*/
static bmp280_error_code_t bmp280_read_calibration_word_unsigned(
	bmp280_handle_t *handle, 
	bmp280_register_address_t startRegisterAddress, 
	uint16_t *result)
{
	uint8_t data[2];

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)startRegisterAddress, data, 2) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}
	BMP280_UNLOCK(handle);

	/*Turning two bytes of data in a two byte block of memory */
	*result = (uint16_t)((((uint16_t)data[1]) << 8) + (uint16_t)data[0]);

	return BMP280_ERROR_OK;
}

/*only used in get_calibration function*/
static bmp280_error_code_t bmp280_read_calibration_word_signed(
	bmp280_handle_t *handle,
	bmp280_register_address_t startRegisterAddress,
	int16_t *result)
{
	uint8_t data[2];

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)startRegisterAddress, data, 2) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}
	BMP280_UNLOCK(handle);

	/*Turning two bytes of data in a two byte block of memory */
	*result = (int16_t)((((int16_t)data[1]) << 8) + (int16_t)data[0]);

	return BMP280_ERROR_OK;
}

/*sets a field of bits in a register*/
static bmp280_error_code_t bmp280_set_bits_in_register(
	bmp280_handle_t *handle, 
	bmp280_register_address_t registerAddress, 
	uint8_t fieldData, 
	bmp280_register_bit_t fieldStartBitAddress, 
	bmp280_register_field_length_t fieldLength)
{
	uint8_t currentRegisterValue;
	uint8_t newRegisterValue;

	/*Reading the register of sensor, changing the needed bits, and transfering the new value to the register*/
	/*Hold the lock in the read-modify-write process*/
	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)registerAddress, &currentRegisterValue, 1) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}

	currentRegisterValue &= (((~0) << ((uint8_t)fieldStartBitAddress + (uint8_t)fieldLength)) | (~(~(0) << (uint8_t)fieldStartBitAddress)));
	newRegisterValue = currentRegisterValue | (fieldData << (uint8_t)fieldStartBitAddress);

	if (handle->dependency_interface.bmp280_write_array(handle->i2c_address, (uint8_t)registerAddress, &newRegisterValue, 1) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_WRITE;
	}
	BMP280_UNLOCK(handle);

	return BMP280_ERROR_OK;
}

/*gets a field of bits from a register*/
static bmp280_error_code_t bmp280_get_bits_in_register(
	bmp280_handle_t *handle, 
	bmp280_register_address_t registerAddress, 
	uint8_t *fieldData, 
	bmp280_register_bit_t fieldStartBitAddress, 
	bmp280_register_field_length_t fieldLength)
{
	uint8_t registerValue;

	BMP280_LOCK(handle);
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)registerAddress, &registerValue, 1) != 0)
	{
		BMP280_UNLOCK(handle);
		return BMP280_ERROR_INTERFACE_READ;
	}
	BMP280_UNLOCK(handle);

	/*Saves bits in a byte of memory*/
	registerValue &= ~(((~0) << ((uint8_t)fieldStartBitAddress + (uint8_t)fieldLength)) | (~(~(0) << (uint8_t)fieldStartBitAddress)));
	*fieldData = registerValue >> (uint8_t)fieldStartBitAddress;

	return BMP280_ERROR_OK;
}
