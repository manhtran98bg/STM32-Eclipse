/*
 * USART.c
 *
 *  Created on: Jul 23, 2020
 *      Author: manht
 *      USART1 Connect to SIM800 on PB7 - TX , PB6 - RX
 *
 */
#include "usart.h"

volatile char buffer1[100],buffer2[100];
volatile unsigned char y1=0,y2=0;

static void usart_clk_init(void);
static void usart_gpio_init(void);
static void usart_module_init(void);
static void usart_nvic_init(void);

static void usart_clk_init(void)
{
	RCC_APB2PeriphClockCmd(USART1_DEV_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(USART2_DEV_CLK, ENABLE);
	RCC_AHBPeriphClockCmd(USART1_PORT_CLK, ENABLE);
	RCC_AHBPeriphClockCmd(USART2_PORT_CLK, ENABLE);
}
static void usart_gpio_init(void)
{
	GPIO_InitTypeDef gpio_init_structure;
	/*Init usart1 gpio PB7 - RX, PB6 - TX, AF0 */
	GPIO_PinAFConfig(USART1_PORT, USART1_TX_PINSOURCE, USART1_AF);
	GPIO_PinAFConfig(USART1_PORT, USART1_RX_PINSOURCE, USART1_AF);
	gpio_init_structure.GPIO_Pin = USART1_TX_PIN | USART1_RX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;
	gpio_init_structure.GPIO_OType = GPIO_OType_PP;
	gpio_init_structure.GPIO_PuPd = GPIO_PuPd_UP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART1_PORT, &gpio_init_structure);
	/*Init usart2 gpio PA2 - TX, PA3 - RX, AF1 */
//	GPIO_PinAFConfig(USART2_PORT, USART2_TX_PINSOURCE, USART2_AF);
//	GPIO_PinAFConfig(USART2_PORT, USART2_RX_PINSOURCE, USART2_AF);
//	gpio_init_structure.GPIO_Pin = USART2_TX_PIN | USART2_RX_PIN;
//	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;
//	gpio_init_structure.GPIO_OType = GPIO_OType_PP;
//	gpio_init_structure.GPIO_PuPd = GPIO_PuPd_UP;
//	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(USART2_PORT, &gpio_init_structure);
}
static void usart_module_init(void)
{
	USART_InitTypeDef usart_init_structure;
	/* Baud rate 9600, 8-bit data, One stop bit
	 * No parity, Do both Rx and Tx, No HW flow control
	 */
	usart_init_structure.USART_BaudRate = 115200;
	usart_init_structure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	usart_init_structure.USART_WordLength = USART_WordLength_8b;
	usart_init_structure.USART_StopBits = USART_StopBits_1;
	usart_init_structure.USART_Parity = USART_Parity_No;
	usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* Init usart1 + usart2 */
	USART_Init(USART1, &usart_init_structure);
//	USART_Init(USART2, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(USART1, ENABLE);
//	USART_Cmd(USART2, ENABLE);
}
static void usart_nvic_init(void)
{
	NVIC_InitTypeDef nvic_init_structure;
	nvic_init_structure.NVIC_IRQChannel = USART1_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPriority = 2;
	NVIC_Init(&nvic_init_structure);
	nvic_init_structure.NVIC_IRQChannel = USART2_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPriority = 2;
	NVIC_Init(&nvic_init_structure);
}
void USART_Config()
{
	usart_clk_init();
	usart_gpio_init();
	usart_module_init();
	usart_nvic_init();
}
void USART1_Send_String(uint8_t *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
		USART_SendData(USART1,*str);
		str++;
	}
}
void USART2_Send_String(uint8_t *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
		USART_SendData(USART2,*str);
		str++;
	}
}
void USART1_Send_Char(unsigned char chr)
{
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	USART_SendData(USART1,(uint16_t)chr);
}

void USART2_Send_Char(unsigned char chr)
{
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
	USART_SendData(USART2,(uint16_t)chr);
}
void USART1_clear_buf()
{
	unsigned char u;
	for (u=0;u<100;u++) buffer1[u] = 0;
	y1=0;
}
void USART2_clear_buf()
{
	unsigned char u;
	for (u=0;u<100;u++) buffer2[u] = 0;
	y2=0;
}

