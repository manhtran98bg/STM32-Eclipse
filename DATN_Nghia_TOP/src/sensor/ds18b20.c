/*
 * ds18b20.c
 *
 *  Created on: May 12, 2021
 *      Author: manht
 */
#include "ds18b20.h"
// ----------------------------------------------------------------------------
ds18b20_t ds18b20;
static void ds18_gpio_input()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = DS18B20_IN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}
// ----------------------------------------------------------------------------
static void ds18_gpio_output()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = DS18B20_IN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}
// ----------------------------------------------------------------------------
static void ds18_gpio_clk()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
}
// ----------------------------------------------------------------------------
static void ds18_set_gpio()
{
	GPIO_SetBits(GPIOA, DS18B20_IN);
}
// ----------------------------------------------------------------------------
static void ds18_reset_gpio()
{
	GPIO_ResetBits(GPIOA, DS18B20_IN);
}
// ----------------------------------------------------------------------------
static u8 ds18_read_input()
{
	return GPIO_ReadInputDataBit(GPIOA, DS18B20_IN);
}
// ----------------------------------------------------------------------------
uint8_t ds18_start(){
	uint8_t Response;
	ds18_gpio_clk();
	ds18_gpio_output();
	ds18_reset_gpio();
	dUS_tim4(480);
	ds18_gpio_input();
	dUS_tim4(80);
	if (!ds18_read_input())Response = 1;
	else Response = 0;
	dUS_tim4(400);
	return Response;
}
void ds18_write(uint8_t data)
{
	ds18_gpio_output();
	for (int i=0;i<8;i++)
	{
		if ((data&(1<<i))!=0){	//Write 1
			ds18_gpio_output();
			ds18_reset_gpio();
			dUS_tim4(6);
			ds18_set_gpio();
			dUS_tim4(64);
		}
		else  {
			ds18_gpio_output();
			ds18_reset_gpio();
			dUS_tim4(60);
			ds18_set_gpio();
			dUS_tim4(10);
		}
	}
}
uint8_t ds18_read()
{
	uint8_t value = 0;
	for (int i=0;i<8;i++)
	{
		ds18_gpio_output();
		ds18_reset_gpio();
		dUS_tim4(6);
		ds18_set_gpio();
		dUS_tim4(9);
		ds18_gpio_input();
		if (ds18_read_input()) {
			value|=(1<<i);
		}
		dUS_tim4(55);
	}
	return value;
}
/*	config resolution convert
 * 	9,10,11,12 bit
 */
void ds18_config(uint8_t resolution)
{
	uint8_t res;
	__IO uint8_t status;
	if (resolution==9) res = 0x1F;
	if (resolution==10) res = 0x3F;
	if (resolution==11) res = 0x5F;
	if (resolution==12) res = 0x7F;
	status = ds18_start();
	ds18_write(DS18B20_SKIP_ROM);
	ds18_write(DS18B20_WRITE_SCRATCH_PAD);
	ds18_write(0x00);
	ds18_write(0x00);
	ds18_write(res);
	ds18b20.res = resolution;
	ds18_read_id(&ds18b20);
}
void ds18_read_id(ds18b20_t *ds18b20)
{
	unsigned char  buf[8]={0};
	__IO long long id = 0;
	int i;
	ds18_start();
	ds18_write(DS18B20_READ_ROM);
	for(i=0;i<8;i++) buf[i]=ds18_read();
	ds18b20->unique_code.family_code = buf[0];
	for	(i=1;i<7;i++) id|=((long long)buf[i]<<((6-i)*8));
	ds18b20->unique_code.serial_number = id;
	ds18b20->unique_code.crc_code = buf[7];
}
void ds18_read_temp(ds18b20_t *ds18b20)
{
	uint16_t temp;
	uint8_t temp_byte1,temp_byte2;
	ds18_start();
	ds18_write(DS18B20_SKIP_ROM);
	ds18_write(DS18B20_CONVERT);
	delay_ms(100);
	ds18_start();
	ds18_write(DS18B20_SKIP_ROM);
	ds18_write(DS18B20_READ_SCRATCH_PAD);
	temp_byte1 = ds18_read();
	temp_byte2 = ds18_read();
	temp = (temp_byte2<<8)|temp_byte1;
	ds18b20->temp = (float)temp/16.0;
}
