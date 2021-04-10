/*
 * dht11.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "dht11.h"
static void dht11_gpio_input()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = DHT11_IN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}
static void dht11_gpio_output()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = DHT11_IN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}
static void dht11_gpio_clk()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}
static void dht11_set_gpio()
{
	GPIO_SetBits(GPIOC, DHT11_IN);
}
static void dht11_reset_gpio()
{
	GPIO_ResetBits(GPIOC, DHT11_IN);
}
static u8 dht11_read_input()
{
	return GPIO_ReadInputDataBit(GPIOC, DHT11_IN);
}
static void dht11_init()
{
	dht11_gpio_clk();
	dht11_gpio_output();	//Set pin as OUTPUT;
	dht11_reset_gpio();		//Pull pin LOW;
	delay_ms(18);
	dht11_set_gpio();
	dUS_tim4(20);
	dht11_gpio_input();		//set pin as input;
}
static int8_t dht11_check_response()
{
	int8_t res = 0;
	dUS_tim4(40);
	if (!dht11_read_input())
	{
		dUS_tim4(80);
		if (dht11_read_input()) res = 1;
		else res = -1;
	}
	while(dht11_read_input());
	return res;
}
static uint8_t dht11_read_d8()
{
	uint8_t i=0,j;
	for(j=0;j<8;j++)
	{
		while (!dht11_read_input()); // wait for the pin to go high
		dUS_tim4(40);
		if(!dht11_read_input())
		{
			i &= ~(1<<(7-j));   		// write 0
		}
		else i|= (1<<(7-j));  		// if the pin is high, write 1
		while (dht11_read_input()); // wait for the pin to go low
	}
	return i;
}
u8 dht11_read_data(dht11_data *dat)
{
	dht11_init();
	if (dht11_check_response())
	{
		dat->RH_Byte1 = dht11_read_d8();
		dat->RH_Byte2 = dht11_read_d8();
		dat->Temp_Byte1 = dht11_read_d8();
		dat->Temp_Byte2 = dht11_read_d8();
		dat->Check_sum = dht11_read_d8();
		if (dat->Check_sum == dat->RH_Byte1+dat->RH_Byte2+dat->Temp_Byte1+dat->Temp_Byte2) return 1;
		else return 0;
	}
	return 0;
}


