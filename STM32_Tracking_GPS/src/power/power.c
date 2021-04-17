/*
 * board.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "power.h"
static void power_gpio_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_InitTypeDef GPIO_init_struct;
	GPIO_init_struct.GPIO_Pin = CTRL_PWR_GSM;
	GPIO_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_init_struct);
	GPIO_init_struct.GPIO_Pin = CTRL_PWR_GPS;
	GPIO_Init(GPIOA, &GPIO_init_struct);
	GPIO_SetBits(GPIOA, CTRL_PWR_GPS);
}
void power_reset_sim()
{
	power_gpio_init();
//	GPIO_SetBits(GPIOB, CTRL_PWR_GSM);
//	delay_ms(100);
//	GPIO_ResetBits(GPIOB, CTRL_PWR_GSM);
}
void power_off_sim()
{
	GPIO_SetBits(GPIOB, CTRL_PWR_GSM);
}
void power_on_sim()
{
	GPIO_ResetBits(GPIOB, CTRL_PWR_GSM);
}
void power_off_gps()
{
	GPIO_SetBits(GPIOA, CTRL_PWR_GPS);
}
void power_on_gps()
{
	GPIO_ResetBits(GPIOA, CTRL_PWR_GPS);
}

