
#include "ds18b20.h"
extern TM_OneWire_t OneWire1;
uint8_t TM_DS18B20_Start(TM_OneWire_t* OneWire, uint8_t *ROM) {
	/* Check if device is DS18B20 */
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Start temperature conversion */
	TM_OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);
	return 1;
}

void TM_DS18B20_StartAll(TM_OneWire_t* OneWire) {
	/* Reset pulse */
	TM_OneWire_Reset(OneWire);
	/* Skip rom */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_SKIPROM);
	/* Start conversion on all connected devices */
	TM_OneWire_WriteByte(OneWire, DS18B20_CMD_CONVERTTEMP);
}

uint8_t TM_DS18B20_Read(TM_OneWire_t* OneWire, uint8_t *ROM, float *destination) {
	uint16_t temperature;
	uint8_t resolution;
	int8_t digit, minus = 0;
	float decimal;
	uint8_t i = 0;
	uint8_t data[9];
	uint8_t crc;
	
	/* Check if device is DS18B20 */
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	
	/* Check if line is released, if it is, then conversion is complete */
	if (!TM_OneWire_ReadBit(OneWire)) {
		/* Conversion is not finished yet */
		return 0; 
	}

	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Get data */
	for (i = 0; i < 9; i++) {
		/* Read byte by byte */
		data[i] = TM_OneWire_ReadByte(OneWire);
	}
	
	/* Calculate CRC */
	crc = TM_OneWire_CRC8(data, 8);
	
	/* Check if CRC is ok */
	if (crc != data[8]) {
		/* CRC invalid */
		return 0;
	}
	
	/* First two bytes of scratchpad are temperature values */
	temperature = data[0] | (data[1] << 8);

	/* Reset line */
	TM_OneWire_Reset(OneWire);
	
	/* Check if temperature is negative */
	if (temperature & 0x8000) {
		/* Two's complement, temperature is negative */
		temperature = ~temperature + 1;
		minus = 1;
	}

	
	/* Get sensor resolution */
	resolution = ((data[4] & 0x60) >> 5) + 9;

	
	/* Store temperature integer digits and decimal digits */
	digit = temperature >> 4;
	digit |= ((temperature >> 8) & 0x7) << 4;
	
	/* Store decimal digits */
	switch (resolution) {
		case 9: {
			decimal = (temperature >> 3) & 0x01;
			decimal *= (float)DS18B20_DECIMAL_STEPS_9BIT;
		} break;
		case 10: {
			decimal = (temperature >> 2) & 0x03;
			decimal *= (float)DS18B20_DECIMAL_STEPS_10BIT;
		} break;
		case 11: {
			decimal = (temperature >> 1) & 0x07;
			decimal *= (float)DS18B20_DECIMAL_STEPS_11BIT;
		} break;
		case 12: {
			decimal = temperature & 0x0F;
			decimal *= (float)DS18B20_DECIMAL_STEPS_12BIT;
		} break;
		default: {
			decimal = 0xFF;
			digit = 0;
		}
	}
	
	/* Check for negative part */
	decimal = digit + decimal;
	if (minus) {
		decimal = 0 - decimal;
	}
	
	/* Set to pointer */
	*destination = decimal;
	
	/* Return 1, temperature valid */
	return 1;
}

uint8_t TM_DS18B20_GetResolution(TM_OneWire_t* OneWire, uint8_t *ROM) {
	uint8_t conf;
	
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 4 bytes */
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	
	/* 5th byte of scratchpad is configuration register */
	conf = TM_OneWire_ReadByte(OneWire);
	
	/* Return 9 - 12 value according to number of bits */
	return ((conf & 0x60) >> 5) + 9;
}

uint8_t TM_DS18B20_SetResolution(TM_OneWire_t* OneWire, uint8_t *ROM, TM_DS18B20_Resolution_t resolution) {
	uint8_t th, tl, conf;
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	
	th = TM_OneWire_ReadByte(OneWire);
	tl = TM_OneWire_ReadByte(OneWire);
	conf = TM_OneWire_ReadByte(OneWire);
	
	if (resolution == TM_DS18B20_Resolution_9bits) {
		conf &= ~(1 << DS18B20_RESOLUTION_R1);
		conf &= ~(1 << DS18B20_RESOLUTION_R0);
	} else if (resolution == TM_DS18B20_Resolution_10bits) {
		conf &= ~(1 << DS18B20_RESOLUTION_R1);
		conf |= 1 << DS18B20_RESOLUTION_R0;
	} else if (resolution == TM_DS18B20_Resolution_11bits) {
		conf |= 1 << DS18B20_RESOLUTION_R1;
		conf &= ~(1 << DS18B20_RESOLUTION_R0);
	} else if (resolution == TM_DS18B20_Resolution_12bits) {
		conf |= 1 << DS18B20_RESOLUTION_R1;
		conf |= 1 << DS18B20_RESOLUTION_R0;
	}
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	TM_OneWire_WriteByte(OneWire, th);
	TM_OneWire_WriteByte(OneWire, tl);
	TM_OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t TM_DS18B20_Is(uint8_t *ROM) {
	/* Checks if first byte is equal to DS18B20's family code */
	if (*ROM == DS18B20_FAMILY_CODE) {
		return 1;
	}
	return 0;
}

uint8_t TM_DS18B20_SetAlarmLowTemperature(TM_OneWire_t* OneWire, uint8_t *ROM, int8_t temp) {
	uint8_t tl, th, conf;
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	if (temp > 125) {
		temp = 125;
	} 
	if (temp < -55) {
		temp = -55;
	}
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	
	th = TM_OneWire_ReadByte(OneWire);
	tl = TM_OneWire_ReadByte(OneWire);
	conf = TM_OneWire_ReadByte(OneWire);
	
	tl = (uint8_t)temp; 

	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	TM_OneWire_WriteByte(OneWire, th);
	TM_OneWire_WriteByte(OneWire, tl);
	TM_OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t TM_DS18B20_SetAlarmHighTemperature(TM_OneWire_t* OneWire, uint8_t *ROM, int8_t temp) {
	uint8_t tl, th, conf;
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	if (temp > 125) {
		temp = 125;
	} 
	if (temp < -55) {
		temp = -55;
	}
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	
	th = TM_OneWire_ReadByte(OneWire);
	tl = TM_OneWire_ReadByte(OneWire);
	conf = TM_OneWire_ReadByte(OneWire);
	
	th = (uint8_t)temp; 

	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	TM_OneWire_WriteByte(OneWire, th);
	TM_OneWire_WriteByte(OneWire, tl);
	TM_OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t TM_DS18B20_DisableAlarmTemperature(TM_OneWire_t* OneWire, uint8_t *ROM) {
	uint8_t tl, th, conf;
	if (!TM_DS18B20_Is(ROM)) {
		return 0;
	}
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Read scratchpad command by onewire protocol */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_RSCRATCHPAD);
	
	/* Ignore first 2 bytes */
	TM_OneWire_ReadByte(OneWire);
	TM_OneWire_ReadByte(OneWire);
	
	th = TM_OneWire_ReadByte(OneWire);
	tl = TM_OneWire_ReadByte(OneWire);
	conf = TM_OneWire_ReadByte(OneWire);
	
	th = 125;
	tl = (uint8_t)-55;

	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_WSCRATCHPAD);
	
	/* Write bytes */
	TM_OneWire_WriteByte(OneWire, th);
	TM_OneWire_WriteByte(OneWire, tl);
	TM_OneWire_WriteByte(OneWire, conf);
	
	/* Reset line */
	TM_OneWire_Reset(OneWire);
	/* Select ROM number */
	TM_OneWire_SelectWithPointer(OneWire, ROM);
	/* Copy scratchpad to EEPROM of DS18B20 */
	TM_OneWire_WriteByte(OneWire, ONEWIRE_CMD_CPYSCRATCHPAD);
	
	return 1;
}

uint8_t TM_DS18B20_AlarmSearch(TM_OneWire_t* OneWire) {
	/* Start alarm search */
	return TM_OneWire_Search(OneWire, DS18B20_CMD_ALARMSEARCH);
}

uint8_t TM_DS18B20_AllDone(TM_OneWire_t* OneWire) {
	/* If read bit is low, then device is not finished yet with calculation temperature */
	return TM_OneWire_ReadBit(OneWire);
}
uint8_t ds18b20_init(TM_OneWire_t* OneWireStruct, ds18b20_t *ds18b20)
{
    char buf[40];
    long long id = 0;
    uint8_t devices, i, j, count;
    uint8_t ds18b20_id[EXPECTING_SENSORS][8];
    TM_OneWire_Init(OneWireStruct, DS18B20_GPIO, DS18B20_GPIO_PIN);
    count = 0;
    devices = TM_OneWire_First(OneWireStruct);
    while (devices) {
    	/* Increase counter */
    	count++;
    	/* Get full ROM value, 8 bytes, give location of first byte where to save */
    	TM_OneWire_GetFullROM(OneWireStruct, ds18b20_id[count - 1]);
    	/* Get next device */
    	devices = TM_OneWire_Next(OneWireStruct);
    }
    OneWireStruct->Devices = count;
    /* If any devices on 1wire */
    if (count > 0) {
        sprintf(buf, "Sensor found on 1-wire: %d sensor\n", count);
#if _DEBUG
        trace_write((char*)"log:", strlen("log:"));
        trace_write((char*)buf, strlen(buf));
#endif
#if _USE_DEBUG_UART
        debug_send_string(buf);
#endif
        for (j = 0; j < count; j++){
        	id = 0;
        	for (i = 0; i<8;i++) ds18b20[j].rom_number[i] = ds18b20_id[j][i];
        	/* Parse 64bit rom code for each device */
        	ds18b20[j].unique_code.family_code = ds18b20_id[j][0];
        	for (i = 1; i < 7; i++) id|=((long long)ds18b20_id[j][i]<<((6-i)*8));
        	ds18b20[j].unique_code.serial_number = id;
        	ds18b20[j].unique_code.crc_code = ds18b20_id[j][7];
        }

    } else {
#if _DEBUG
        trace_write((char*)"log:", strlen("log:"));
    	trace_puts("No sensor on OneWire.");
#endif
#if _USE_DEBUG_UART
    	debug_send_string("No sensor on OneWire.\n");
#endif
    }
    /* Go through all connected devices and set resolution to 9bits */
    for (i = 0; i < count; i++) {
        /* Set resolution to 9bits */
        TM_DS18B20_SetResolution(OneWireStruct, ds18b20_id[i], TM_DS18B20_Resolution_9bits);
        ds18b20[i].resolution = TM_DS18B20_Resolution_9bits;
    }
    return count;

}
uint8_t ds18b20_read_temp(TM_OneWire_t* OneWireStruct, ds18b20_t *ds18b20)
{
	int i;
	uint8_t cnt=0;
	if (OneWireStruct->Devices == 0) return 0;
	/* Start temperature conversion on all devices on one bus */
	TM_DS18B20_StartAll(OneWireStruct);
	/* Wait until all are done on one onewire port */
	while (!TM_DS18B20_AllDone(&OneWire1));
	/* Read temperature from each device separatelly */
	for (i = 0; i < OneWireStruct->Devices; i++) {
		/* Read temperature from ROM address and store it to temps variable */
		if (TM_DS18B20_Read(OneWireStruct,ds18b20[i].rom_number,&ds18b20[i].temp)) {
			cnt++;
		} else {
			/* Reading error */
		}
	}
	if (cnt == OneWireStruct->Devices) return 1;
	else return 0;
}

