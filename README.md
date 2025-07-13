# BMP280 Barometric Pressure and Temperature Sensor C Device Driver (Platform Independent)
* version 2.0
* Reza Ebrahimi

BMP280 is an absolute barometric pressure sensor especially designed by Bosch Sensortec for mobile applications. It consists of a Piezo-resistive pressure sensing element and a mixed-signal ASIC. The ASIC performs A/D conversions and provides the conversion results and sensor specific compensation data through a digital interface. This library is written in **portable C**, and is **MCU independent**. .

BMP280 C library abstracts away the internals of the hardware, using high level functions (description below). BMP280 is capable of interfacing with SPI and I2C.

This library is based on the 'BMP280 Digital Pressure Sensor Datsheet' V. 1.23.

## GENERAL INFORMATION

BMP280 consists of two sensors on one module: temperature and barometric pressure. The barometric pressure value needs to be calibrated and compensated using temperature data, so a reading of temperature must be done prior to a pressure reading.

The BMP280 has three modes of operation (Normal, Forced and Sleep), an optional IIR filter, optional timing in normal mode (continuous reading) and optional oversampling.

It can also be used to calculate altimeter based on barometric pressure. Two functions are provided for this purpose.

## Version 2.0

There are many design changes to the version 1.0, including:
- Device driver handle design pattern for unlimited instances of sensors
- Dependency injection pattern for interface
- Stateless and reentrant
- Possible to be used with baremetal, RTOS or under Linux
- Error handling baked into the driver

## HOW TO USE

Start by defining a handle:
```c
bmp280_handle_t BMP280;
```
Please pay attention to the scope and life cycle of the handle. The next step is to implement all the interface dependencies; As an example:
```c
BMP280.dependency_interface.bmp280_interface_init = implementation_of_interface_init;
BMP280.dependency_interface.bmp280_interface_deinit = implementation_of_interface_deinit;
BMP280.dependency_interface.bmp280_write_array = implementation_of_write_array;
BMP280.dependency_interface.bmp280_read_array = implementation_of_read_array;
BMP280.dependency_interface.bmp280_delay_function = implementation_of_delay_function;
BMP280.dependency_interface.bmp280_power_function = implementation_of_power_function;
```
Here you are free to implement the interface and set it up to be used with I2C, SPI or even mock an interface. These 'int implementation_of_...(...)' functions should be provided by the application writer. All functions of the interface must return 0 in case of no error. the 'address' parameter in the interface is used solely in case of I2C and is not used in other cases. Once set up correctly, the initializer can be called. As an example for I2C interface:
```c
bmp280_error_code_t error = bmp280_init(&BMP280, BMP280_I2C, I2C_ADDRESS_1);

  if(error != ERROR_OK)
  {
    printf("FAIL");

    error_handler();
  }

  printf("SUCCESS");
```
All functions return their error status and its application writer's responsibility to check the return values. Errors are of custom type 'bmp280_error_code_t' and include:
```c
  ERROR_OK 
  ERROR_INTERFACE  
  ERROR_NULL_DEPENDENCY 
  ERROR_SENSOR_ID
  ERROR_NULL_HANDLE
  ERROR_TIMEOUT
  ERROR_UNKNOWN
```
In which ERROR_OK means no errors. Now you can set the mode of operation, oversampling rate for temperature or pressure etc. In order to read all he data at once:
```c
bmp280_sensors_data_t sensorsData;

error = bmp280_get_all(&BMP280, &sensorsData);

if(error != ERROR_OK)
{
  printf("FAIL");

  error_handler();
}
```
Optionally, you can calculate the altimeter using one of 2 functions provided:
```c
float altitudeHypsometric;

error = bmp280_calculate_altitude_hypsometric(&BMP280, &altitudeHypsometric, sensorsData.pressure, sensorsData.temperature);
```


