/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 2.0*/

#ifndef __BMP280_DEFS_H__
#define __BMP280_DEFS_H__

#ifdef __cplusplus
	#define NULL 0
#else
	#define NULL ((void *)0)
#endif

/*configurable definitions*/
static const uint16_t BMP280_STARTUP_DELAY_IN_MS = 1000;
static const uint8_t BMP280_MEASURING_POLL_TIMEOUT_IN_MS = 100; 		/*MAX: 255*/
static const uint8_t BMP280_MEASURING_POLL_PERIOD_IN_MS = 1; 			/*must not be 0 and must be less than BMP280_MEASURING_POLL_TIMEOUT_IN_MS*/
/*end of configurable definitions*/

/*constant definitions*/
static const uint8_t BMP280_DEFAULT_CHIP_ID = 0x58;
static const uint8_t BMP280_RESET_VALUE = 0xB6;

static const uint8_t BMP280_MEASURING_IN_PROGRESS = 0x01;
static const uint8_t BMP280_MEASURING_DONE = 0x00;

static const uint32_t SEA_LEVEL_PRESSURE = 101325;

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

/*end of constant definitions*/

typedef enum
{
	OVERSAMPLING_0X = 0X00,
	OVERSAMPLING_1X = 0X01,
	OVERSAMPLING_2X = 0X02,
	OVERSAMPLING_4X = 0X03,
	OVERSAMPLING_8X = 0X04,
	OVERSAMPLING_16X = 0X05
} bmp280_over_sampling_t;

typedef enum {
	BMP280_I2C,
	BMP280_SPI
} bmp280_hardware_interface_t;

typedef enum
{
	I2C_ADDRESS_NONE = 0,
	I2C_ADDRESS_1 = 0X76,
	I2C_ADDRESS_2 = 0X77
} bmp280_i2c_address_t;

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

typedef enum
{
	MODE_SLEEP = 0x00,
	MODE_FORCED = 0x01,
	MODE_NORMAL = 0x03
} bmp280_operation_mode_t;

typedef enum
{
	FILTER_OFF = 0X00,
	FILTER_2X,
	FILTER_4X,
	FILTER_8X,
	FILTER_16X
} bmp280_iir_filter_t;

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

/*This struct is needed for get_all function*/
typedef struct
{
	float temperature;
	uint32_t pressure;
	float altitude;
} bmp280_sensors_data_t;

static const bmp280_over_sampling_t BMP280_PRESSURE_OVERSAMPLING_DEFAULT = OVERSAMPLING_1X;
static const bmp280_over_sampling_t BMP280_TEMPERATURE_OVERSAMPLING_DEFAULT = OVERSAMPLING_1X;
static const bmp280_standby_time_t BMP280_STANDBY_TIME_DEFAULT = T_STANDBY_125MS;
static const bmp280_iir_filter_t BMP280_FILTER_DEFAULT = FILTER_OFF;
static const bmp280_operation_mode_t BMP280_MODE_DEFAULT = MODE_SLEEP;

#endif
