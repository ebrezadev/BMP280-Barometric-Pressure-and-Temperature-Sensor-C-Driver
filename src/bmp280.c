/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 2.0*/

#include "bmp280.h"

#define CHECK_AND_RETURN_ERROR(error) \
    do { \
        if (error != ERROR_OK) \
		{ \
			return error; \
		} \
    } while(0)

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
	if (handle == NULL)
	{
		return ERROR_NULL_HANDLE;
	}

	/*check for NULL or undefined dependencies*/
	if (handle->dependency_interface.bmp280_delay_function == NULL ||
		handle->dependency_interface.bmp280_interface_init == NULL ||
		handle->dependency_interface.bmp280_interface_deinit == NULL ||
		handle->dependency_interface.bmp280_power_function == NULL ||
		handle->dependency_interface.bmp280_read_array == NULL ||
		handle->dependency_interface.bmp280_write_array == NULL)
	{
		return ERROR_NULL_DEPENDENCY;
	}

	/*initialize the hardware interface*/
	if (handle->dependency_interface.bmp280_interface_init() != 0)
	{
		return ERROR_INTERFACE;
	}

	handle->hardware_interface = hw_interface;

	/*check if the hardware interface is set to I2C*/
	if (hw_interface == BMP280_I2C)
	{
		handle->i2c_address = i2c_address;
	}
	else
	{
		handle->i2c_address = I2C_ADDRESS_NONE;
	}

	/*check if the driver can connect to sensor hardware by checking its ID*/
	error = bmp280_check_id(handle);

	if (error != ERROR_OK)
	{
		handle->i2c_address = I2C_ADDRESS_NONE;
		return error;
	}

	error = bmp280_reset(handle);

	CHECK_AND_RETURN_ERROR(error);

	if (handle->dependency_interface.bmp280_delay_function(BMP280_STARTUP_DELAY_IN_MS) != 0)
	{
		return ERROR_INTERFACE;
	}

	error = bmp280_get_calibration(handle);

	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set(
		handle,
		BMP280_MODE_DEFAULT,
		BMP280_TEMPERATURE_OVERSAMPLING_DEFAULT,
		BMP280_PRESSURE_OVERSAMPLING_DEFAULT,
		BMP280_STANDBY_TIME_DEFAULT,
		BMP280_FILTER_DEFAULT);

	CHECK_AND_RETURN_ERROR(error);

	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	return ERROR_OK;
}

bmp280_error_code_t bmp280_deinit(bmp280_handle_t *handle)
{
	bmp280_error_code_t error = bmp280_reset(handle);

	CHECK_AND_RETURN_ERROR(error);

	if(handle->dependency_interface.bmp280_interface_deinit() != 0)
	{
		return ERROR_INTERFACE;
	}

	handle->operation_mode = MODE_SLEEP;

	return ERROR_OK;
}

/*checks chip id to see if it really is a bmp280 module with correct wiring and address*/
bmp280_error_code_t bmp280_check_id(bmp280_handle_t *handle)
{
	uint8_t data;

	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, BMP280_REGISTER_ADDRESS_ID, &data, 1) != 0)
	{
		return ERROR_INTERFACE;
	}

	if (data == BMP280_DEFAULT_CHIP_ID)
	{
		return ERROR_OK;
	}
	else
	{
		return ERROR_SENSOR_ID;
	}
}

/*soft resets bm280 using special reset register*/
bmp280_error_code_t bmp280_reset(bmp280_handle_t *handle)
{
	uint8_t data = BMP280_RESET_VALUE;

	if (handle->dependency_interface.bmp280_write_array((uint8_t)handle->i2c_address, BMP280_REGISTER_ADDRESS_RESET, &data, 1) != 0)
	{
		return ERROR_INTERFACE;
	}

	handle->operation_mode = MODE_SLEEP;

	return ERROR_OK;
}

/*setting bmp280 mode, NORMAL_MODE, FORCED_MODE, SLEEP_MODE*/
bmp280_error_code_t bmp280_set_mode(
	bmp280_handle_t *handle,
	bmp280_operation_mode_t operationMode)
{
	bmp280_error_code_t error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, operationMode, BMP280_REGISTER_BIT_MODE, BMP280_REGISTER_FIELD_LENGTH_MODE);

	CHECK_AND_RETURN_ERROR(error);

	handle->operation_mode = operationMode;

	return ERROR_OK;
}

/*setting pressure oversampling from 0 (skip) to 16x*/
bmp280_error_code_t bmp280_set_pressure_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t osValue)
{
	/*In order to change configuration, the BMP280 sensor must be in SLEEP mode*/
	bmp280_error_code_t error = bmp280_set_mode(handle, MODE_SLEEP);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, osValue, BMP280_REGISTER_BIT_OSRS_P, BMP280_REGISTER_FIELD_LENGTH_OSRS_P);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, handle->operation_mode);
	CHECK_AND_RETURN_ERROR(error);

	return ERROR_OK;
}

/*setting temperature oversampling from 0 (skip) to 16x*/
bmp280_error_code_t bmp280_set_temperature_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t osValue)
{
	bmp280_error_code_t error = bmp280_set_mode(handle, MODE_SLEEP);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, osValue, BMP280_REGISTER_BIT_OSRS_T, BMP280_REGISTER_FIELD_LENGTH_OSRS_T);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, handle->operation_mode);
	CHECK_AND_RETURN_ERROR(error);

	return ERROR_OK;
}

/*sets standby time between measurements in normal mode. lower standby time means higher power consumption*/
bmp280_error_code_t bmp280_set_standby_time(
	bmp280_handle_t *handle,
	bmp280_standby_time_t standbyTime)
{
	bmp280_error_code_t error = bmp280_set_mode(handle, MODE_SLEEP);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, standbyTime, BMP280_REGISTER_BIT_T_SB, BMP280_REGISTER_FIELD_LENGTH_T_SB);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, handle->operation_mode);
	CHECK_AND_RETURN_ERROR(error);

	return ERROR_OK;
}

/*sets low pass internal filter coefficient for bmp280. used in noisy environments*/
bmp280_error_code_t bmp280_set_filter_coefficient(
	bmp280_handle_t *handle,
	bmp280_iir_filter_t filterCoefficient)
{
	bmp280_error_code_t error = bmp280_set_mode(handle, MODE_SLEEP);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, filterCoefficient, BMP280_REGISTER_BIT_FILTER, BMP280_REGISTER_FIELD_LENGTH_FILTER);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_mode(handle, handle->operation_mode);
	CHECK_AND_RETURN_ERROR(error);

	return ERROR_OK;
}

/*one function to completely set up bmp280 mode, temperature and pressure oversampling, normal mode stadby time and IIR filter coefficient*/
bmp280_error_code_t bmp280_set(
	bmp280_handle_t *handle,
	bmp280_operation_mode_t operationMode,
	bmp280_over_sampling_t tempOS,
	bmp280_over_sampling_t pressureOS,
	bmp280_standby_time_t standbyTime,
	bmp280_iir_filter_t filterCoefficient)
{
	/*In order to change configuration, the BMP280 sensor must be in SLEEP mode*/
	bmp280_error_code_t error = bmp280_set_mode(handle, MODE_SLEEP);
	CHECK_AND_RETURN_ERROR(error);

	if (handle->dependency_interface.bmp280_delay_function(50) != 0)
	{
		return ERROR_INTERFACE;
	}

	error = bmp280_set_pressure_oversampling(handle, pressureOS);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_temperature_oversampling(handle, tempOS);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_standby_time(handle, standbyTime);
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_set_filter_coefficient(handle, filterCoefficient);
	CHECK_AND_RETURN_ERROR(error);

	/*After configuration, we can change the operation mode*/
	error = bmp280_set_mode(handle, operationMode);
	CHECK_AND_RETURN_ERROR(error);

	return ERROR_OK;
}

/*reads temperature value from internal bmp280 registers in centigrade*/
bmp280_error_code_t bmp280_get_temperature(
	bmp280_handle_t *handle,
	int32_t *temperature)
{
	int32_t adc_T;

	bmp280_error_code_t error = bmp280_raw_temperature_data(handle, &adc_T);

	CHECK_AND_RETURN_ERROR(error);

	int32_t var1, var2;

	var1 = ((((adc_T >> 3) - ((int32_t)handle->dig.T1 << 1))) * ((int32_t)handle->dig.T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)handle->dig.T1)) * ((adc_T >> 4) - ((int32_t)handle->dig.T1))) >> 12) * ((int32_t)handle->dig.T3)) >> 14;

	handle->t_fine = var1 + var2;
	*temperature = (handle->t_fine * 5 + 128) >> 8;

	return ERROR_OK;
}

/*reads pressure value from internal bmp280 registers in pascal*/
bmp280_error_code_t bmp280_get_pressure(
	bmp280_handle_t *handle,
	uint32_t *pressure)
{
	int32_t adc_P;

	bmp280_error_code_t error = bmp280_raw_pressure_data(handle, &adc_P);

	CHECK_AND_RETURN_ERROR(error);

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

	return ERROR_OK;
}

/*calculates altitude from barometric pressure without temperature as an argument*/
bmp280_error_code_t bmp280_calculate_altitude_quick(
	bmp280_handle_t *handle,
	float *alt,
	uint32_t barometricPressure)
{
	float temp_result;

	if (handle->dependency_interface.bmp280_power_function(barometricPressure, 0.190284, &temp_result) != 0)
	{
		return ERROR_INTERFACE;
	}

	*alt = 44307.69396 * (1 - 0.111555816 * temp_result); /*calculating altitude from barometric formula*/

	return ERROR_OK;
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
		return ERROR_INTERFACE;
	}

	*alt = ((ambientTemperatureInC + 273.15) * (temp_result - 1)) / 0.0065;

	return ERROR_OK;
}

/*returns a complete set of sensor readings and altitude calculation (quick). if bmp280 is in sleep mode, gets data by setting bmp280 to forced mode then back to sleep mode*/
bmp280_error_code_t bmp280_get_all(
	bmp280_handle_t *handle,
	bmp280_sensors_data_t *data)
{
	bmp280_error_code_t error;

	if (handle->operation_mode == MODE_FORCED)
	{
		error = bmp280_set_mode(handle, MODE_FORCED);
		CHECK_AND_RETURN_ERROR(error);
	}

	int32_t temp_temperature;

	error = bmp280_get_temperature(handle, &temp_temperature);
	CHECK_AND_RETURN_ERROR(error);

	data->temperature = ((float)temp_temperature) / 100.0;

	error = bmp280_get_pressure(handle, &(data->pressure));
	CHECK_AND_RETURN_ERROR(error);

	error = bmp280_calculate_altitude_quick(handle, &data->altitude, data->pressure);
	CHECK_AND_RETURN_ERROR(error);

	return ERROR_OK;
}

/*returns bmp280 mode of operation: sleep, normal or forced*/
bmp280_error_code_t bmp280_get_mode(
	bmp280_handle_t *handle,
	bmp280_operation_mode_t *mode)
{
	uint8_t readMode;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, &readMode, BMP280_REGISTER_BIT_MODE, BMP280_REGISTER_FIELD_LENGTH_MODE);
	CHECK_AND_RETURN_ERROR(error);

	switch (readMode)
	{
	case 0X00:
		*mode = MODE_SLEEP;
		break;
	case 0X01:
		*mode = MODE_FORCED;
		break;
	case 0X02:
		*mode = MODE_FORCED;
		break;
	case 0X03:
		*mode = MODE_NORMAL;
		break;
	default:
		return ERROR_UNKNOWN;
	}

	return ERROR_OK;
}

/*returns the current temperature oversampling*/
bmp280_error_code_t bmp280_get_temperature_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t *tempOS)
{
	uint8_t readOS;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, &readOS, BMP280_REGISTER_BIT_OSRS_T, BMP280_REGISTER_FIELD_LENGTH_OSRS_T);
	CHECK_AND_RETURN_ERROR(error);

	if (readOS > OVERSAMPLING_16X)
	{
		*tempOS = OVERSAMPLING_16X;
	}
	else
	{
		*tempOS = (bmp280_over_sampling_t)readOS;
	}

	return ERROR_OK;
}

/*returns the current pressure oversampling*/
bmp280_error_code_t bmp280_get_pressure_oversampling(
	bmp280_handle_t *handle,
	bmp280_over_sampling_t *pressureOS)
{
	uint8_t readOS;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONTROL_MEAS, &readOS, BMP280_REGISTER_BIT_OSRS_P, BMP280_REGISTER_FIELD_LENGTH_OSRS_P);
	CHECK_AND_RETURN_ERROR(error);

	if (readOS > OVERSAMPLING_16X)
	{
		*pressureOS = OVERSAMPLING_16X;
	}
	else
	{
		*pressureOS = (bmp280_over_sampling_t)readOS;
	}

	return ERROR_OK;
}

/*returns the current standby time (for normal mode)*/
bmp280_error_code_t bmp280_get_standby_time(
	bmp280_handle_t *handle,
	bmp280_standby_time_t *standby_time)
{
	uint8_t readTime;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, &readTime, BMP280_REGISTER_BIT_T_SB, BMP280_REGISTER_FIELD_LENGTH_T_SB);
	CHECK_AND_RETURN_ERROR(error);

	*standby_time = (bmp280_standby_time_t)readTime;

	return ERROR_OK;
}

/*returns the current IIR filter coefficient*/
bmp280_error_code_t bmp280_get_filter_coefficient(
	bmp280_handle_t *handle,
	bmp280_iir_filter_t *filter_coeff)
{
	uint8_t readFilter;
	bmp280_error_code_t error;

	error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_CONFIG, &readFilter, BMP280_REGISTER_BIT_FILTER, BMP280_REGISTER_FIELD_LENGTH_FILTER);
	CHECK_AND_RETURN_ERROR(error);

	if(readFilter > (int) FILTER_16X)
	{
		return ERROR_UNKNOWN;
	}

	*filter_coeff = (bmp280_iir_filter_t)readFilter;

	return ERROR_OK;
}

/*raw reading of temperature registers, uncompensated*/
static bmp280_error_code_t bmp280_raw_temperature_data(bmp280_handle_t *handle, int32_t *raw_data)
{
	uint8_t temp[3];

	for( uint8_t measuring_status = BMP280_MEASURING_IN_PROGRESS; measuring_status == BMP280_MEASURING_IN_PROGRESS; )
	{
		bmp280_error_code_t error;

		error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_STATUS, &measuring_status, BMP280_REGISTER_BIT_MEASURING, BMP280_REGISTER_FIELD_LENGTH_MEASURING);
		CHECK_AND_RETURN_ERROR(error);

		if(measuring_status == BMP280_MEASURING_IN_PROGRESS)
		{
			if(handle->poll_timeout_ms < BMP280_MEASURING_POLL_PERIOD_IN_MS)
			{
				return ERROR_TIMEOUT;
			}

			handle->poll_timeout_ms -= BMP280_MEASURING_POLL_PERIOD_IN_MS;

			if(handle->dependency_interface.bmp280_delay_function(BMP280_MEASURING_POLL_PERIOD_IN_MS) != 0)
			{
				return ERROR_INTERFACE;
			}
		}
	}
	
	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, BMP280_REGISTER_ADDRESS_TEMPERATURE_MSB, temp, 3) != 0)
	{
		return ERROR_INTERFACE;
	}

	*raw_data = (int32_t)((((uint32_t)temp[0]) << 12) + (((uint32_t)temp[1]) << 4) + (((uint32_t)temp[2]) >> 4));

	return ERROR_OK;
}

/*raw reading of pressure registers, uncompensated*/
static bmp280_error_code_t bmp280_raw_pressure_data(bmp280_handle_t *handle, int32_t *raw_data)
{
	uint8_t pressure[3];

	for( uint8_t measuring_status = BMP280_MEASURING_IN_PROGRESS; measuring_status == BMP280_MEASURING_IN_PROGRESS; )
	{
		bmp280_error_code_t error;

		error = bmp280_get_bits_in_register(handle, BMP280_REGISTER_ADDRESS_STATUS, &measuring_status, BMP280_REGISTER_BIT_MEASURING, BMP280_REGISTER_FIELD_LENGTH_MEASURING);
		CHECK_AND_RETURN_ERROR(error);

		if(measuring_status == BMP280_MEASURING_IN_PROGRESS)
		{
			if(handle->poll_timeout_ms < BMP280_MEASURING_POLL_PERIOD_IN_MS)
			{
				return ERROR_TIMEOUT;
			}

			handle->poll_timeout_ms -= BMP280_MEASURING_POLL_PERIOD_IN_MS;

			if(handle->dependency_interface.bmp280_delay_function(BMP280_MEASURING_POLL_PERIOD_IN_MS) != 0)
			{
				return ERROR_INTERFACE;
			}
		}
	}
	
	handle->poll_timeout_ms = BMP280_MEASURING_POLL_TIMEOUT_IN_MS;

	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, BMP280_REGISTER_ADDRESS_PRESSURE_MSB, pressure, 3) != 0)
	{
		return ERROR_INTERFACE;
	}

	*raw_data = (int32_t)((((uint32_t)pressure[0]) << 12) + (((uint32_t)pressure[1]) << 4) + (((uint32_t)pressure[2]) >> 4));

	return ERROR_OK;
}

/*extracting calibration data in chip's "non volatile memory". we need to do this only once for each chip*/
static bmp280_error_code_t bmp280_get_calibration(bmp280_handle_t *handle)
{
	int16_t result_signed;
	uint16_t result_unsigned;

	bmp280_error_code_t error;

	error = bmp280_read_calibration_word_unsigned(handle, BMP280_REGISTER_ADDRESS_T1, &result_unsigned);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.T1 = result_unsigned;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_T2, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.T2 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_T3, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.T3 = result_signed;

	error = bmp280_read_calibration_word_unsigned(handle, BMP280_REGISTER_ADDRESS_P1, &result_unsigned);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P1 = result_unsigned;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P2, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P2 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P3, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P3 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P4, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P4 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P5, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P5 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P6, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P6 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P7, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P7 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P8, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P8 = result_signed;

	error = bmp280_read_calibration_word_signed(handle, BMP280_REGISTER_ADDRESS_P9, &result_signed);
	CHECK_AND_RETURN_ERROR(error);
	handle->dig.P9 = result_signed;

	return ERROR_OK;
}

/*only used in get_calibration function*/
static bmp280_error_code_t bmp280_read_calibration_word_unsigned(
	bmp280_handle_t *handle, 
	bmp280_register_address_t startRegisterAddress, 
	uint16_t *result)
{
	uint8_t data[2];
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)startRegisterAddress, data, 2) != 0)
	{
		return ERROR_INTERFACE;
	}

	*result = (uint16_t)((((uint16_t)data[1]) << 8) + (uint16_t)data[0]);

	return ERROR_OK;
}

/*only used in get_calibration function*/
static bmp280_error_code_t bmp280_read_calibration_word_signed(
	bmp280_handle_t *handle,
	bmp280_register_address_t startRegisterAddress,
	int16_t *result)
{
	uint8_t data[2];
	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)startRegisterAddress, data, 2) != 0)
	{
		return ERROR_INTERFACE;
	}

	*result = (int16_t)((((int16_t)data[1]) << 8) + (int16_t)data[0]);

	return ERROR_OK;
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

	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)registerAddress, &currentRegisterValue, 1) != 0)
	{
		return ERROR_INTERFACE;
	}

	currentRegisterValue &= (((~0) << ((uint8_t)fieldStartBitAddress + (uint8_t)fieldLength)) | (~(~(0) << (uint8_t)fieldStartBitAddress)));
	newRegisterValue = currentRegisterValue | (fieldData << (uint8_t)fieldStartBitAddress);

	if (handle->dependency_interface.bmp280_write_array(handle->i2c_address, (uint8_t)registerAddress, &newRegisterValue, 1) != 0)
	{
		return ERROR_INTERFACE;
	}

	return ERROR_OK;
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

	if (handle->dependency_interface.bmp280_read_array(handle->i2c_address, (uint8_t)registerAddress, &registerValue, 1) != 0)
	{
		return ERROR_INTERFACE;
	}

	registerValue &= ~(((~0) << ((uint8_t)fieldStartBitAddress + (uint8_t)fieldLength)) | (~(~(0) << (uint8_t)fieldStartBitAddress)));
	*fieldData = registerValue >> (uint8_t)fieldStartBitAddress;

	return ERROR_OK;
}
