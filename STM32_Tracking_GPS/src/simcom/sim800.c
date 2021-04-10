/*
 * sim800.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "sim800.h"
#include "../service/delay.h"
void sim_gpio_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_Init_Structure;
	GPIO_Init_Structure.GPIO_Pin = GSM_POWERKEY;
	GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init_Structure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_Init_Structure);

	GPIO_Init_Structure.GPIO_Pin = GSM_STATUS;
	GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_Init_Structure);
}
static void toggle_powerkey()
{
	GPIO_ResetBits(GPIOC, GSM_POWERKEY);
	delay_ms(1500);
	GPIO_SetBits(GPIOC, GSM_POWERKEY);
	delay_ms(500);
}
uint8_t sim_power_status()
{
	if (GPIO_ReadInputDataBit(GPIOC, GSM_STATUS))return 1;
	else return 0;
}
void sim_power_on()
{
	if (!sim_power_status()) {
		toggle_powerkey();
		while(!sim_power_status());
#if	_DEBUG
		trace_puts("SIM ON");
#endif
	}
	else{
#if _DEBUG
		trace_puts("SIM ALREADY ON");
#endif
	}
}

void sim_power_off()
{
	if (sim_power_status()) {
		toggle_powerkey();
		while(sim_power_status());
#if	_DEBUG
		trace_puts("SIM OFF");
#endif
	}
	else{
#if _DEBUG
		trace_puts("SIM ALREADY OFF");
#endif
	}
}

