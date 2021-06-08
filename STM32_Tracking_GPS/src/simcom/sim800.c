/*
 * sim800.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "sim800.h"
#include "../service/delay.h"
#include "../usart/usart.h"
#include "../lcd/sh1106.h"


bool txFlag = 0;
bool PINGRESP_Flag = 0;
bool mqtt_receive = 0;
volatile uint32_t timeout_rx_topic=0;

char sub_topic_index=0;

char sim_buffer[SIM_BUFFER_SIZE];
uint16_t sim_buffer_index = 0;

uint16_t mqtt_index = 0;
signed char rssi_arr[31] = {-113,-111,-109,-107,-105,-103,-101,-99,-97,-95,
							-93,-91,-89,-87,-85,-83,-81,-79,-77,-75,
							-73,-71,-69,-67,-65,-63,-61,-59,-57,-55,-53
};
char res_packet_buffer[32]={0};
char res_packet_index = 0;
extern uint8_t nosignal_check;
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
static void sim_uart_clk_init()
{
	/*Enable UART clock and GPIO clock*/
	RCC_APB2PeriphClockCmd(SIM_UART_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(SIM_UART_GPIO_CLK, ENABLE);
}
static void sim_uart_gpio_init()
{
	GPIO_InitTypeDef gpio_init_structure;
	//Config USART1: PA9 = TX, PA10 = RX
	gpio_init_structure.GPIO_Pin = SIM_UART_GPIO_TX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SIM_UART_GPIO, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = SIM_UART_GPIO_RX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(SIM_UART_GPIO, &gpio_init_structure);
}
static void sim_uart_module_init()
{
	USART_InitTypeDef usart_init_structure;
	/* Baud rate 9600, 8-bit data, One stop bit
	* No parity, Do both Rx and Tx, No HW flow control
	*/
	usart_init_structure.USART_BaudRate = 115200;
	usart_init_structure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init_structure.USART_Parity = USART_Parity_No;
	usart_init_structure.USART_StopBits = USART_StopBits_1;
	usart_init_structure.USART_WordLength = USART_WordLength_8b;
	USART_Init(SIM_UART, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(SIM_UART, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(SIM_UART, ENABLE);
}
static void sim_uart_nvic_init()
{
	NVIC_InitTypeDef nvic_init_structure;
	nvic_init_structure.NVIC_IRQChannel = USART1_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&nvic_init_structure);
}
void sim_uart_init()
{
	sim_uart_clk_init();
	sim_uart_gpio_init();
	sim_uart_nvic_init();
	sim_uart_module_init();
}
/*	Function for Sim800 Usart */

/**
  * @brief  Clear uart buffer of sim800
  * @param 	None
  * @retval None
  */
void sim_uart_clear_buffer()
{
	for (int i=0;i<SIM_BUFFER_SIZE;i++) sim_buffer[i]=0;
	sim_buffer_index=0;
}
/**
  * @brief  Send a string to uart
  * @param 	str : Pointer to String or Array.
  * @retval None
  */
void sim_uart_send_string(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(SIM_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(SIM_UART,*str);
		str++;
	}
}
/**
  * @brief  Send a char to uart
  * @param 	chr: char need to send
  * @retval None
  */
void sim_uart_send_char( char chr)
{
	while(USART_GetFlagStatus(SIM_UART,USART_FLAG_TXE) == RESET);
	USART_SendData(SIM_UART,chr);
}
/**
  * @brief  Send array data to uart
  * @param 	str: pointer to array.
  * @param	length: data length
  * @retval None
  */
void sim_uart_send_array(unsigned char *str, uint8_t length)
{
	for(int i=0;i<length;i++)
	{
		while(USART_GetFlagStatus(SIM_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(SIM_UART,*(str+i));
	}
}

static void toggle_powerkey()
{
	GPIO_ResetBits(GPIOC, GSM_POWERKEY);
	delay_ms(1500);
	GPIO_SetBits(GPIOC, GSM_POWERKEY);
	delay_ms(500);
}
uint8_t sim_power_status(SIM800_t *sim800)
{
	if (GPIO_ReadInputDataBit(GPIOC, GSM_STATUS)){
		sim800->power_state = ON;
		return 1;
	}
	else {
		sim800->power_state = OFF;
		sim800->tcp_connect = false;
		sim800->mqttServer.connect = false;
		return 0;
	}
}
/*	Power On SIM800C
 */
uint8_t sim_power_on(SIM800_t *sim800)
{
	u8 time_out=0;
	if (!sim_power_status(sim800)) {
		toggle_powerkey();
#if	_DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("Waiting SIM Power ON.");
#endif
#if _USE_DEBUG_UART
		debug_send_string("log: Waiting SIM Power ON.\n");
#endif
		while((time_out++<100)) {
			if(sim_power_status(sim800)) break;
			delay_ms(100);
		}
		if (time_out>=100)
		{
#if	_DEBUG
			trace_write((char*)"log:", strlen("log:"));
			trace_puts("SIM can't turn on. Check SIM Power!");
#endif
#if _USE_DEBUG_UART
			debug_send_string((char*)"SIM can't turn on. Check SIM Power!\n");
#endif
			sim800->power_state = OFF;
			return 0;
		}
		else
		{
#if	_DEBUG
			trace_write((char*)"log:", strlen("log:"));
			trace_puts("SIM Power State: ON");
#endif
#if _USE_DEBUG_UART
			debug_send_string("log: SIM Power State: ON\n");
#endif
			sim800->power_state = ON;
			return 1;
		}
	}
	else{
#if _DEBUG
		trace_puts("SIM Power State: ALREADY ON");
#endif

#if _USE_DEBUG_UART
		debug_send_string("log: SIM Power State: ALREADY ON\n");
#endif
		sim800->power_state = ON;
		return 1;
	}
}

void sim_power_off(SIM800_t *sim800)
{
	if (sim_power_status(sim800)) {
		toggle_powerkey();
		while(sim_power_status(sim800));
#if	_DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("SIM Power State: OFF");
#endif
#if _USE_DEBUG_UART
		debug_send_string("log: SIM Power State: OFF\n");
#endif
	}
	else{
#if _DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("SIM Power State: ALREADY OFF");
#endif
#if _USE_DEBUG_UART
		debug_send_string("log: SIM Power State: ALREADY OFF\n");
#endif
	}
}
/**
  * @brief  Send Command to SIM800
  * @param 	cmd: AT Command need to send.
  * @param	ms: time_out(millisecond)
  * @retval None
  */
void sim_send_cmd(char* cmd, uint16_t ms)
{
#if _DEBUG && _DEBUG_AT_CMD	//Gui len CMD ra man hinh debug
	trace_write((char*)"cmd:", strlen("cmd:"));
	trace_puts((char*)cmd);
#endif
	sim_uart_clear_buffer();
	sim_uart_send_string(cmd);
	delay_ms(ms);
#if _DEBUG && _DEBUG_RES	//gui phan hoi cua sim len man hinh Debug
	trace_write((char*)"res:", strlen("res:"));
	trace_puts((char*)RxBuffer1);
#endif
#if _USE_DEBUG_UART && _DEBUG_RES	//gui phan hoi cua sim len Debug UART
	debug_send_string((uint8_t*)RxBuffer1);
#endif

}
/**
  * @brief  Check Respond data from SIM800
  * @param 	response: Pointer to uart_buffer - data from SIM800
  * @param	cmd: Pointer to string ( Command to check)
  * @retval	True: If find cmd in response
  * 		False: Can not find cmd in response
  */
static uint8_t sim_check_cmd(char* response, char* cmd)
{
	char *res;
	res = strstr(response,cmd);
	if (res) return 1;
	else return 0;
}
/**
  * @brief  Send Log to Debug terminal / uart5
  * @param 	log: Pointer to string need to log
  * @retval	None
  */
static void sim_log(char* log)
{
#if	_DEBUG
	trace_write((char*)"log:", strlen("log:"));
	trace_puts(log);
#endif
#if _USE_DEBUG_UART
	debug_send_string(log);
	debug_send_chr('\n');
#endif
}
/*	Check Response SIM800C
 * 	Send AT Command
 * 	Response AT OK
 */
static uint8_t sim_check_response(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat=0;
	//Check SIM after Start Up
	sim_send_cmd((char*)"ATE0\r\n", 200);	//Turn off Echo mode
	for (repeat=0;repeat<num_try;repeat++)
	{
		time_out=0;
		sim_log("Check SIM Response.");
		sim_send_cmd((char*)"AT\r\n", 100);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=10){
			sim_log("SIM not Response. Try again.");
		}
		else {
			sim_log("SIM Responsed.");
			return 1;
		}
	}
	sim_log("SIM not Response.");
	return 0;

}
/*	Check SIM Card SIM800C
 * 	Send AT+CPIN? Command
 * 	Response +CPIN: READY
 */
static uint8_t sim_check_simcard(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat=0;
	//Check SIM Card
	for (repeat=0;repeat<num_try;repeat++)
	{
		sim_log("Check SIM CARD.");
		time_out = 0;
		sim_send_cmd((char*)"AT+CPIN?\r\n", 100);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"+CPIN: READY\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
			sim_log("No SIM CARD. Try again.");
		}
		else {
			sim_log("SIM CARD OK.");
			return 1;
		}
	}
	sim_log("No SIM CARD.");
	return 0;

}
signal_t sim_check_signal_condition(SIM800_t *sim800, int timeout_ms)
{
	sim_send_cmd((char*)"AT+CSQ\r\n", 50);
	u8 time_out=0;
	u8 r=0;
	u8 signal = 0;
	char rssi_buff[4]={0};
	char *temp;
	char signal_str[3]={0};
	do {
		r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n");
		time_out++;
		delay_ms(50);
	}while ((time_out<(timeout_ms/50))&&(!r));
	if (time_out>=(timeout_ms/50)) return NORESPONSE;
	temp = strstr((char*)sim_buffer,(char*)"+CSQ:");
	if (temp!=NULL)
	{
		signal_str[0]=*(temp+6);
		signal_str[1]=*(temp+7);
		if (atoi(signal_str)!=0) signal = atoi(signal_str);
	}
	sprintf(sim800->rssi,"%d",rssi_arr[signal]);
	if ((signal>=2)&&(signal<=9)) return MARGINAL;
	if ((signal>=10)&&(signal<=14)) return OK;
	if ((signal>=15)&&(signal<=19)) return GOOD;
	if ((signal>=20)&&(signal<=30)) return EXCELLENT;
	return NOSIGNAL;
}
void sim_get_id(SIM800_t *sim800)
{
	u8 r=0;
	u8 i=0,index=0;
	char temp[20]={0};
	//Get IMEI
	sim_send_cmd((char*)"AT+GSN\r\n", 100);
	do {
		r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n");
	}while (!r);
	for (i=2;i<17;i++)temp[index++]=sim_buffer[i];
	strcpy(sim800->sim_id.imei,temp);
	delay_ms(100);
}
/*	Check SIM registration status
 * 	Send AT+CREG? Command
 * 	Response +CREG: 0,1
 */
static uint8_t sim_check_reg(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat=0;
	//Check SIM Registration status
	for (repeat=0;repeat<num_try;repeat++)
	{
		sim_log("Check SIM Registration status.");
		time_out = 0;
		sim_send_cmd((char*)"AT+CREG?\r\n", 100);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"+CREG: 0,1\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
			sim_log("Not registered. Try again.");
		}
		else {
			sim_log("Registered.");
			return 1;
		}
	}
	sim_log("Not registered.");
	return 0;
}
/* Ket noi GPRS
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
uint8_t sim_attach_gprs(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat = 0;
	for(repeat = 0;repeat<num_try;repeat++)
	{
		sim_log("Try to Attach GPRS Service...");
		time_out = 0;
		sim_send_cmd((char*)"AT+CGATT=1\r\n", 200);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n");
			delay_ms(100);
			time_out++;
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
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
uint8_t sim_detach_gprs(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat = 0;
	for(repeat = 0;repeat<num_try;repeat++)
	{
		sim_log("Try to Deactive GPRS Service...");
		time_out = 0;
		sim_send_cmd((char*)"AT+CIPSHUT\r\n", 200);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"SHUT OK\r\n");
			delay_ms(100);
			time_out++;
		}while ((time_out<(timeout_ms/100))&&(!r));
		sim800.simState = sim_current_connection_status();
		if (time_out>=(timeout_ms/100)){
			sim_log("Deactive GPRS Service:FAILED. Try again");
		}
		else {
			sim_log("Deactive GPRS Service:SUCCESSED");
			return 1;
		}
	}
	sim_log("Deactive GPRS Service:FAILED.");
	return 0;
}
static uint8_t sim_set_APN(SIM800_t *sim800, char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat = 0;
	char str[64];
	for(repeat = 0;repeat<num_try;repeat++)
	{
		sim_log("Try to Set APN...");
		time_out=0;
		snprintf(str,sizeof(str),"AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n",sim800->sim.apn,sim800->sim.apn_user,sim800->sim.apn_pass);
		sim_send_cmd((char*)str, 200);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n");
			delay_ms(100);
			time_out++;
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
			sim_log("Set APN:FAILED. Try again.");
		}
		else {
			sim_log("Set APN:SUCCESSED");
			return 1;
		}
	}
	sim_log("Set APN:FAILED.");
	return 0;
}
static uint8_t sim_bringup_wireless_connection(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat=0;
	for (repeat=0;repeat<num_try;repeat++)
	{
		time_out = 0;
		sim_log("Bring up wireless connection.");
		sim_send_cmd((char*)"AT+CIICR\r\n", 100);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
			sim_log("Bring up wireless connection:FAILED. Try again.");
		}
		else {
			sim_log("Bring up wireless connection:SUCCESSED.");
			return 1;
		}
	}
	sim_log("Bring up wireless connection:FAILED.");
	return 0;
}
static uint8_t sim_get_local_IP(char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat=0;
	char buff[128]={0};
	for (repeat=0;repeat<num_try;repeat++)
	{
		time_out = 0;
		sim_log("Get Local IP Address.");
		sim_send_cmd((char*)"AT+CIFSR\r\n",100);
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"ERROR\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<(timeout_ms/100))&&(r));
		if (time_out>=(timeout_ms/100)){
			sim_log("Can not get Local IP Address. Try again");
		}
		else {
			memset(buff,0,40);
			sprintf((char*)buff,"Your Local IP Address: %s",sim_buffer);
			sim_log((char*)buff);
			return 1;
		}
	}
	sim_log("Can not get Local IP Address!");
	return 0;
}


/* Init Sim800
 * Param: No
 * Return:		1:SUCCESS
 * 				0:FAIL
 */
uint8_t sim_init(SIM800_t *sim800)
{
	char max_try = 10;
	int time_out_ms  = 3000;
	sim_uart_init();
	if (sim800->power_state==OFF) {
		sim800->sim_err = NO_PWR;
		return 0;
	}
	if (!sim_check_response(max_try, time_out_ms)) {
		sim800->sim_err = NO_RES;
		return 0;
	}
	if (!sim_check_simcard(max_try, time_out_ms)) {
		sim800->sim_err = NO_SIM;
		return 0;
	}
	if (!sim_check_reg(max_try, time_out_ms)) {
		sim800->sim_err = NO_REG;
		return 0;
	}
	sim800->signal_condition=sim_check_signal_condition(sim800, 1000);
	sim_get_id(sim800);
	if (!sim_attach_gprs(max_try, time_out_ms)) {
		sim800->sim_err = NO_GPRS;
		return 0;
	}
	if (!sim_set_APN(sim800,max_try,time_out_ms)) {
		sim800->sim_err = NO_APN;
		return 0;
	}
	sim_send_cmd((char*)"AT+CIPQSEND=1\r\n", 1000);
	delay_ms(1000);
	sim_uart_clear_buffer();
	sim800->sim_err = NO_ERR;
	return 1;
}

void sim_error_handler(void)
{
	if (sim800.sim_err == NO_PWR) {
		sh1106_WriteString(2, 0, "NO POWER", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (sim800.sim_err == NO_RES) {
		sh1106_WriteString(2, 0, "NO RES", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (sim800.sim_err == NO_SIM) {
		sh1106_WriteString(2, 0, "NO SIM", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (sim800.sim_err == NO_REG) {
		sh1106_WriteString(2, 0, "NO REG", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (sim800.sim_err == NO_GPRS) {
		sh1106_WriteString(2, 0, "NO GPRS", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (sim800.sim_err == NO_APN) {
		sh1106_WriteString(2, 0, "NO APN", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (sim800.sim_err == NO_ERR) {
		sh1106_WriteString(2, 0, "NO ERROR", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
}
uint8_t sim_send_message(unsigned char* message, uint8_t datalen)
{
	u8 time_out=0;
	u8 r=0;
	char accept_buffer[20]={0};
	sim800.send_state = NO_SEND;
	sprintf(accept_buffer,"DATA ACCEPT:%d\r\n",datalen);
	sim_send_cmd((char*)"AT+CIPSEND\r\n", 50);
	sim_uart_send_array(message, datalen);
	sim_uart_send_char(0x1A);
	sim800.send_state = SENDING;
	do{
		r = sim_check_cmd((char*)sim_buffer, accept_buffer);
		if (r) break;
		else{
			time_out++;
			delay_ms(50);
		}
	}
	while ((time_out<10)&&(!r));
	sim800.send_state = SENT;
	if (time_out>=10) return 0;
	return 1;
}
uint8_t sim_set_TCP_connection()
{
	char max_try = 10;
	int time_out_ms  = 3000;
	if (!sim_bringup_wireless_connection(max_try, time_out_ms)) return 0;
	delay_ms(200);
	if (!sim_get_local_IP(max_try, time_out_ms)) return 0;
	delay_ms(200);
	return 1;
}
uint8_t sim_connect_server(SIM800_t *sim800, char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat = 0;
	char buff[128]={0};
	memset(buff,0,128);
	sprintf((char*)buff,"Start connecting to: %s:%d",sim800->mqttServer.host,sim800->mqttServer.port);
	sim_log((char*)buff);
	for (repeat=0;repeat<num_try;repeat++)
	{
		memset(buff,0,128);
		sprintf((char*)buff,"AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n",sim800->mqttServer.host,sim800->mqttServer.port);
		sim_send_cmd((char*)buff, 200);
		time_out = 0;
		do{
			r = sim_check_cmd((char*)sim_buffer, (char*)"OK\r\n\r\nCONNECT OK");
			delay_ms(100);
			time_out++;
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
			sim800->tcp_connect=false;
			sim800->simState = sim_current_connection_status();
			memset(buff,0,128);
			sprintf((char*)buff,"Connecting to: %s:%d: FAILED. Try again",sim800->mqttServer.host,sim800->mqttServer.port);
			sim_log((char*)buff);
			sim_disconnect_server(sim800);
		}
		else {
			memset(buff,0,128);
			sprintf((char*)buff,"Connecting to: %s:%d: SUCCESS",sim800->mqttServer.host,sim800->mqttServer.port);
			sim_log((char*)buff);
			sim800->tcp_connect = true;
			sim800->simState = sim_current_connection_status();
			return 1;
		}
	}
	memset(buff,0,128);
	sprintf((char*)buff,"Connecting to: %s:%d: FAILED",sim800->mqttServer.host,sim800->mqttServer.port);
	sim_log((char*)buff);
	sim800->mqttServer.connect = false;
	sim800->simState = sim_current_connection_status();
	return 0;
}

state sim_current_connection_status()
{
	sim_uart_clear_buffer();
	sim_uart_send_string((char*)"AT+CIPSTATUS\r\n");
	delay_ms(500);
	if (sim_check_cmd((char*)sim_buffer, (char*)"IP INITIAL\r\n")) return IP_INITIAL;
	if (sim_check_cmd((char*)sim_buffer, (char*)"IP START\r\n")) return IP_START ;
	if (sim_check_cmd((char*)sim_buffer, (char*)"IP CONFIG\r\n")) return IP_CONFIG ;
	if (sim_check_cmd((char*)sim_buffer, (char*)"IP GPRSACT\r\n")) return IP_GPRSACT;
	if (sim_check_cmd((char*)sim_buffer, (char*)"IP STATUS\r\n")) return IP_STATUS;
	if (sim_check_cmd((char*)sim_buffer, (char*)"TCP CONNECTING")) return TCP_CONNECTING;
	if (sim_check_cmd((char*)sim_buffer, (char*)"CONNECT OK\r\n")) return CONNECT_OK;
	if (sim_check_cmd((char*)sim_buffer, (char*)"TCP CLOSING\r\n")) return TCP_CLOSING;
	if (sim_check_cmd((char*)sim_buffer, (char*)"TCP CLOSED\r\n")) return TCP_CLOSED;
	if (sim_check_cmd((char*)sim_buffer, (char*)"PDP DEACT\r\n")) return PDP_DEACT;
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
			r = sim_check_cmd((char*)sim_buffer, (char*)"CLOSE OK\r\n");
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
		sim800->tcp_connect = false;
		return 1;
	}
	return 0;
}
uint8_t sim_nosignal_handler(SIM800_t *sim800)
{
	u8 ping_res=0;
	//Check Signal Condition
	ping_res = MQTT_PingReq(sim800);
		if (sim800->signal_condition<=MARGINAL && sim800->tcp_connect == true && ping_res==0){
	#if _USE_DEBUG_UART
			debug_send_string((char*)"Check connection with MQTT Broker: ");
	#endif
			ping_res = MQTT_PingReq(sim800);
			if (ping_res==0 ){	//Poor Signal Condition.
	#if _USE_DEBUG_UART
				debug_send_string((char*)"Poor Signal Condition.\n");
				debug_send_string((char*)"Disconnecting from MQTT Broker.\n");
	#endif
				sim_disconnect_server(sim800);	//Ngat ket noi TCP/IP
				sim800->simState = sim_current_connection_status();	//Check TCP/IP status
				nosignal_check = 1;
				return 1;
			}
		}
		else if (ping_res == 0)
		{
			sim800->simState = sim_current_connection_status();	//Check TCP/IP status
			if (sim800->simState == CONNECT_OK)
			{
				sim_disconnect_server(sim800);	//Ngat ket noi TCP/IP
				nosignal_check = 1;
				return 1;
			}
			else if (sim800->simState == TCP_CLOSED)
			{
				sim800->tcp_connect = false;
				nosignal_check = 1;
				return 1;
			}
		}
	return 0;
}
void sim_reconnect_handler(SIM800_t *sim800)
{
	if (sim800->signal_condition>MARGINAL && sim800->tcp_connect==false) {	//Signal Condition OK, GOOD, EXCELLENT but Disconnected
		if (sim800->simState== TCP_CLOSED) {	//Neu da ngat ket noi TCP/IP
			if (sim_connect_server(sim800,1,2000)) {
				delay_ms(200);
				MQTT_Connect(sim800);			//Ket noi MQTT Broker.
				MQTT_Pub(pub_topicList[1].cstring,(char*) "ready");
				nosignal_check =  0;
			}
		}
		else {
			sim_detach_gprs(1, 1000);
			delay_ms(200);
			if (sim_attach_gprs(1, 1000))
				if (sim_connect_server(sim800,3,2000)) {
					delay_ms(500);
					MQTT_Connect(sim800);			//Ket noi MQTT Broker.
					MQTT_Pub(pub_topicList[1].cstring,(char*) "ready");
					nosignal_check =  0;
				}
		}
	}
}
uint8_t MQTT_Connect(SIM800_t *sim800){
	unsigned char buf[128] = {0};
	sim800->mqttReceive.newEvent = false;
	sim800->mqttServer.connect = false;
	if (sim800->tcp_connect == true)
	{
		MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
//		datas.username.cstring = sim800->mqttClient.username;
//		datas.password.cstring = sim800->mqttClient.pass;
		datas.clientID.cstring = sim800->mqttClient.clientID;
		datas.keepAliveInterval = sim800->mqttClient.keepAliveInterval;
		datas.cleansession = 1;
		datas.willFlag = 1;
		datas.will.qos = 1;
		datas.will.retained = 1;
		datas.will.topicName.cstring = pub_topicList[1].cstring;
		datas.will.message.cstring = "lost";
		int data_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);
//		sim_send_cmd((char*)"AT+CIPSEND\r\n", 200);
//		sim_uart_send_array((unsigned char*)buf,data_len);
//		sim_uart_send_char(0x1A);
//		delay_ms(500);
//		if (MQTT_PingReq(sim800)) return 1;
//		else return 0;
		memset(res_packet_buffer,0,32);
	    res_packet_index=0;
	    sim800->send_packet.name = CONNECT;
	    sim800->send_packet.flag = 1;
	    sim_send_message(buf, data_len);
	    while (res_packet_index<4);
	    sim800->res_packet.flag = 0 ;
	    if (res_packet_buffer[3]==0x00) {
	    	sim800->mqttServer.connect = true;
	    	return 1;
	    }
	    else {
	    	sim800->mqttServer.connect = false;
	    	return 0;
	    }
	}
	return 0;
}
uint8_t MQTT_PingReq(SIM800_t *sim800)
{
	unsigned char ping_frame[2]={0xC0,0};
	uint32_t time_out=0;
	memset(res_packet_buffer,0,32);
    res_packet_index=0;
    sim800->send_packet.name = PINGREQ;
    sim800->send_packet.flag = 1;
    sim800->res_packet.flag = 0 ;
    sim_send_message(ping_frame, 2);
    time_out = millis();
    while (res_packet_index<2){
    	if (millis()-time_out>2000) {
    		sim800->mqttServer.connect = false;
    		return 0;
    	}
    }
    sim800->res_packet.flag = 0 ;
    if (res_packet_buffer[1]==0x00) {
    	sim800->mqttServer.connect = true;
    	return 1;
    }
    else {
    	sim800->mqttServer.connect = false;
    	return 0;
    }
//	PINGRESP_Flag = 0;
//	sim_send_message(ping_frame, 2);
//	while ((time_out<10)&&(PINGRESP_Flag==0)){
//		time_out++;
//		delay_ms(100);
//	}
//	if (time_out>=10) {
//		sim800->mqttServer.connect = false;
//		return 0;
//	}
//	else {
//		sim800->mqttServer.connect = true;
//		return 1;
//	}
}
uint8_t MQTT_PUBACK(SIM800_t *sim800)
{
	unsigned char puback_frame[4]={0x40,0x02,0x00,0x00};
	puback_frame[2] = (sim800->mqttReceive.msgId>>8)&0xff;
	puback_frame[3] = sim800->mqttReceive.msgId&0xff;
	if (sim_send_message(puback_frame, 4))return 1;
	else return 0;
}
void MQTT_Pub(char *topic, char *payload) {
    unsigned char buf[256] = {0};

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;

    int data_len = MQTTSerialize_publish(buf, sizeof(buf), 0, 0, 1, 0,
                                         topicString, (unsigned char *) payload, (int) strlen(payload));
    sim800.send_packet.name = PUBLISH;
    sim800.send_packet.flag = 1;
    sim_send_message((unsigned char*)buf,data_len);
}
bool MQTT_Sub(MQTTString *topicString, int *requestedQoSs, int topic_count) {
    unsigned char buf[512] = {0};
    uint32_t time_out=0;
    int data_len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 10, topic_count,
                                           topicString, requestedQoSs);
    memset(res_packet_buffer,0,32);
    res_packet_index=0;
    sim800.send_packet.name = SUBSCRIBE;
    sim800.send_packet.flag = 1;
    sim800.res_packet.flag = 0 ;
    sim_send_message((unsigned char*)buf,data_len);
    time_out = millis();
    while (res_packet_index<topic_count+3){
		if (millis()-time_out>2000) {
			return 0;
		}
	}
    sim800.res_packet.flag = 0 ;
    if (res_packet_buffer[1]==2+topic_count && res_packet_buffer[3]==10) return 1;
    else return 0;
}

/**
 * Receive message from MQTT broker
 * @param receive mqtt bufer
 * @return NONE
 */
void MQTT_Receive(unsigned char *buf) {
    memset(sim800.mqttReceive.topic, 0, sizeof(sim800.mqttReceive.topic));
    memset(sim800.mqttReceive.payload, 0, sizeof(sim800.mqttReceive.payload));
    MQTTString receivedTopic;
    unsigned char *payload;
    MQTTDeserialize_publish(&sim800.mqttReceive.dup, &sim800.mqttReceive.qos, &sim800.mqttReceive.retained,
                            &sim800.mqttReceive.msgId,
                            &receivedTopic, &payload, &sim800.mqttReceive.payloadLen, buf,
                            sizeof(buf));
    if (receivedTopic.lenstring.len>4){
    	memcpy(sim800.mqttReceive.topic, receivedTopic.lenstring.data, receivedTopic.lenstring.len);
    	sim800.mqttReceive.topicLen = receivedTopic.lenstring.len;
    }
    if (sim800.mqttReceive.payloadLen>0)
    	memcpy(sim800.mqttReceive.payload, payload, sim800.mqttReceive.payloadLen);
    sim800.mqttReceive.newEvent = 1;
}
void clearMqttBuffer() {
    mqtt_receive = 0;
    mqtt_index = 0;
	sub_topic_index = 0;
    for (int i=0;i<NUM_SUB_TOPIC;i++)
    	for (int j=0;j<256;j++) mqtt_buffer[i][j]=0;
}

void Sim800_RxCallBack(void) {
	unsigned char c;
	c = USART_ReceiveData(SIM_UART);
//	debug_send_chr(c);
	if (sim_buffer_index<SIM_BUFFER_SIZE) sim_buffer[sim_buffer_index++]=c;
	else sim_buffer_index = 0;
	if (sim800.send_state == SENT || sim800.send_state == NO_SEND){
		if (sim800.send_packet.name == SUBSCRIBE && c == 0x90 ){	//SUBACK Packet
			sim800.res_packet.name = SUBACK;
			sim800.res_packet.flag = 1;
			res_packet_index=0;
		}
		else if (sim800.send_packet.name == PINGREQ && c == 0xD0 ){	//PINGRESP Packet
			sim800.res_packet.name = PINGRESP;
			sim800.res_packet.flag = 1;
			res_packet_index=0;
		}
		else if (sim800.send_packet.name == CONNECT && c== 0x20){	//CONNACK Packet
			sim800.res_packet.name = CONNACK;
			sim800.res_packet.flag = 1;
			res_packet_index=0;
		}
		if (sim800.res_packet.flag == 1) res_packet_buffer[res_packet_index++]=c;
	}
	if (sim800.mqttServer.connect == 1 && c == 0x30  ) {	//QoS0
		if (sim800.send_state == SENT || sim800.send_state == NO_SEND){
			if (sub_topic_index == 0 ) {
				timeout_rx_topic = millis();	//Bat dau dem de Time out
				sub_topic_rx_data_flag = true;
			}
	        mqtt_receive = 1;
		}
	}
    if (mqtt_receive == 1)
    {
        mqtt_buffer[sub_topic_index][mqtt_index++] = c;
        if (mqtt_index > 1 && mqtt_index - 1 > mqtt_buffer[sub_topic_index][1]) {
        	//MQTT_Receive((unsigned char *) mqtt_buffer);
        	if (sub_topic_index<NUM_SUB_TOPIC) 	sub_topic_index++;
        	else sub_topic_index = 0;
        	mqtt_index = 0;
        	mqtt_receive = 0;
        }
    }
}
