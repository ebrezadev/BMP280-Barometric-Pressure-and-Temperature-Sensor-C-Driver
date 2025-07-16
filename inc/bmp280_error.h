/**
* @file	bmp280_error.h
* @brief BMP280 barometric pressure and temperature sensor C Driver
* @author Reza G. Ebrahimi <https://github.com/ebrezadev>
* @version 4.0
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

#ifndef __BMP280_ERROR_H__
#define __BMP280_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The return errors for BMP280 device driver API
 * 
 */
typedef enum {
	BMP280_ERROR_OK = 0,		/*no error*/
	BMP280_ERROR_INTERFACE,		/*error in interface*/
	BMP280_ERROR_NULL_INTERFACE,		/*error in dependency functions*/
	BMP280_ERROR_SENSOR_ID,		/*error in sensor ID*/
	BMP280_ERROR_NULL_HANDLE,		/*error in null handle pointer*/
	BMP280_ERROR_TIMEOUT,		/*error in polling timeout. please select a higher timeout*/
	BMP280_ERROR_EXCLUSION,		/*error in mutex hooks */
	BMP280_ERROR_NULL_MUTEX_HANDLE,		/*handle to mutex does not exist*/
	BMP280_ERROR_UNKNOWN		/*unknown error*/
} bmp280_error_code_t;

#ifdef __cplusplus
}
#endif

#endif
