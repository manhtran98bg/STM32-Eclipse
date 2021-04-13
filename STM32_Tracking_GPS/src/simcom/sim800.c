/*
 * sim800.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "sim800.h"
#include "../service/delay.h"
#include "../usart/usart.h"
extern __IO uint8_t RxBuffer1[];

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
static uint8_t sim_power_status()
{
	if (GPIO_ReadInputDataBit(GPIOC, GSM_STATUS))return 1;
	else return 0;
}
/*	Power On SIM800C
 */
uint8_t sim_power_on()
{
	u8 time_out=0;
	if (!sim_power_status()) {
		toggle_powerkey();
#if	_DEBUG
		trace_puts("Waiting SIM Power ON.");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((uint8_t*)"Waiting SIM Power ON.\n");
#endif
		while((time_out++<10)) {
			if(sim_power_status()) break;
#if	_DEBUG
		trace_puts(".");
#endif
			delay_ms(1000);
		}
		if (time_out>=10)
		{
#if	_DEBUG
			trace_puts("SIM can't turn on. Check SIM Power!");
#endif
#if _DEBUG_SIM_UART5
			UART5_Send_String((uint8_t*)"SIM can't turn on. Check SIM Power!\n");
#endif
			return 0;
		}
		else
		{
#if	_DEBUG
			trace_puts("SIM POWER ON");
#endif
#if _DEBUG_SIM_UART5
			UART5_Send_String((uint8_t*)"SIM POWER ON\n");
#endif
			return 1;
		}
	}
	else{
#if _DEBUG
		trace_puts("SIM ALREADY ON");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((uint8_t*)"SIM ALREADY POWER ON\n");
#endif
		return 1;
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
static void sim_send_cmd(uint8_t* cmd, uint16_t ms)
{
	USART_clear_buf(1);
	USART1_Send_String(cmd);
	delay_ms(ms);
#if _DEBUG && _DEBUG_AT_CMD
	trace_puts((char*)RxBuffer1);
#endif
#if _DEBUG_SIM_UART5 && _DEBUG_AT_CMD
	UART5_Send_String((uint8_t*)RxBuffer1);
#endif

}
static uint8_t sim_check_cmd(__IO uint8_t* response, uint8_t* cmd)
{
	char *res;
	res = strstr((char*)response,(char*)cmd);
	if (res) return 1;
	else return 0;
}
/*	Check Response SIM800C
 * 	Send AT Command
 * 	Response AT OK
 */
static uint8_t sim_check_response()
{
	u8 time_out=0;
	u8 r=0;
	//Check SIM after Start Up
#if	_DEBUG
	trace_puts("Check SIM Response.");
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)"Check SIM Response.\n");
#endif
	do{
		sim_send_cmd((uint8_t*)"AT\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"AT\r\r\nOK\r\n");
		time_out++;
	}while ((time_out<10)&&(!r));
	if (time_out>=10){
#if	_DEBUG
		trace_puts("SIM no Response.");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((uint8_t*)"SIM no Response.\n");
#endif
		return 0;
	}
#if	_DEBUG
	trace_puts("SIM Responsed.");
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)"SIM Response.\n");
#endif
	return 1;
}
/*	Check SIM Card SIM800C
 * 	Send AT+CPIN? Command
 * 	Response +CPIN: READY
 */
static uint8_t sim_check_simcard()
{
	u8 time_out=0;
	u8 r=0;
	//Check SIM Card
#if	_DEBUG
	trace_puts("Check SIM CARD.");
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)"Check SIM CARD.\n");
#endif
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CPIN?\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"+CPIN: READY\r\n");
		time_out++;
	}while ((time_out<10)&&(!r));
	if (time_out>=10){
#if	_DEBUG
		trace_puts("No SIM CARD.");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((uint8_t*)"No SIM CARD.\n");
#endif
		return 0;
	}
#if	_DEBUG
	trace_puts("SIM CARD OK.");
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)"SIM CARD OK.\n");
#endif
	return 1;
}
/*	Check SIM registration status
 * 	Send AT+CREG? Command
 * 	Response +CREG: 0,1
 */
static uint8_t sim_check_reg()
{
	u8 time_out=0;
	u8 r=0;
	//Check SIM Registration status
#if	_DEBUG
	trace_puts("Check SIM Registration status.");
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)"Check SIM Registration status.\n");
#endif
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CREG?\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"+CREG: 0,1\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	if (time_out>=20){
#if	_DEBUG
		trace_puts("Not registered.");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((uint8_t*)"Not registered.\n");
#endif
		return 0;
	}
#if	_DEBUG
	trace_puts("Registered, home network.");
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)"Registered, home network.\n");
#endif
	return 1;
}
uint8_t sim_init()
{
	if (!sim_check_response()) return 0;
	if (!sim_check_simcard()) return 0;
	if (!sim_check_reg()) return 0;
	return 1;
}

