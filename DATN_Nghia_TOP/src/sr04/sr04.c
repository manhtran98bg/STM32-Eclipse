/*
 * sr04.c
 *
 *  Created on: Jun 26, 2021
 *      Author: manht
 */
#include "sr04.h"
__IO uint16_t duration = 0;
uint8_t range =0 ;
__IO bool sr04_flag = 0;
void sr04_init(void){
	RCC_APB2PeriphClockCmd(sr04_gpio_clk, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitTypeDef gpio_struct;
	EXTI_InitTypeDef exti_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	gpio_struct.GPIO_Pin = sr04_echo;
	gpio_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(sr04_gpio, &gpio_struct);
	/* Connect EXTI5 Line to PA5 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);
	exti_init_struct.EXTI_Line = EXTI_Line5;
	exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_struct.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init_struct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init_struct);
	nvic_init_struct.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);

	gpio_struct.GPIO_Pin = sr04_trig;
	gpio_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(sr04_gpio, &gpio_struct);
	sr04_trig_reset;
}
static bool sr04_get_echo(){
	return GPIO_ReadInputDataBit(sr04_gpio, sr04_echo);
}
void start_tim4(void){
	TIM_SetCounter(TIM4, 0);
	TIM4->CR1 |=TIM_CR1_CEN;	//Start TIM4
}
void stop_tim4(void){
	TIM4->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));	//Stop TIM4
}
uint8_t sr04_get_distance(){
	// Trigger pulse 10us
	sr04_trig_set;
	dUS_tim4(10);
	sr04_trig_reset;
	while (!sr04_get_echo());
	start_tim4();
	delay_ms(100);
	return (uint8_t)(duration/58);
}
