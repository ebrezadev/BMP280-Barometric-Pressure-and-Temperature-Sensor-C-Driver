/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 2.0*/

#ifndef __BMP280_ERROR_H__
#define __BMP280_ERROR_H__

typedef enum {
	ERROR_OK,		/*no error*/
	ERROR_INTERFACE,		/*error in interface*/
	ERROR_NULL_DEPENDENCY,		/*error in dependency functions*/
	ERROR_SENSOR_ID,		/*error in sensor ID*/
	ERROR_NULL_HANDLE,		/*error in null handle pointer*/
	ERROR_TIMEOUT,		/*error in polling timeout. please select a higher timeout*/
	ERROR_UNKNOWN		/*unknown error*/
} bmp280_error_code_t;

#endif