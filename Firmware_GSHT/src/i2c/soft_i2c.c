/*
 * soft_i2c.c
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */
#include "soft_i2c.h"
void soft_i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(SI2C_GPIO_RCC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = SI2C_GPIO_SDA|SI2C_GPIO_SCL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SI2C_GPIO, &GPIO_InitStruct);
	SDA_1;
	SCL_1;
}
void soft_i2c_start(void)
{

	SCL_1;
	dUS_tim4(3);
	SDA_1;
	dUS_tim4(3);
	SDA_0;
	dUS_tim4(3);
	SCL_0;
	dUS_tim4(3);
}
void soft_i2c_stop(void)
{

	SDA_0;
	dUS_tim4(3);
	SCL_1;
	dUS_tim4(3);
	SDA_1;
	dUS_tim4(3);
}
uint8_t soft_i2c_write(uint8_t u8Data)
{
	uint8_t i;
	uint8_t u8Ret;

	for (i = 0; i < 8; ++i) {
		if (u8Data & 0x80) {
			SDA_1;
		} else {
			SDA_0;
		}
		dUS_tim4(3);
		SCL_1;
		dUS_tim4(5);
		SCL_0;
		dUS_tim4(2);
		u8Data <<= 1;
	}

	SDA_1;
	dUS_tim4(3);
	SCL_1;
	dUS_tim4(3);
	if (SDA_VAL) {
		u8Ret = 0;
	} else {
		u8Ret = 1;
	}
	dUS_tim4(2);
	SCL_0;
	dUS_tim4(5);
	return u8Ret;
}
uint8_t soft_i2c_read(uint8_t u8Ack)
{
	uint8_t i;
	uint8_t u8Ret=0;

	SDA_1;
	dUS_tim4(3);

	for (i = 0; i < 8; ++i) {
		u8Ret <<= 1;
		SCL_1;
		dUS_tim4(3);
		if (SDA_VAL) {
			u8Ret |= 0x01;
		}
		dUS_tim4(2);
		SCL_0;
		dUS_tim4(5);
	}

	if (u8Ack) {
		SDA_0;
	} else {
		SDA_1;
	}
	dUS_tim4(3);

	SCL_1;
	dUS_tim4(5);
	SCL_0;
	dUS_tim4(5);

	return u8Ret;
}
