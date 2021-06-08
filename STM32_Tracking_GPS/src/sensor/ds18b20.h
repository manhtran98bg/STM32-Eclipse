/*
 * ds18b20.h
 *
 *  Created on: May 12, 2021
 *      Author: manht
 */

#ifndef DS18B20_H
#define DS18B20_H
#include "onewire.h"
#include "../src/usart/usart.h"
/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE				0x28
#define DS18B20_CMD_ALARMSEARCH			0xEC

/* How many sensor on bus */
#define EXPECTING_SENSORS    2
/* DS18B20 read temperature command */
#define DS18B20_CMD_CONVERTTEMP			0x44 	/* Convert temperature */

#define DS18B20_DECIMAL_STEPS_12BIT		0.0625
#define DS18B20_DECIMAL_STEPS_11BIT		0.125
#define DS18B20_DECIMAL_STEPS_10BIT		0.25
#define DS18B20_DECIMAL_STEPS_9BIT		0.5

/* Bits locations for resolution */
#define DS18B20_RESOLUTION_R1			6
#define DS18B20_RESOLUTION_R0			5

/* CRC enabled */
#ifdef DS18B20_USE_CRC	
#define DS18B20_DATA_LEN				9
#else
#define DS18B20_DATA_LEN				2
#endif

/**
 * @}
 */

/**
 * @defgroup TM_DS18B20_Typedefs
 * @brief    Library Typedefs
 * @{
 */

/**
 * @brief  DS18B0 Resolutions available
 */
typedef enum {
	TM_DS18B20_Resolution_9bits = 9,   /*!< DS18B20 9 bits resolution */
	TM_DS18B20_Resolution_10bits = 10, /*!< DS18B20 10 bits resolution */
	TM_DS18B20_Resolution_11bits = 11, /*!< DS18B20 11 bits resolution */
	TM_DS18B20_Resolution_12bits = 12  /*!< DS18B20 12 bits resolution */
} TM_DS18B20_Resolution_t;

typedef struct {
	uint8_t	int_part;
	uint8_t dec_part;
}temp_t;
typedef struct {
	uint8_t family_code;
	long long serial_number;
	uint8_t	crc_code;
}rom_code_t;
typedef struct {
	float	temp;
	uint8_t rom_number[8];
	rom_code_t unique_code;
	TM_DS18B20_Resolution_t resolution;
}ds18b20_t;

/**
 * @}
 */

/**
 * @defgroup TM_DS18B20_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Starts temperature conversion for specific DS18B20 on specific onewire channel
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval 1 if device is DS18B20 or 0 if not
 */
uint8_t TM_DS18B20_Start(TM_OneWire_t *OneWireStruct, uint8_t* ROM);

/**
 * @brief  Starts temperature conversion for all DS18B20 devices on specific onewire channel
 * @note   This mode will skip ROM addressing
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @retval None
 */
void TM_DS18B20_StartAll(TM_OneWire_t* OneWireStruct);

/**
 * @brief  Reads temperature from DS18B20
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  *destination: Pointer to float variable to store temperature
 * @retval Temperature status:
 *            - 0: Device is not DS18B20 or conversion is not done yet or CRC failed
 *            - > 0: Temperature is read OK
 */
uint8_t TM_DS18B20_Read(TM_OneWire_t* OneWireStruct, uint8_t* ROM, float* destination);

/**
 * @brief  Gets resolution for temperature conversion from DS18B20 device
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval Resolution:
 *            - 0: Device is not DS18B20
 *            - 9 - 12: Resolution of DS18B20
 */
uint8_t TM_DS18B20_GetResolution(TM_OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Sets resolution for specific DS18B20 device
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  resolution: Resolution for DS18B20 device. This parameter can be a value of @ref TM_DS18B20_Resolution_t enumeration.
 * @retval Success status:
 *            - 0: Device is not DS18B20
 *            - > 0: Resolution set OK
 */
uint8_t TM_DS18B20_SetResolution(TM_OneWire_t* OneWireStruct, uint8_t* ROM, TM_DS18B20_Resolution_t resolution);

/**
 * @brief  Checks if device with specific ROM number is DS18B20
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval Device status
 *            - 0: Device is not DS18B20
 *            - > 0: Device is DS18B20
 */
uint8_t TM_DS18B20_Is(uint8_t* ROM);

/**
 * @brief  Sets high alarm temperature to specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  temp: integer value for temperature between -55 to 125 degrees
 * @retval Success status:
 *            - 0: Device is not DS18B20
 *            - > 0: High alarm set OK
 */
uint8_t TM_DS18B20_SetAlarmHighTemperature(TM_OneWire_t *OneWireStruct, uint8_t* ROM, int8_t temp);

/**
 * @brief  Sets low alarm temperature to specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @param  temp: integer value for temperature between -55 to 125 degrees
 * @retval Success status:
 *            - 0: Device is not DS18B20
 *            - > 0: Low alarm set OK
 */
uint8_t TM_DS18B20_SetAlarmLowTemperature(TM_OneWire_t* OneWireStruct, uint8_t* ROM, int8_t temp);

/**
 * @brief  Disables alarm temperature for specific DS18B20 sensor
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @param  *ROM: Pointer to first byte of ROM address for desired DS12B80 device.
 *         Entire ROM address is 8-bytes long
 * @retval Success status:
 *            - 0: Device is not DS18B20
 *            - > 0: Alarm disabled OK
 */
uint8_t TM_DS18B20_DisableAlarmTemperature(TM_OneWire_t* OneWireStruct, uint8_t* ROM);

/**
 * @brief  Searches for devices with alarm flag set
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @retval Alarm search status
 *            - 0: No device found with alarm flag set
 *            - > 0: Device is found with alarm flag
 * @note   To get all devices on one onewire channel with alarm flag set, you can do this:
@verbatim
while (TM_DS18B20_AlarmSearch(&OneWireStruct)) {
	//Read device ID here 
	//Print to user device by device
}
@endverbatim 
 * @retval 1 if any device has flag, otherwise 0
 */
uint8_t TM_DS18B20_AlarmSearch(TM_OneWire_t* OneWireStruct);

/**
 * @brief  Checks if all DS18B20 sensors are done with temperature conversion
 * @param  *OneWireStruct: Pointer to @ref TM_OneWire_t working structure (OneWire channel)
 * @retval Conversion status
 *            - 0: Not all devices are done
 *            - > 0: All devices are done with conversion
 */
uint8_t TM_DS18B20_AllDone(TM_OneWire_t* OneWireStruct);

uint8_t ds18b20_init(TM_OneWire_t* OneWireStruct, ds18b20_t *ds18b20);
uint8_t ds18b20_read_temp(TM_OneWire_t* OneWireStruct, ds18b20_t *ds18b20);
extern ds18b20_t ds18b20[];
/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */
 
#endif

