/*
 * sim800.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "sim800.h"
#include "../service/delay.h"
#include "../usart/usart.h"
extern __IO char RxBuffer1[];
char buffer[128]={0};
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
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("Waiting SIM Power ON.");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((char*)"Waiting SIM Power ON.\n");
#endif
		while((time_out++<10)) {
			if(sim_power_status()) break;
			delay_ms(1000);
		}
		if (time_out>=10)
		{
#if	_DEBUG
			trace_write((char*)"log:", strlen("log:"));
			trace_puts("SIM can't turn on. Check SIM Power!");
#endif
#if _DEBUG_SIM_UART5
			UART5_Send_String((char*)"SIM can't turn on. Check SIM Power!\n");
#endif
			return 0;
		}
		else
		{
#if	_DEBUG
			trace_write((char*)"log:", strlen("log:"));
			trace_puts("SIM Power State: ON");
#endif
#if _DEBUG_SIM_UART5
			UART5_Send_String((char*)"SIM Power State: ON\n");
#endif
			return 1;
		}
	}
	else{
#if _DEBUG
		trace_puts("SIM Power State: ALREADY ON");
#endif
#if _DEBUG_SIM_UART5
		UART5_Send_String((char*)"SIM Power State: ALREADY ON\n");
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
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("SIM Power State: OFF");
#endif
	}
	else{
#if _DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("SIM Power State: ALREADY OFF");
#endif
	}
}
void sim_send_cmd(char* cmd, uint16_t ms)
{
#if _DEBUG && _DEBUG_AT_CMD	//Gui len CMD ra man hinh debug
	trace_write((char*)"cmd:", strlen("cmd:"));
	trace_puts((char*)cmd);
#endif
	USART_clear_buf(1);
	USART1_Send_String(cmd);
	delay_ms(ms);
#if _DEBUG && _DEBUG_AT_CMD	//gui phan hoi cua sim len man hinh Debug
	trace_write((char*)"res:", strlen("res:"));
	trace_puts((char*)RxBuffer1);
#endif
#if _DEBUG_AT_UART5 && _DEBUG_AT_CMD	//gui phan hoi cua sim len UART5
	UART5_Send_String((uint8_t*)RxBuffer1);
#endif

}
static uint8_t sim_check_cmd(char* response, char* cmd)
{
	char *res;
	res = strstr(response,cmd);
	if (res) return 1;
	else return 0;
}
static void sim_log(char* log)
{
#if	_DEBUG
	trace_write((char*)"log:", strlen("log:"));
	trace_puts(log);
#endif
#if _DEBUG_SIM_UART5
	UART5_Send_String(log);
	UART5_Send_String((char*)"\n");
#endif
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
	sim_send_cmd((char*)"ATE0\r\n", 200);
	do{
		sim_send_cmd((char*)"AT\r\n", 1000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
		sim_send_cmd((char*)"AT+CPIN?\r\n", 1000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"+CPIN: READY\r\n");
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
		sim_send_cmd((char*)"AT+CREG?\r\n", 1000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"+CREG: 0,1\r\n");
		time_out++;
	}while ((time_out<60)&&(!r));
	if (time_out>=60){
		sim_log("Not registered.");
		return 0;
	}
	sim_log("Registered.");
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
	u8 repeat = 0;
	for(repeat = 0;repeat<10;repeat++)
	{
		sim_log("Try to Attach GPRS Service...");
		time_out = 0;
		sim_send_cmd((char*)"AT+CGATT=1\r\n", 1000);
		do{
			r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
			delay_ms(100);
			time_out++;
		}while ((time_out<30)&&(!r));
		if (time_out>=30){
			sim_log("Attach GPRS Service:FAILED. Try again");
		}
		else {sim_log("Attach GPRS Service:SUCCESSED");
		return 1;
		}
	}
	sim_log("Attach GPRS Service:FAILED.");
	return 0;
}
/* Ngat ket noi GPRS
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
uint8_t sim_detach_gprs(SIM800_t *sim800)
{
	u8 time_out=0;
	u8 r=0;
	time_out = 0;
	do{
		sim_send_cmd((char*)"AT+CIPSHUT\r\n", 1000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"SHUT OK\r\n");
		time_out++;
	}while ((time_out<20)&&(!r));
	sim800->simState = sim_current_connection_status();
	if (time_out>=20){
		sim_log("Deactive GPRS Service:FAILED.");
		return 0;
	}
	sim_log("Deactive GPRS Service:SUCCESSED");
	return 1;
}
static uint8_t sim_set_APN(sim_t *sim_APN)
{
	u8 time_out=0;
	u8 r=0;
	char str[32];
	sim_log("Set APN.");
	snprintf(str,sizeof(str),"AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n",sim_APN->apn,sim_APN->apn_user,sim_APN->apn_pass);
	do{
		sim_send_cmd((char*)str, 1000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
	sim_log("Bring up wireless connection.");
	do{
		sim_send_cmd((char*)"AT+CIICR\r\n", 2000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
	sim_log("Get Local IP Address.");
	do{
		sim_send_cmd((char*)"AT+CIFSR\r\n", 2000);
		r = sim_check_cmd((char*)RxBuffer1, (char*)"ERROR\r\n");
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


/* Init Sim800
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
uint8_t sim_init(SIM800_t *sim800)
{

	if (!sim_check_response()) return 0;
	if (!sim_check_simcard()) return 0;
	if (!sim_check_reg()) return 0;
	sim_send_cmd((char*)"AT+CSQ\r\n", 1000);
	if (!sim_attach_gprs()) return 0;
	if (!sim_set_APN(sim800->sim)) return 0;
	return 1;
}

uint8_t sim_send_message(unsigned char* message, uint8_t datalen)
{
	u8 time_out=0;
	u8 r=0;
	sim_send_cmd((char*)"AT+CIPSEND\r\n", 200);
	USART1_Send_Array(message, datalen);
	UART5_Send_Array(message, datalen);
	USART_SendData(USART1, 0x1A);
	USART_SendData(UART5, 0x1A);
	do{
		r = sim_check_cmd((char*)RxBuffer1, (char*)"SEND OK\r\n");
		time_out++;
		delay_ms(100);
	}
	while ((time_out<30)&&(!r));
	if (time_out>=10) return 0;
	return 1;
}
uint8_t sim_set_TCP_connection()
{
	if (!sim_bringup_wireless_connection()) return 0;
	if (!sim_get_local_IP()) return 0;
	return 1;
}
uint8_t sim_connect_server(SIM800_t *sim800)
{
	u8 time_out=0;
	u8 r=0;
	char *buff=(char*)malloc(128*sizeof(char));
	memset(buff,0,128);
	sprintf((char*)buff,"Start connecting to: %s:%d",sim800->mqttServer.host,sim800->mqttServer.port);
	sim_log((char*)buff);
	memset(buff,0,128);
	sprintf((char*)buff,"AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n",sim800->mqttServer.host,sim800->mqttServer.port);
	sim_send_cmd((char*)buff, 2000);
	time_out = 0;
	do{
		r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n\r\nCONNECT OK");
		delay_ms(100);
		time_out++;
	}while ((time_out<50)&&(!r));
	if (time_out>=5){
		memset(buff,0,128);
		sprintf((char*)buff,"Connecting to: %s:%d: FAILED",sim800->mqttServer.host,sim800->mqttServer.port);
		sim800->simState = sim_current_connection_status();
		sim800->mqttServer.connect = false;
		return 0;
	}
	memset(buff,0,128);
	sprintf((char*)buff,"Connecting to: %s:%d: SUCCESS",sim800->mqttServer.host,sim800->mqttServer.port);
	sim800->mqttServer.connect = true;
	sim800->simState = sim_current_connection_status();
	sim_log((char*)buff);
	free(buff);
	return 1;
}

state sim_current_connection_status()
{
	USART_clear_buf(1);
	USART1_Send_String((char*)"AT+CIPSTATUS\r\n");
	delay_ms(500);
	if (sim_check_cmd((char*)RxBuffer1, (char*)"IP INITIAL\r\n")) return IP_INITIAL;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"IP START\r\n")) return IP_START ;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"IP CONFIG\r\n")) return IP_CONFIG ;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"IP GPRSACT\r\n")) return IP_GPRSACT;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"IP STATUS\r\n")) return IP_STATUS;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"TCP CONNECTING")) return TCP_CONNECTING;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"CONNECT OK\r\n")) return CONNECT_OK;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"TCP CLOSING\r\n")) return TCP_CLOSING;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"TCP CLOSED\r\n")) return TCP_CLOSED;
	if (sim_check_cmd((char*)RxBuffer1, (char*)"PDP DEACT\r\n")) return PDP_DEACT;
	return DEFAUT;

}
/* Dong ket noi TCP
 * Param: No
 * Return:		1:Success
 * 				0:Fail
 */
uint8_t sim_disconnect_server(SIM800_t* sim800)
{
	u8 time_out=0;
	u8 r=0;
	if (sim800->simState==CONNECT_OK){
		sim_log("Close TCP connection.");
		time_out = 0;
		sim_send_cmd((char*)"AT+CIPCLOSE\r\n", 1000);
		do{
			r = sim_check_cmd((char*)RxBuffer1, (char*)"CLOSE OK\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<30)&&(!r));
		sim800->simState = sim_current_connection_status();
		if (time_out>=30){
			sim_log("Close TCP connection: FAILED.");
			return 0;
		}
		sim_log("Close TCP connection: SUCCESSED.");
		sim800->mqttServer.connect = false;
		return 1;
	}
	return 0;
}
void MQTT_connect(SIM800_t *sim800){
	unsigned char buf[128] = {0};
	sim800->mqttReceive.newEvent = false;
	if (sim800->mqttServer.connect == true)
	{
		MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
		datas.username.cstring = sim800->mqttClient.username;
		datas.password.cstring = sim800->mqttClient.pass;
		datas.clientID.cstring = sim800->mqttClient.clientID;
		datas.keepAliveInterval = sim800->mqttClient.keepAliveInterval;
		datas.cleansession = 1;
		int data_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);
		sim_send_message((unsigned char*)buf,data_len);
	}
}
void MQTT_Pub(char *topic, char *payload) {
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int data_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 0, 0,
                                         topicString, (unsigned char *) payload, (int) strlen(payload));
    sim_send_message((unsigned char*)buf,data_len);
}


