# BMP280 Barometric Pressure and Temperature Sensor C Device Driver (Platform Independent)
* version 5.0
* Reza Ebrahimi

## Linux example

In order to compile:
```bash
gcc -I. -I./inc/ *.c ./src/*.c -o main.out -lm
```
In order to run, set an env variable to select the I2C device path. It would be '/dev/i2c-1' if not provided (RPi).
```bash
I2C_DEV_PATH=/dev/i2c-2 ./main.out
```

