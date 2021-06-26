/*
 * board.c
 *
 *  Created on: May 11, 2021
 *      Author: manht
 */
#include "board.h"
#define _USE_HSI	0
void rcc_config(){
#if _USE_HSI
	RCC_DeInit();
	RCC_HSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY)==RESET);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
	while(RCC_GetSYSCLKSource()!=0x00);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div1);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
#else
	RCC_DeInit();
	RCC_HSICmd(DISABLE);
	RCC_HSEConfig(RCC_HSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY)==RESET);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_PLLCmd(DISABLE);
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource()!=0x08);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
#endif
}
void user_led_config(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef gpio_struct;
	gpio_struct.GPIO_Pin = USER_LED_PIN;
	gpio_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_struct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpio_struct);
}
void user_led_toggle(){
	GPIOB->ODR ^= USER_LED_PIN;
}

/*	TIMER 4 Config 1uS, SysClock = 8Mhz
 * 	Prescaler = 71
 */
void tim4_init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 71;
	TIM_TimeBaseInitStruct.TIM_Period = 0xffff-1;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
	TIM_Cmd(TIM4, ENABLE);
}

