/*
 * board.c
 *
 *  Created on: May 11, 2021
 *      Author: manht
 */
#include <main.h>
//board_t board;
void clk_init()
{
#ifdef _STM32F103RCT6_
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
#else
	RCC_DeInit();
	RCC_HSICmd(DISABLE);
	RCC_HSEConfig(RCC_HSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY)==RESET);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_PLLCmd(DISABLE);
	RCC_PREDIV1Config(RCC_PREDIV1_Source_HSE, RCC_PREDIV1_Div1);
	RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);
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
void tim5_init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 7199;
	TIM_TimeBaseInitStruct.TIM_Period = 40000;
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStruct);
	TIM_ClearFlag(TIM5, TIM_FLAG_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	NVIC_EnableIRQ(TIM5_IRQn);
	TIM_Cmd(TIM5, ENABLE);
}
/*	TIMER 4 Config 1uS, SysClock = 72Mhz
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
}
void user_led_init()
{
	GPIO_InitTypeDef GPIO_init_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_init_struct.GPIO_Pin = USER_LED;
	GPIO_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_init_struct);
}
void user_led_toggle()
{
	GPIOA->ODR ^=(uint32_t)(USER_LED);
}
void btn_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef gpio_init_struct;
	EXTI_InitTypeDef exti_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	gpio_init_struct.GPIO_Pin = USER_BTN1|USER_BTN2|USER_BTN3;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &gpio_init_struct);
	/* Connect EXTI12 Line to PB12 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	/* Connect EXTI13 Line to PB13 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	/* Connect EXTI14 Line to PB14 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);

	/* Configure EXTI line */
	exti_init_struct.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line0;
	exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_struct.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init_struct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init_struct);
	nvic_init_struct.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);
}
