# BMP280 Barometric Pressure and Temperature Sensor C Driver
* version 1.0
* Reza Ebrahimi

BMP280 is an absolute barometric pressure sensor especially designed by Bosch Sensortec for mobile applications. The BMP280 consists of a Piezo-resistive pressure sensing element and a mixed-signal ASIC. The ASIC performs A/D conversions and provides the conversion results and sensor specific compensation data througha digital interface. This library is written in **portable C**, and is **MCU independent**. In order to implement it to your MCU of choice, you need to manipulate functions inside bmp280_low_level.c file (I2C configurations, delay function and math operations) and leave other files as they are.
