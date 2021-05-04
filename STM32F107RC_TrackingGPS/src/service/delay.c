/*
 * delay.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "delay.h"
extern volatile uint32_t msTicks;
extern uint32_t uwTick;
void delay_ms(uint32_t ms)
{
	msTicks=ms;
	while(msTicks);
}
void dUS_tim4(uint16_t uS)
{
	TIM_SetCounter(TIM4, 0);
	TIM4->CR1 |=TIM_CR1_CEN;	//Start TIM4
	while (TIM_GetCounter(TIM4)<uS);
	TIM4->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));	//Stop TIM4
}
uint32_t millis(void){
	return uwTick;
}
