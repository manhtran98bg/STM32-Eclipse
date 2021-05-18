/*
 * ds18b20.h
 *
 *  Created on: May 12, 2021
 *      Author: manht
 */

#ifndef SENSOR_DS18B20_H_
#define SENSOR_DS18B20_H_

#include "main.h"
#include "../service/delay.h"
#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_CONVERT	0x44
#define DS18B20_READ_SCRATCH_PAD	0xBE
#define DS18B20_WRITE_SCRATCH_PAD	0x4E
#define DS18B20_READ_ROM	0x33

#define RES_9BIT	9
#define RES_10BIT	10
#define RES_11BIT	11
#define RES_12BIT	12

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
	rom_code_t unique_code;
	uint8_t res;
}ds18b20_t;
extern ds18b20_t ds18b20;
uint8_t ds18_start();
void ds18_write(uint8_t data);
uint8_t ds18_read();
void ds18_read_temp(ds18b20_t *ds18b20);
void ds18_config(uint8_t resolution);
void ds18_read_id(ds18b20_t *ds18b20);
#endif /* SENSOR_DS18B20_H_ */
