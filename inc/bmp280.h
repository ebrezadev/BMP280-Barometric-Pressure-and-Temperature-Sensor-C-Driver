/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 2.0*/

#ifndef __BMP280_H__
#define __BMP280_H__

#include "bmp280_definitions.h"
#include "bmp280_error.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*Implements the interface (or optionally chip power) initializer, whether I2c, SPI or test mock.*/
typedef int (*bmp280_interface_init_fp)(void);
/*Implements the interface (or optionally chip power) de-initializer, whether I2c, SPI or test mock.*/
typedef int (*bmp280_interface_deinit_fp)(void);
/*Implements a delay function in milliseconds, could be blocking or non blocking based on context*/
typedef int (*bmp280_delay_function_fp)(uint32_t delayMS);
/*Implements a power function (used in altitude calculation. returns 0 for no error.*/
typedef int (*bmp280_power_function_fp) (float x, float y, float *result);
/*Implements the interface write function.*/
typedef int (*bmp280_write_array_fp)(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);
/*Implements the interface read function.*/
typedef int (*bmp280_read_array_fp)(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);

/*The dependency interface for the BMP280 handle. MUST be set correctly in order to work.*/
typedef struct
{
	bmp280_interface_init_fp bmp280_interface_init;
	bmp280_interface_deinit_fp bmp280_interface_deinit;
	bmp280_delay_function_fp bmp280_delay_function;
	bmp280_power_function_fp bmp280_power_function;
	bmp280_write_array_fp bmp280_write_array;
	bmp280_read_array_fp bmp280_read_array;
} bmp280_dependency_t;

/*The handle to an instance of BMP280 sensor. Please set the correct dependency interface.*/
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

/*Checks for correct handle, checks for correct interface and address, resets and initializes sensor, gets the calibration data for further calculations. also sets the default values.*/
bmp280_error_code_t bmp280_init(
	bmp280_handle_t *handle, 
	bmp280_hardware_interface_t hw_interface, 
	bmp280_i2c_address_t i2c_address);

/*checks chip id to see if it really is a bmp280 module with correct wiring and address*/
bmp280_error_code_t bmp280_check_id(bmp280_handle_t *bmp280_handle);

/*soft resets bm280 using special reset register*/
bmp280_error_code_t bmp280_reset(bmp280_handle_t *handle);

/*setting bmp280 mode, NORMAL_MODE, FORCED_MODE, SLEEP_MODE*/
bmp280_error_code_t bmp280_set_mode(
	bmp280_handle_t *handle, 
	bmp280_operation_mode_t operationMode);

/*setting pressure oversampling from 0 (skip) to 16x*/
bmp280_error_code_t bmp280_set_pressure_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t osValue);

/*setting temperature oversampling from 0 (skip) to 16x*/
bmp280_error_code_t bmp280_set_temperature_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t osValue);

/*sets standby time between measurements in normal mode.*/
bmp280_error_code_t bmp280_set_standby_time(
	bmp280_handle_t *handle, 
	bmp280_standby_time_t standbyTime);

/*sets low pass internal filter coefficient for bmp280*/
bmp280_error_code_t bmp280_set_filter_coefficient(
	bmp280_handle_t *handle, 
	bmp280_iir_filter_t filterCoefficient);

/*one function to completely set up bmp280 mode, temperature and pressure oversampling, normal mode stadby time and IIR filter coefficient*/
bmp280_error_code_t bmp280_set(
	bmp280_handle_t *handle, 
	bmp280_operation_mode_t operationMode, 
	bmp280_over_sampling_t tempOS, 
	bmp280_over_sampling_t pressureOS, 
	bmp280_standby_time_t standbyTime, 
	bmp280_iir_filter_t filterCoefficient);

/*reads temperature value from internal bmp280 registers in centigrade*/
bmp280_error_code_t bmp280_get_temperature(
	bmp280_handle_t *handle, 
	int32_t *temperature);

/*reads pressure value from internal bmp280 registers in pascal*/
bmp280_error_code_t bmp280_get_pressure(
	bmp280_handle_t *handle, 
	uint32_t *pressure);

/*calculates altitude from barometric pressure without temperature as an argument*/
bmp280_error_code_t bmp280_calculate_altitude_quick(
	bmp280_handle_t *handle, 
	float *alt, 
	uint32_t barometricPressure);

/*calculates altitude from barometric pressure and temperature as arguments*/
bmp280_error_code_t bmp280_calculate_altitude_hypsometric(
	bmp280_handle_t *handle, 
	float *alt, 
	uint32_t barometricPressure, 
	float ambientTemperatureInC);

/*returns a complete set of sensor readings and altitude calculation (quick).*/
bmp280_error_code_t bmp280_get_all(
	bmp280_handle_t *handle, 
	bmp280_sensors_data_t *data);

/*returns bmp280 mode of operation: sleep, normal or forced*/
bmp280_error_code_t bmp280_get_mode(
	bmp280_handle_t *handle, 
	bmp280_operation_mode_t *mode);

/*returns the current temperature oversampling*/
bmp280_error_code_t bmp280_get_temperature_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t *tempOS);

/*returns the current pressure oversampling*/
bmp280_error_code_t bmp280_get_pressure_oversampling(
	bmp280_handle_t *handle, 
	bmp280_over_sampling_t *pressureOS);

/*returns the current standby time (for normal mode)*/
bmp280_error_code_t bmp280_get_standby_time(
	bmp280_handle_t *handle, 
	bmp280_standby_time_t *standby_time);

/*returns the current IIR filter coefficient*/
bmp280_error_code_t bmp280_get_filter_coefficient(
	bmp280_handle_t *handle, 
	bmp280_iir_filter_t *filter_coeff);

#ifdef __cplusplus
}
#endif

#endif
