#include "AHT10.h"

uint8_t AHT10_ADDRESS_0X38 =       0x38<<1;  //chip I2C address no.1 for AHT10/AHT15/AHT20, address pin connected to GND
uint8_t AHT10_ADDRESS_0X39 =        0x39;  //chip I2C address no.2 for AHT10 only, address pin connected to Vcc

uint8_t AHT10_INIT_CMD =           0xE1;  //initialization command for AHT10/AHT15
uint8_t AHT20_INIT_CMD =           0xBE;  //initialization command for AHT20
uint8_t AHT10_START_MEASURMENT_CMD=0xAC;  //start measurment command
uint8_t AHT10_NORMAL_CMD =         0xA8;  //normal cycle mode command, no info in datasheet!!!
uint8_t AHT10_SOFT_RESET_CMD =     0xBA;  //soft reset command

uint8_t AHT10_INIT_NORMAL_MODE =   0x00;  //enable normal mode
uint8_t AHT10_INIT_CYCLE_MODE =    0x20;  //enable cycle mode
uint8_t AHT10_INIT_CMD_MODE =      0x40;  //enable command mode
uint8_t AHT10_INIT_CAL_ENABLE =    0x08;  //load factory calibration coeff


uint8_t AHT10_DATA_MEASURMENT_CMD =0x33;  //no info in datasheet!!! my guess it is DAC resolution, saw someone send 0x00 instead
uint8_t AHT10_DATA_NOP =           0x00;  //no info in datasheet!!!

uint8_t _rawDataBuffer[6] = {0, 0, 0, 0, 0, 0};


/**************************************************************************/
/*
    begin()
    Initialize I2C & configure the sensor, call this function before
    doing anything else
*/
/**************************************************************************/

void AHT10_begin() 
{
  HAL_Delay(AHT10_POWER_ON_DELAY);    //wait for sensor to initialize 

  setNormalMode();                //one measurement+sleep mode

	enableFactoryCalCoeff(); //load factory calibration coeff
}


/**************************************************************************/
/*
    readRawData()
    Read raw measurment data from sensor over I2C
*/
/**************************************************************************/
void readRawData()
{
  /* send measurment command */
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_START_MEASURMENT_CMD, 1, 100);  //send measurment command
  HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_DATA_MEASURMENT_CMD, 1, 100);   //send measurment parameter
  HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_DATA_NOP, 1, 100);							//send measurment parameter

	HAL_Delay(AHT10_MEASURMENT_DELAY); //measurement delay

  /* read 6-bytes from sensor */
	
	HAL_I2C_Master_Receive(&hi2c1, AHT10_ADDRESS_0X38, _rawDataBuffer, 6, 100);
	
}


/**************************************************************************/
/*
    readTemperature()
    Read temperature, °C 
    NOTE:
    - temperature range      -40°C..+80°C
    - temperature resolution 0.01°C
    - temperature accuracy   ±0.3°C
*/
/**************************************************************************/
float readTemperature()
{
	readRawData();   //force to read data to _rawDataBuffer
	uint32_t temperature = ((uint32_t)(_rawDataBuffer[3] & 0x0F) << 16) | ((uint16_t)_rawDataBuffer[4] << 8) | _rawDataBuffer[5]; //20-bit raw temperature data

  return (float)temperature * 0.000191 - 50;
}


/**************************************************************************/
/*
    readHumidity()
    Read relative humidity, %
    NOTE:
    - prolonged exposure for 60 hours at humidity > 80% can lead to a
      temporary drift of the signal +3%. Sensor slowly returns to the
      calibrated state at normal operating conditions.
    - relative humidity range      0%..100%
    - relative humidity resolution 0.024%
    - relative humidity accuracy   ±2%
*/
/**************************************************************************/
float readHumidity()
{
	float humidity = 0;
	
	do{
		readRawData();   //force to read data to _rawDataBuffer

		uint32_t rawData = (((uint32_t)_rawDataBuffer[1] << 16) | ((uint16_t)_rawDataBuffer[2] << 8) | (_rawDataBuffer[3])) >> 4; //20-bit raw humidity data

		humidity = (float)rawData * 0.000095;

		if (humidity < 0) humidity = 0;
		if (humidity > 100) humidity = 100;
		
  }	
	while(humidity == 0);
	
	return humidity;
}


/**************************************************************************/
/*
    softReset()  
 
    Restart sensor, without power off
    NOTE:
    - takes ~20ms
    - all registers restores to default
*/
/**************************************************************************/
void softReset(void)
{
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_SOFT_RESET_CMD, 1, 100);

  HAL_Delay(AHT10_SOFT_RESET_DELAY);

  setNormalMode();                                   //reinitialize sensor registers after reset

  enableFactoryCalCoeff();                    //reinitialize sensor registers after reset
}


/**************************************************************************/
/*
    setNormalMode()  
 
    Set normal measurment mode
    NOTE:
    - one measurement & power down??? no info in datasheet!!!
*/
/**************************************************************************/
void setNormalMode()
{
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_NORMAL_CMD, 1, 100);
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_DATA_NOP, 1, 100);
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDRESS_0X38 , &AHT10_DATA_NOP, 1, 100);

  HAL_Delay(AHT10_CMD_DELAY);
}


/**************************************************************************/
/*
    setCycleMode()  
 
    Set cycle measurment mode
    NOTE:
    - continuous measurement
*/
/**************************************************************************/
/*bool setCycleMode(void)
{
  Wire.beginTransmission(_address);

  if   (_sensorName != AHT20_SENSOR) Wire.write(AHT10_INIT_CMD); //set command mode
  else                               Wire.write(AHT20_INIT_CMD); 
  Wire.write(AHT10_INIT_CYCLE_MODE | AHT10_INIT_CAL_ENABLE);     //0,[0,1],0,[1],0,0,0
  Wire.write(AHT10_DATA_NOP); 

  if (Wire.endTransmission(true) != 0) return false;             //safety check, make sure transmission complete
                                       return true;
}
*/

/**************************************************************************/
/*
    readStatusByte()
    Read status byte from sensor over I2C
*/
/**************************************************************************/
/*uint8_t readStatusByte()
{

  Wire.requestFrom(_address, 1, true);           //true - send stop after transmission & release I2C bus
  if (Wire.available() != 1) return AHT10_ERROR; //check rxBuffer & error handler, collision on I2C bus

  //read byte from "wire.h" rxBuffer
  return Wire.read();
}
*/



/**************************************************************************/
/*
    enableFactoryCalCoeff()
 
    Load factory calibration coefficients
*/
/**************************************************************************/
void enableFactoryCalCoeff()
{

	HAL_I2C_Master_Transmit(&hi2c1, AHT10_INIT_CMD, &AHT10_NORMAL_CMD, 1, 100);//set command mode
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_INIT_CAL_ENABLE, &AHT10_NORMAL_CMD, 1, 100);//0,0,0,0,[1],0,0,0  
	HAL_I2C_Master_Transmit(&hi2c1, AHT10_DATA_NOP, &AHT10_NORMAL_CMD, 1, 100);	

  HAL_Delay(AHT10_CMD_DELAY);
}


/**************************************************************************/
/*
    getBusyBit()
    Read busy bit from status byte
    NOTE:
    - 0, sensor idle & sleeping
    - 1, sensor busy & in measurement state
*/
/**************************************************************************/
/*uint8_t getBusyBit(bool readI2C)
{
  if (readI2C == AHT10_FORCE_READ_DATA) _rawDataBuffer[0] = readStatusByte(); //force to read status byte

  if (_rawDataBuffer[0] != AHT10_ERROR) return bitRead(_rawDataBuffer[0], 7); //get 7-th bit
                                        return AHT10_ERROR;
}*/
