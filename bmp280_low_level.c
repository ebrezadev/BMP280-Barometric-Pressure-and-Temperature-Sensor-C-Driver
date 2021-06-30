/*BMP280 barometric pressure and temperature sensor C Driver*/
/*Reza Ebrahimi - https://github.com/ebrezadev */
/*Version 1.0*/

#include "bmp280.h"

/*writes an array (data[]) of arbitrary size (dataLength) to I2C address (deviceAddress), starting from an internal register address (startRegisterAddress)*/
void bmp280_write_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength)
{
  
}

/*reads an array (data[]) of arbitrary size (dataLength) from I2C address (deviceAddress), starting from an internal register address (startRegisterAddress)*/
void bmp280_read_array (uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength)
{
  
}

/*initiates the I2C peripheral and sets its speed*/
void bmp280_i2c_init()        
{
  
}

/*a delay function for milliseconds delay*/
void delay_function (uint32_t delayMS)
{
  
}

/*implements a power function (used in altitude calculation)*/
float power_function (float x, float y)
{
  
}
