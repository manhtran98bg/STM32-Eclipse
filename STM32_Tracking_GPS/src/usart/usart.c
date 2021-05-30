/*
 * usart.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "usart.h"


char debug_rx_buffer[DEBUG_BUFFER_SIZE];
uint8_t debug_rx_buffer_index=0;

static void usart_clk_init(void);
static void usart_gpio_init(void);
static void usart_module_init(void);
static void usart_nvic_init(void);

static void usart_clk_init(void)
{
	/*Enable UART clock and GPIO clock*/
	RCC_APB1PeriphClockCmd(DEBUG_UART_RCC, ENABLE);
	RCC_APB2PeriphClockCmd(DEBUG_UART_GPIO_RCC, ENABLE);
}
static void usart_gpio_init(void)
{
	GPIO_InitTypeDef gpio_init_structure;
	//Config UART5: PC12 = TX, PD2 = RX
	gpio_init_structure.GPIO_Pin = DEBUG_UART_TX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_UART_GPIO_TX, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = DEBUG_UART_RX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(DEBUG_UART_GPIO_RX, &gpio_init_structure);

}
static void usart_module_init(void)
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
	USART_Init(DEBUG_UART, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(DEBUG_UART, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(DEBUG_UART, ENABLE);
}
static void usart_nvic_init(void)
{
	NVIC_InitTypeDef nvic_init_structure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	nvic_init_structure.NVIC_IRQChannel = UART5_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_Init(&nvic_init_structure);
}
void usart_init()
{
#if _USE_DEBUG_UART
	usart_clk_init();
	usart_gpio_init();
	usart_module_init();
	usart_nvic_init();
#endif
}
void debug_send_chr(unsigned char chr)
{
	while(USART_GetFlagStatus(DEBUG_UART,USART_FLAG_TXE) == RESET);
	USART_SendData(DEBUG_UART,chr);
}
void debug_send_string(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(DEBUG_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_UART,*str);
		str++;
	}
}
void debug_send_array(unsigned char *str, uint8_t length)
{
	for(int i=0;i<length;i++)
	{
		while(USART_GetFlagStatus(DEBUG_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(DEBUG_UART,*(str+i));
	}
}
void debug_clear_buffer()
{
	for (int i=0;i<DEBUG_BUFFER_SIZE;i++) debug_rx_buffer[i]=0;
	debug_rx_buffer_index=0;
}


