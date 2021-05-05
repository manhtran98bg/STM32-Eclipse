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

extern __IO uint16_t RxCounter1;

extern SIM800_t *sim800;
bool txFlag = 0;
bool PINGRESP_Flag = 0;
bool mqtt_receive = 0;
unsigned char CONNACK_buffer[6]={0};
unsigned char CONNACK_index = 0;
char mqtt_buffer[1460] = {0};
uint16_t mqtt_index = 0;
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
uint8_t sim_power_status(SIM800_t *sim800)
{
	if (GPIO_ReadInputDataBit(GPIOC, GSM_STATUS)){
		sim800->power_state = ON;
		return 1;
	}
	else {
		sim800->power_state = OFF;
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
#if _DEBUG_SIM_UART5
		UART5_Send_String((char*)"Waiting SIM Power ON.\n");
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
#if _DEBUG_SIM_UART5
			UART5_Send_String((char*)"SIM can't turn on. Check SIM Power!\n");
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
#if _DEBUG_SIM_UART5
			UART5_Send_String((char*)"SIM Power State: ON\n");
#endif
			sim800->power_state = ON;
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"+CPIN: READY\r\n");
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
	char *temp;
	char signal_str[3]={0};
	do {
		r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
		time_out++;
		delay_ms(100);
	}while ((time_out<(timeout_ms/100))&&(!r));
	if (time_out>=(timeout_ms/100)) return NORESPONSE;
	temp = strstr((char*)RxBuffer1,(char*)"+CSQ:");
	if (temp!=NULL)
	{
		signal_str[0]=*(temp+6);
		signal_str[1]=*(temp+7);
		if (atoi(signal_str)!=0) signal = atoi(signal_str);
	}
	if (signal<2) return NOSIGNAL;
	if ((signal>=2)&&(signal<=9)) return MARGINAL;
	if ((signal>=10)&&(signal<=14)) return OK;
	if ((signal>=15)&&(signal<=19)) return GOOD;
	if ((signal>=20)&&(signal<=30)) return EXCELLENT;
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"+CREG: 0,1\r\n");
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"SHUT OK\r\n");
			delay_ms(100);
			time_out++;
		}while ((time_out<(timeout_ms/100))&&(!r));
		sim800->simState = sim_current_connection_status();
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
static uint8_t sim_set_APN(sim_t *sim_APN, char num_try, int timeout_ms)
{
	u8 time_out=0;
	u8 r=0;
	u8 repeat = 0;
	char str[64];
	for(repeat = 0;repeat<num_try;repeat++)
	{
		sim_log("Try to Set APN...");
		time_out=0;
		snprintf(str,sizeof(str),"AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n",sim_APN->apn,sim_APN->apn_user,sim_APN->apn_pass);
		sim_send_cmd((char*)str, 200);
		do{
			r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n");
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
	char *buff=(char*)malloc(40*sizeof(char));
	for (repeat=0;repeat<num_try;repeat++)
	{
		time_out = 0;
		sim_log("Get Local IP Address.");
		sim_send_cmd((char*)"AT+CIFSR\r\n",100);
		do{
			r = sim_check_cmd((char*)RxBuffer1, (char*)"ERROR\r\n");
			time_out++;
			delay_ms(100);
		}while ((time_out<(timeout_ms/100))&&(r));
		if (time_out>=(timeout_ms/100)){
			sim_log("Can not get Local IP Address. Try again");
		}
		else {
			memset(buff,0,40);
			sprintf((char*)buff,"Your Local IP Address: %s",RxBuffer1);
			sim_log((char*)buff);
			free(buff);
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
	if (!sim_check_response(max_try, time_out_ms)) return 0;
	if (!sim_check_simcard(max_try, time_out_ms)) return 0;
	if (!sim_check_reg(max_try, time_out_ms)) return 0;
	sim800->signal_condition=sim_check_signal_condition(sim800, 1000);
	if (!sim_attach_gprs(max_try, time_out_ms)) return 0;
	if (!sim_set_APN(sim800->sim,max_try,time_out_ms)) return 0;
	sim_send_cmd((char*)"AT+CIPQSEND=1\r\n", 1000);
	delay_ms(2000);
	USART_clear_buf(1);
	return 1;
}

uint8_t sim_send_message(unsigned char* message, uint8_t datalen)
{
	u8 time_out=0;
	u8 r=0;
	sim_send_cmd((char*)"AT+CIPSEND\r\n", 200);
	USART1_Send_Array(message, datalen);
	USART1_Send_Char(0x1A);
	do{
		r = sim_check_cmd((char*)RxBuffer1, (char*)"DATA ACCEPT");
		time_out++;
		delay_ms(100);
	}
	while ((time_out<10)&&(!r));
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
	char *buff=(char*)malloc(128*sizeof(char));
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
			r = sim_check_cmd((char*)RxBuffer1, (char*)"OK\r\n\r\nCONNECT OK");
			delay_ms(100);
			time_out++;
		}while ((time_out<(timeout_ms/100))&&(!r));
		if (time_out>=(timeout_ms/100)){
			sim800->tcp_connect=false;
			sim800->simState = sim_current_connection_status();
			memset(buff,0,128);
			sprintf((char*)buff,"Connecting to: %s:%d: FAILED. Try again",sim800->mqttServer.host,sim800->mqttServer.port);
			sim_log((char*)buff);
		}
		else {
			memset(buff,0,128);
			sprintf((char*)buff,"Connecting to: %s:%d: SUCCESS",sim800->mqttServer.host,sim800->mqttServer.port);
			sim_log((char*)buff);
			free(buff);
			sim800->tcp_connect = true;
			sim800->simState = sim_current_connection_status();
			return 1;
		}
	}
	memset(buff,0,128);
	sprintf((char*)buff,"Connecting to: %s:%d: FAILED",sim800->mqttServer.host,sim800->mqttServer.port);
	sim_log((char*)buff);
	free(buff);
	sim800->mqttServer.connect = false;
	sim800->simState = sim_current_connection_status();
	return 0;
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
uint8_t MQTT_Connect(SIM800_t *sim800){
	unsigned char buf[128] = {0};
	sim800->mqttReceive.newEvent = false;
	sim800->mqttServer.connect = false;
	if (sim800->tcp_connect == true)
	{
		MQTTPacket_connectData datas = MQTTPacket_connectData_initializer;
		datas.username.cstring = sim800->mqttClient.username;
		datas.password.cstring = sim800->mqttClient.pass;
		datas.clientID.cstring = sim800->mqttClient.clientID;
		datas.keepAliveInterval = sim800->mqttClient.keepAliveInterval;
		datas.cleansession = 1;
		int data_len = MQTTSerialize_connect(buf, sizeof(buf), &datas);
		sim_send_cmd((char*)"AT+CIPSEND\r\n", 200);
		USART1_Send_Array((unsigned char*)buf,data_len);
		USART1_Send_Char(0x1A);
		if (MQTT_PingReq(sim800)) return 1;
		else return 0;
	}
	return 0;
}
uint8_t MQTT_PingReq(SIM800_t *sim800)
{
	unsigned char ping_frame[2]={0xC0,0};
	uint8_t time_out=0;
	USART_clear_buf(1);
	PINGRESP_Flag = 0;
	sim_send_cmd((char*)"AT+CIPSEND\r\n", 200);
	USART1_Send_Array(ping_frame, 2);
	USART1_Send_Char(0x1A);
	while ((time_out<20)&&(PINGRESP_Flag==0)){
		time_out++;
		delay_ms(100);
	}
	if (time_out>=20) {
		sim800->mqttServer.connect = false;
		return 0;
	}
	else {
		sim800->mqttServer.connect = true;
		return 1;
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
void MQTT_Sub(MQTTString *topicString, int *requestedQoSs, int topic_count) {
    unsigned char buf[512] = {0};
    int data_len = MQTTSerialize_subscribe(buf, sizeof(buf), 0, 10, topic_count,
                                           topicString, requestedQoSs);
    sim_send_message((unsigned char*)buf,data_len);
}

/**
 * Receive message from MQTT broker
 * @param receive mqtt bufer
 * @return NONE
 */
void MQTT_Receive(unsigned char *buf) {
    memset(sim800->mqttReceive.topic, 0, sizeof(sim800->mqttReceive.topic));
    memset(sim800->mqttReceive.payload, 0, sizeof(sim800->mqttReceive.payload));
    MQTTString receivedTopic;
    unsigned char *payload;
    MQTTDeserialize_publish(&sim800->mqttReceive.dup, &sim800->mqttReceive.qos, &sim800->mqttReceive.retained,
                            &sim800->mqttReceive.msgId,
                            &receivedTopic, &payload, &sim800->mqttReceive.payloadLen, buf,
                            sizeof(buf));
    memcpy(sim800->mqttReceive.topic, receivedTopic.lenstring.data, receivedTopic.lenstring.len);
    sim800->mqttReceive.topicLen = receivedTopic.lenstring.len;
    memcpy(sim800->mqttReceive.payload, payload, sim800->mqttReceive.payloadLen);
    sim800->mqttReceive.newEvent = 1;
}
void clearMqttBuffer(void) {
    mqtt_receive = 0;
    mqtt_index = 0;
    memset(mqtt_buffer, 0, sizeof(mqtt_buffer));
}

void Sim800_RxCallBack(void) {
	unsigned char c;
	c = USART_ReceiveData(USART1);
	if (c== 0xD0) PINGRESP_Flag = 1;
	if (RxCounter1<BUFFER_SIZE1) RxBuffer1[RxCounter1++]=c;
	else RxCounter1 = 0;
#if (QoS==0)
	if (sim800->mqttServer.connect == 1 && c == 48 ) {
	        mqtt_receive = 1;
	}
#elif (QoS == 2)
	if (sim800->mqttServer.connect == 1 && c == 52 ) {
	        mqtt_receive = 1;
	}
#endif
    if (mqtt_receive == 1)
    {
        mqtt_buffer[mqtt_index++] = c;
        if (mqtt_index>2 && mqtt_buffer[2]!=0 && mqtt_buffer[3]!=13)
        {
        	mqtt_index = 0;
        	mqtt_receive = 0;
        }
        else if (mqtt_index > 1 && mqtt_index - 1 > mqtt_buffer[1]) {
            MQTT_Receive((unsigned char *) mqtt_buffer);
            clearMqttBuffer();
        }
        if (mqtt_index >= sizeof(mqtt_buffer)) {
            clearMqttBuffer();
        }
    }
}
