//Bosch Sensortec BMP280 barometric pressure and temperature sensor
//example implementation for arduino
#include "bmp280.h"
#include "interface.h"

bmp280_sensors_data_t sensorsData;
bmp280_handle_t handle;

void setup() {
  Serial.begin(115200);

  delay(1000);

  handle.dependency_interface.bmp280_interface_init = bmp280_i2c_init;
  handle.dependency_interface.bmp280_interface_deinit = bmp280_i2c_deinit;
  handle.dependency_interface.bmp280_write_array = bmp280_write_array;
  handle.dependency_interface.bmp280_read_array = bmp280_read_array;
  handle.dependency_interface.bmp280_delay_function = delay_function;
  handle.dependency_interface.bmp280_power_function = power_function;
  
  bmp280_error_code_t error = bmp280_init(&handle, BMP280_I2C, BMP280_I2C_ADDRESS_1);

  if(error != BMP280_ERROR_OK)
  {
    Serial.println("FAIL");

    for(;;)
      ;
  }

  Serial.println("SUCCESS");
  
  error = bmp280_set_mode(&handle, BMP280_MODE_NORMAL);
  error = bmp280_set_temperature_oversampling(&handle, BMP280_OVERSAMPLING_4X);
  error = bmp280_set_pressure_oversampling(&handle, BMP280_OVERSAMPLING_16X);
  error = bmp280_set_standby_time(&handle, BMP280_T_STANDBY_250MS);
  error = bmp280_set_filter_coefficient(&handle, BMP280_FILTER_16X);
}

void loop() {
  bmp280_error_code_t error = bmp280_get_all(&handle, &sensorsData);

  if(error != BMP280_ERROR_OK)
  {
    Serial.println("FAIL");

    for(;;)
      ;
  }
  
  float altitudeHypsometric;

  bmp280_calculate_altitude_hypsometric(&handle, &altitudeHypsometric, sensorsData.pressure, sensorsData.temperature);
  
  Serial.print("Temperature = ");
  Serial.print(sensorsData.temperature, 2);
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(sensorsData.pressure);
  Serial.println(" Pa");

  Serial.print("Altitude (quick) = ");
  Serial.print(sensorsData.altitude);
  Serial.println(" m");

  Serial.print("Altitude (hypsometric) = ");
  Serial.print(altitudeHypsometric, 2);
  Serial.println(" m\n");

  delay(2000);
}
