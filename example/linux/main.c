#include <stdio.h>
#include "bmp280.h"
#include "interface.h"

#define CHECK_AND_RETURN_ERROR(error)               \
    do                                              \
    {                                               \
        if (error != BMP280_ERROR_OK)               \
        {                                           \
            printf("ERROR CODE: %d\n", (int)error); \
            return (int)error;                      \
        }                                           \
    } while (0)

bmp280_handle_t BMP280;
bmp280_sensors_data_t sensorsData;

int main(int arg, char *argv[])
{
    BMP280.dependency_interface.bmp280_interface_init = bmp280_i2c_init;
    BMP280.dependency_interface.bmp280_interface_deinit = bmp280_i2c_deinit;
    BMP280.dependency_interface.bmp280_write_array = bmp280_write_array;
    BMP280.dependency_interface.bmp280_read_array = bmp280_read_array;
    BMP280.dependency_interface.bmp280_delay_function = delay_function;
    BMP280.dependency_interface.bmp280_power_function = power_function;

    bmp280_error_code_t error = bmp280_init(&BMP280, BMP280_I2C, BMP280_I2C_ADDRESS_1);
    CHECK_AND_RETURN_ERROR(error);

    error = bmp280_set_mode(&BMP280, BMP280_MODE_NORMAL);
    CHECK_AND_RETURN_ERROR(error);
    error = bmp280_set_temperature_oversampling(&BMP280, BMP280_OVERSAMPLING_4X);
    CHECK_AND_RETURN_ERROR(error);
    error = bmp280_set_pressure_oversampling(&BMP280, BMP280_OVERSAMPLING_16X);
    CHECK_AND_RETURN_ERROR(error);
    error = bmp280_set_standby_time(&BMP280, BMP280_T_STANDBY_250MS);
    CHECK_AND_RETURN_ERROR(error);
    error = bmp280_set_filter_coefficient(&BMP280, BMP280_FILTER_16X);
    CHECK_AND_RETURN_ERROR(error);

    for (;;)
    {
        error = bmp280_get_all(&BMP280, &sensorsData);
        CHECK_AND_RETURN_ERROR(error);
        error = bmp280_calculate_altitude_quick(&BMP280, &sensorsData.altitude, sensorsData.pressure);
        CHECK_AND_RETURN_ERROR(error);

        printf("TEMP: %.2f °C, PRESSURE: %d Pa, ALTITUDE: %.1f m\n", sensorsData.temperature, sensorsData.pressure, sensorsData.altitude);

        sleep(1);
    }

    return 0;
}
