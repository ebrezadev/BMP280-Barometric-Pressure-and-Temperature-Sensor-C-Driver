#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>

/*Default bus address in case of no env variable. Works with RPi.*/
static const char *default_i2c_bus_address = "/dev/i2c-1";

int bmp280_i2c_init(uint8_t deviceAddress);
int bmp280_i2c_deinit(uint8_t deviceAddress);
int bmp280_read_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);
int bmp280_write_array(uint8_t deviceAddress, uint8_t startRegisterAddress, uint8_t *data, uint8_t dataLength);
int delay_function(uint32_t delayMS);
int power_function(float x, float y, float *result);

#endif
