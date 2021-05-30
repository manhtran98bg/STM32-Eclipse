/*
 * rs232.c
 *
 *  Created on: May 23, 2021
 *      Author: manht
 */
#include "rs232.h"
unsigned char rs232_rx_buffer[RS232_BUFFER_SIZE]= {0};
int rs232_rx_index = 0;
bool rs232_rx_start_flag = 0;
bool rs232_rx_stop_flag = 0;
rx_frame_t rs232_rx_frame;

static void rs232_clk_init(){
	//Enable GPIO and USART2 Clock
	RCC_APB2PeriphClockCmd(RS232_GPIO_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(RS232_UART_RCC, ENABLE);
}
static void rs232_gpio_init(){
	GPIO_InitTypeDef gpio_init_structure;
	//Config UART5: PA2 = TX, PA3 = RX
	gpio_init_structure.GPIO_Pin = RS232_TX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RS232_GPIO, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = RS232_RX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(RS232_GPIO, &gpio_init_structure);
}
static void rs232_module_init(){
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
	USART_Init(RS232_UART, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(RS232_UART, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(RS232_UART, ENABLE);
}
static void usart_nvic_init(void)
{
	NVIC_InitTypeDef nvic_init_structure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	nvic_init_structure.NVIC_IRQChannel = USART2_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_Init(&nvic_init_structure);
}
void rs232_init(){
#if _USE_RS232
	rs232_clk_init();
	rs232_gpio_init();
	rs232_module_init();
	usart_nvic_init();
#endif
}
void rs232_send_string(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(RS232_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(RS232_UART,*str);
		str++;
	}
}
void rs232_rx_callback()
{
	unsigned char c;
	c = USART_ReceiveData(RS232_UART);
	if (c=='R') {
		rs232_rx_start_flag = true;
		rs232_rx_index = 0;
	}
	if (c=='#')	{
		rs232_rx_stop_flag = true;
		rs232_rx_start_flag = false;
		rs232_rx_parse(rs232_rx_buffer, &rs232_rx_frame);
		rs232_tx_handler(&rs232_rx_frame);
	}
	if	(rs232_rx_index>=RS232_BUFFER_SIZE) rs232_rx_index = 0;
	else if (rs232_rx_start_flag){
		rs232_rx_buffer[rs232_rx_index++]=c;
	}
}
void rs232_rx_parse(uint8_t *buffer,rx_frame_t *rs232_frame)
{
	char buf[10]={0};
	memcpy(rs232_frame->cmd,buffer,4);
	memcpy(buf, &buffer[4],2);
	if (strlen(buf)>0) rs232_frame->data_type = atoi(buf);
	memcpy(buf,&buffer[6],2);
	if (strlen(buf)>0) rs232_frame->date.day = atoi(buf);
	memcpy(buf,&buffer[8],2);
	if (strlen(buf)>0) rs232_frame->date.month = atoi(buf);
	memcpy(buf,&buffer[10],2);
	if (strlen(buf)>0) rs232_frame->date.year = atoi(buf);
}
static void tx_one_packet(uint8_t data_type, int data_len, char *content){
	char buff[256]={0};
	int i;
	uint8_t check_sum;
	if (data_len!=0)sprintf(buff,"$GSHT,%d,%d,<%s>,",data_type,data_len,content);
	else sprintf(buff,"$GSHT,%d,%d,,",data_type,data_len);
	check_sum = calc_checksum((uint8_t*)buff, strlen(buff));
	i = strlen(buff);
	sprintf(&buff[i],"%d#",check_sum);
	rs232_send_string(buff);
	debug_send_string(buff);
}
void rs232_tx_handler(rx_frame_t *rs232_frame){
	char buff[30]={0};
	int i=0;
	if (rs232_frame->data_type == 1) {
		tx_one_packet(rs232_frame->data_type, strlen("1,DHBKHN"), "1,DHBKHN");
		tx_one_packet(rs232_frame->data_type, strlen("2,Giam sat may cong trinh"), "2,Giam sat may cong trinh");
		tx_one_packet(rs232_frame->data_type, strlen("3,12345678969"), "3,12345678969");
		tx_one_packet(rs232_frame->data_type, strlen("4,29N1234"), "4,29N1234");
		tx_one_packet(rs232_frame->data_type, strlen("5,0"), "5,0");
		tx_one_packet(rs232_frame->data_type, strlen("7,60"), "7,60");
		tx_one_packet(rs232_frame->data_type, strlen("8,2021/05/27"), "8,2021/05/27");
		tx_one_packet(rs232_frame->data_type, strlen("9,2021/05/27"), "9,2021/05/27");
		if (sim800.power_state== OFF || sim800.sim_err == NO_PWR)
			tx_one_packet(rs232_frame->data_type, strlen("10,4"), "10,4");
		else if (sim800.signal_condition == NOSIGNAL)
			tx_one_packet(rs232_frame->data_type, strlen("10,0"), "10,0");
		else if (sim800.signal_condition!=NOSIGNAL && sim800.mqttServer.connect == false)
			tx_one_packet(rs232_frame->data_type, strlen("10,2"), "10,2");
		else if (sim800.signal_condition!=NOSIGNAL && sim800.mqttServer.connect == true)
			tx_one_packet(rs232_frame->data_type, strlen("10,3"), "10,3");
		if (gps_l70.gps_err==GPS_NO_PWR)
			tx_one_packet(rs232_frame->data_type, strlen("11,2"), "11,2");
		else if (gps_l70.RMC.Data_Valid=='V')
			tx_one_packet(rs232_frame->data_type, strlen("11,0"), "11,0");
		else if (gps_l70.RMC.Data_Valid=='A')
			tx_one_packet(rs232_frame->data_type, strlen("11,1"), "11,1");
		sprintf(buff,"18,%d/",Time.year+1900);
		i=strlen(buff);
		if (Time.mon<10) sprintf(&buff[i],"0%d/",Time.mon);
		else sprintf(&buff[i],"%d/",Time.mon);
		i=strlen(buff);
		if (Time.day<10) sprintf(&buff[i],"0%d ",Time.day);
		else sprintf(&buff[i],"%d ",Time.day);
		i=strlen(buff);
		if (Time.hour<10) sprintf(&buff[i],"0%d:",Time.hour);
		else sprintf(&buff[i],"%d:",Time.hour);
		i=strlen(buff);
		if (Time.minute<10) sprintf(&buff[i],"0%d:",Time.minute);
		else sprintf(&buff[i],"%d:",Time.minute);
		i=strlen(buff);
		if (Time.second<10) sprintf(&buff[i],"0%d",Time.second);
		else sprintf(&buff[i],"%d",Time.second);
		tx_one_packet(rs232_frame->data_type, strlen(buff), buff);
	}

}
uint8_t calc_checksum(uint8_t *buffer, int data_len){
	int i=0;
	int sum=0;
	for (i=0;i<data_len;i++){
		sum = sum+buffer[i];
	}
	return (uint8_t)sum&0xff;
}
