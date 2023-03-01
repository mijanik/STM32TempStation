/*
   This is an Arduino library for Aosong ASAIR AHT10, AHT15 Digital Humidity & Temperature Sensor
   written by : enjoyneering79
   sourse code: https://github.com/enjoyneering/
   This chip uses I2C bus to communicate, specials pins are required to interface
   Board:                                    SDA                    SCL                    Level
   ESP32.................................... GPIO21/D21             GPIO22/D22             3.3v
   Frameworks & Libraries:
   ESP32   Core          - https://github.com/espressif/arduino-esp32
   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#ifndef AHT10_h
#define AHT10_h

#include "stm32l4xx_hal.h"
#include "stdio.h"
#include "stdbool.h"


#define AHT10_MEASURMENT_DELAY     80    //at least 75 milliseconds
#define AHT10_POWER_ON_DELAY       40    //at least 20..40 milliseconds
#define AHT10_CMD_DELAY            350   //at least 300 milliseconds, no info in datasheet!!!
#define AHT10_SOFT_RESET_DELAY     20    //less than 20 milliseconds

extern I2C_HandleTypeDef hi2c1;


void AHT10_begin();
void setNormalMode(); 
void enableFactoryCalCoeff();
void readRawData();
float readTemperature();
float readHumidity();
void softReset();
#endif
