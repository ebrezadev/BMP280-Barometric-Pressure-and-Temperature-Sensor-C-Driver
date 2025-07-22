/**
* @file	bmp280_config.h
* @brief BMP280 barometric pressure and temperature sensor C Driver
* @author Reza G. Ebrahimi <https://github.com/ebrezadev>
* @version 5.0
* @Section HOW-TO-USE
* This is the configuration file for BMP280 device driver. There are
* 3 config constants and 3 macros.
* If you want to use the altitude calculations, add:
*	@code 
*	#define BMP280_INCLUDE_ALTITUDE	1
*	@endcode
* If you want to use the getters, add:
*	@code 
*	#define BMP280_INCLUDE_ADDITIONAL_GETTERS 1
*	@endcode
* If you want to provide callback functions for mutual exclusion:
	@code
	#define BMP280_INCLUDE_EXCLUSION_HOOK 1
	@endcode

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

#ifndef __BMP280_CONFIG_H__
#define __BMP280_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*macros*/
#define BMP280_INCLUDE_ALTITUDE	1
#define BMP280_INCLUDE_ADDITIONAL_GETTERS 1
#define BMP280_INCLUDE_EXCLUSION_HOOK 1

/*config constants*/
static const uint16_t BMP280_STARTUP_DELAY_IN_MS = 2000;
static const uint8_t BMP280_MEASURING_POLL_TIMEOUT_IN_MS = 100; 		/*MAX: 255*/
static const uint8_t BMP280_MEASURING_POLL_PERIOD_IN_MS = 1; 			/*must not be 0 and must be less than BMP280_MEASURING_POLL_TIMEOUT_IN_MS*/

#ifdef __cplusplus
}
#endif

#endif
