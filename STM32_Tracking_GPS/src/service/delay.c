/*
 * delay.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "delay.h"
extern volatile uint32_t msTicks;
void delay_ms(uint32_t ms)
{
	msTicks=ms;
	while(msTicks);
}

