/*
 * hc05.c
 *
 *  Created on: Jun 15, 2021
 *      Author: manht
 */
#include "hc05.h"
char hc05_rx_buffer[HC05_BUFFER_SIZE] = {0};
uint8_t	hc05_rx_index = 0;
bool rx_flag = 0;
data_t hc05_data;
static void hc05_gpio_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef gpio_init_structure;
	gpio_init_structure.GPIO_Pin = HC05_EN_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HC05_GPIO, &gpio_init_structure);

	gpio_init_structure.GPIO_Pin = HC05_STATE_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HC05_GPIO, &gpio_init_structure);
}
void hc05_en_set(){
	GPIO_SetBits(HC05_GPIO, HC05_EN_PIN);
}
void hc05_en_reset(){
	GPIO_ResetBits(HC05_GPIO, HC05_EN_PIN);
}
void hc05_en_toggle(){
	HC05_GPIO->ODR ^=HC05_EN_PIN;
}
void hc05_uart_init(){
	RCC_APB2PeriphClockCmd(HC05_UART_RCC|HC05_GPIO_RCC, ENABLE);
	GPIO_InitTypeDef gpio_init_structure;
	USART_InitTypeDef usart_init_structure;
	NVIC_InitTypeDef nvic_init_structure;
	gpio_init_structure.GPIO_Pin = HC05_TX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HC05_GPIO, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = HC05_RX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(HC05_GPIO, &gpio_init_structure);

	/* Baud rate 9600, 8-bit data, One stop bit
	* No parity, Do both Rx and Tx, No HW flow control
	*/
	usart_init_structure.USART_BaudRate = 9600;
	usart_init_structure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init_structure.USART_Parity = USART_Parity_No;
	usart_init_structure.USART_StopBits = USART_StopBits_1;
	usart_init_structure.USART_WordLength = USART_WordLength_8b;
	USART_Init(HC05_UART, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(HC05_UART, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(HC05_UART, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	nvic_init_structure.NVIC_IRQChannel = USART1_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_Init(&nvic_init_structure);
}
void hc05_init(){
	hc05_gpio_init();
	hc05_uart_init();
	//Enter Command Mode
//	hc05_en_set();
	hc05_en_reset();
}
void hc05_send_cmd(char *cmd){
	while(*cmd)
	{
		while(USART_GetFlagStatus(HC05_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(HC05_UART,*cmd);
		cmd++;
	}
}
void hc05_clear_buffer(){
	hc05_rx_index = 0;
	memset(hc05_rx_buffer,0,HC05_BUFFER_SIZE);
}
bool hc05_parse_data(char *rx_buffer, int buffer_len, data_t *hc05_data){
	int i=0,j=0,k=0;
	char tmp[5]={0};
	if (rx_buffer[0]!='$') return false;
	while(i<buffer_len){
		memset(tmp,0,5);
		while((i<buffer_len)&&(rx_buffer[i]!=',')){
			tmp[j++] = rx_buffer[i];
			i++;
		}
		j=0;
		i++;
		if (k==0) strcpy(hc05_data->ID,tmp);
		if (strstr(hc05_data->ID,"$DATA")==0) return false;
		if (k==1) hc05_data->temp = atoi(tmp);
		if (k==2) hc05_data->range = atoi(tmp);
		if (k==3) hc05_data->dust = atoi(tmp);
		k++;
	}
	return true;
}
