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
#if _DEBUG && _DEBUG_AT_CMD	//Gui len CMD ra man hinh debug
	trace_puts((char*)cmd);
#endif
	USART_clear_buf(1);
	USART1_Send_String(cmd);
	delay_ms(ms);
#if _DEBUG && _DEBUG_AT_CMD	//gui phan hoi cua sim len man hinh Debug
	trace_puts((char*)RxBuffer1);
#endif
#if _DEBUG_AT_UART5 && _DEBUG_AT_CMD	//gui phan hoi cua sim len UART5
	UART5_Send_String((uint8_t*)RxBuffer1);
#endif

}
static void sim_log(char* log)
{
#if	_DEBUG
	trace_puts(log);
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String((uint8_t*)log);
	UART5_Send_String((uint8_t*)"\n");
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
	sim_log("Check SIM Response.");
	sim_send_cmd((uint8_t*)"ATE0\r\n", 200);
	do{
		sim_send_cmd((uint8_t*)"AT\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"OK\r\n");
		time_out++;
	}while ((time_out<10)&&(!r));
	if (time_out>=10){
		sim_log("SIM not Response.");
		return 0;
	}
	sim_log("SIM Responsed.");
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
	sim_log("Check SIM CARD.");
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CPIN?\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"+CPIN: READY\r\n");
		time_out++;
	}while ((time_out<10)&&(!r));
	if (time_out>=10){
		sim_log("No SIM CARD.");
		return 0;
	}
	sim_log("SIM CARD OK.");
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
	sim_log("Check SIM Registration status.");
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CREG?\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"+CREG: 0,1\r\n");
		time_out++;
	}while ((time_out<60)&&(!r));
	if (time_out>=60){
		sim_log("Not registered.");
		return 0;
	}
	sim_log("Registered, home network.");
	return 1;
}
/* Ket noi GPRS
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
static uint8_t sim_attach_gprs()
{
	u8 time_out=0;
	u8 r=0;
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CGATT=1\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"OK\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	if (time_out>=20){
		sim_log("Attach GPRS Service:FAILED.");
		return 0;
	}
	sim_log("Attach GPRS Service:SUCCESSED");
	return 1;
}
/* Ngat ket noi GPRS
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
static uint8_t sim_detach_gprs()
{
	u8 time_out=0;
	u8 r=0;
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CIPSHUT\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"SHUT OK\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	if (time_out>=20){
		sim_log("Deactive GPRS Service:FAILED.");
		return 0;
	}
	sim_log("Deactive GPRS Service:SUCCESSED");
	return 1;
}
static uint8_t sim_set_APN()
{
	u8 time_out=0;
	u8 r=0;
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CSTT=\"m-wap\",\"mms\",\"mms\"\r\n", 1000);
//		sim_send_cmd((uint8_t*)"AT+CSTT=\"CMNET\"\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"OK\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	if (time_out>=20){
		sim_log("Set APN:FAILED.");
		return 0;
	}
	sim_log("Set APN:SUCCESSED.");
	return 1;
}
static uint8_t sim_bringup_wireless_connection()
{
	u8 time_out=0;
	u8 r=0;
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CIICR\r\n", 2000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"OK\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	if (time_out>=20){
		sim_log("Bring up wireless connection:FAILED.");
		return 0;
	}
	sim_log("Bring up wireless connection:SUCCESSED.");
	return 1;
}
static uint8_t sim_get_local_IP()
{
	u8 time_out=0;
	u8 r=0;
	char *buff=(char*)malloc(40*sizeof(char));
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CIFSR\r\n", 2000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"ERROR\r\n");
		time_out++;
	}while ((time_out<20)&&(r));
	if (time_out>=20){
		sim_log("Can not get Local IP Address!");
		return 0;
	}
	memset(buff,0,40);
	sprintf((char*)buff,"Your Local IP Address: %s",RxBuffer1);
	sim_log((char*)buff);
	return 1;
}
/* Dong ket noi TCP
 * Param: No
 * Return:		1:Success
 * 				0:Fail
 */
static uint8_t	sim_close_tcp()
{
	u8 time_out=0;
	u8 r=0;
	sim_log("Close TCP connection.");
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)"AT+CIPCLOSE\r\n", 1000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"CLOSE OK\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	if (time_out>=20){
		sim_log("Close TCP connection: FAILED.");
		return 0;
	}
	sim_log("Close TCP connection: SUCCESSED.");
	return 1;
}

/* Init Sim800
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
uint8_t sim_init()
{
	if (!sim_check_response()) return 0;
	if (!sim_check_simcard()) return 0;
	if (!sim_check_reg()) return 0;
	if (!sim_attach_gprs()) return 0;
	if (!sim_set_APN()) return 0;
	return 1;
}

uint8_t sim_send_message(uint8_t* message)
{
	u8 time_out=0;
	u8 r=0;
	sim_send_cmd((uint8_t*)"AT+CIPSEND\r\n", 200);
	USART_SendData(USART1, 0x10);
	USART1_Send_String((uint8_t*)message);
	USART_SendData(USART1, 0x1A);
	do{
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"SEND OK\r\n");
		time_out++;
		delay_ms(100);
	}
	while ((time_out<10)&&(!r));
	if (time_out>=10) return 0;
	return 1;
}
uint8_t sim_set_TCP_connection()
{
	if (!sim_bringup_wireless_connection()) return 0;
	if (!sim_get_local_IP()) return 0;
	return 1;
}
uint8_t sim_connect_server(server *myServer)
{
	u8 time_out=0;
	u8 r=0;
	char *buff=(char*)malloc(60*sizeof(char));
	memset(buff,0,60);
	sprintf((char*)buff,"Start connecting to: %s:%s",myServer->IP,myServer->Port);
	sim_log((char*)buff);
	memset(buff,0,60);
	sprintf((char*)buff,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",myServer->IP,myServer->Port);
	time_out = 0;
	do{
		sim_send_cmd((uint8_t*)buff, 4000);
		r = sim_check_cmd(RxBuffer1, (uint8_t*)"OK\r\n\r\nCONNECT OK");
		time_out++;
	}while ((time_out<5)&&(!r));
	if (time_out>=5){
		memset(buff,0,60);
		sprintf((char*)buff,"Connecting to: %s:%s: FAILED",myServer->IP,myServer->Port);
		myServer->state = sim_current_connection_status();
		return 0;
	}
	memset(buff,0,60);
	sprintf((char*)buff,"Connecting to: %s:%s: SUCCESS",myServer->IP,myServer->Port);
	myServer->state = sim_current_connection_status();
	sim_log((char*)buff);
	return 1;
}
state sim_current_connection_status()
{
	sim_send_cmd((uint8_t*)"AT+CIPSTATUS\r\n", 500);
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"IP INITIAL\r\n")) return IP_INITIAL;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"IP START\r\n")) return IP_START ;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"IP CONFIG\r\n")) return IP_CONFIG ;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"IP GPRSACT\r\n")) return IP_GPRSACT;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"IP STATUS\r\n")) return IP_STATUS;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"TCP CONNECTING")) return TCP_CONNECTING;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"CONNECT OK\r\n")) return CONNECT_OK;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"TCP CLOSING\r\n")) return TCP_CLOSING;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"TCP CLOSED\r\n")) return TCP_CLOSED;
	if (sim_check_cmd(RxBuffer1, (uint8_t*)"PDP DEACT\r\n")) return PDP_DEACT;
	return DEFAUT;

}
uint8_t sim_disconnect_server(server *myServer)
{
	if (myServer->state==CONNECT_OK){
		if(!sim_close_tcp()) return 0;
		myServer->state = sim_current_connection_status();
	}
	if(!sim_detach_gprs()) return 0;
	myServer->state = sim_current_connection_status();
	return 1;
}

