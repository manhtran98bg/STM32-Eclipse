/*
 * usart.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "usart.h"

__IO char RxBuffer1[BUFFER_SIZE1];
__IO uint8_t RxCounter1=0;

__IO char RxBuffer5[BUFFER_SIZE5];
__IO uint8_t RxCounter5=0;

__IO char RxBuffer4[BUFFER_SIZE4];
__IO uint8_t RxCounter4=0;
uint8_t RxBuffer[1024];

static void usart_clk_init(void);
static void usart_gpio_init(void);
static void usart_module_init(void);
static void usart_nvic_init(void);

static void usart_clk_init(void)
{
	/*Enable UART clock and GPIO clock*/
	RCC_APB1PeriphClockCmd(UART5_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(UART5_TX_CLK|UART5_RX_CLK, ENABLE);

	RCC_APB2PeriphClockCmd(USART1_TX_RX_CLK|USART1_CLK, ENABLE);

	RCC_APB1PeriphClockCmd(UART4_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(UART4_TX_RX_CLK, ENABLE);
}
static void usart_gpio_init(void)
{
	GPIO_InitTypeDef gpio_init_structure;
	//Config UART5: PC12 = TX, PD2 = RX
	gpio_init_structure.GPIO_Pin = UART5_TX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART5_TX_PORT, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = UART5_RX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART5_RX_PORT, &gpio_init_structure);
	//Config USART1: PA9 = TX, PA10 = RX
	gpio_init_structure.GPIO_Pin = USART1_TX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART1_TX_RX_PORT, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = USART1_RX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(USART1_TX_RX_PORT, &gpio_init_structure);
	//Config UART4: PC10 = TX, PC11 = RX
	gpio_init_structure.GPIO_Pin = UART4_TX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART4_TX_RX_PORT, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = UART4_RX_PIN;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART4_TX_RX_PORT, &gpio_init_structure);
}
static void usart_module_init(void)
{
	USART_InitTypeDef usart_init_structure;
	/* Baud rate 9600, 8-bit data, One stop bit
	* No parity, Do both Rx and Tx, No HW flow control
	*/
	usart_init_structure.USART_BaudRate = 9600;
	usart_init_structure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init_structure.USART_Parity = USART_Parity_No;
	usart_init_structure.USART_StopBits = USART_StopBits_1;
	usart_init_structure.USART_WordLength = USART_WordLength_8b;
	USART_Init(UART5, &usart_init_structure);
	USART_Init(USART1, &usart_init_structure);
	USART_Init(UART4, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#if _USE_DMA
	/* UART4 with DMA2 Channel 3 */
	USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
#else
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
#endif
	/*Enable USART	 */
	USART_Cmd(UART5, ENABLE);
	USART_Cmd(USART1, ENABLE);
	USART_Cmd(UART4, ENABLE);
}
static void usart_nvic_init(void)
{
	NVIC_InitTypeDef nvic_init_structure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	nvic_init_structure.NVIC_IRQChannel = USART1_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&nvic_init_structure);
#if !_USE_DMA
	nvic_init_structure.NVIC_IRQChannel = UART4_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_Init(&nvic_init_structure);
#endif

	nvic_init_structure.NVIC_IRQChannel = UART5_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_Init(&nvic_init_structure);
}
static void uart4_dma_init()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	DMA_DeInit(DMA2_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_BufferSize = 1024;

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);
	DMA_Cmd(DMA2_Channel3, ENABLE);
}
void usart_init()
{
	usart_clk_init();
	usart_gpio_init();
	usart_module_init();
#if _USE_DMA
	uart4_dma_init();
#endif
	usart_nvic_init();
}
void USART1_Send_String(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
		USART_SendData(USART1,*str);
		str++;
	}
}
void USART1_Send_Array(unsigned char *str, uint8_t length)
{
	for(int i=0;i<=length;i++)
	{
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
		USART_SendData(USART1,*(str+i));
	}
}
void UART5_Send_String(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(UART5,USART_FLAG_TXE) == RESET);
		USART_SendData(UART5,*str);
		str++;
	}
}
void UART5_Send_Array(unsigned char *str, uint8_t length)
{
	for(int i=0;i<=length;i++)
	{
		while(USART_GetFlagStatus(UART5,USART_FLAG_TXE) == RESET);
		USART_SendData(UART5,*(str+i));
	}
}
void UART4_Send_String(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(UART4,USART_FLAG_TXE) == RESET);
		USART_SendData(UART4,*str);
		str++;
	}
}
void USART_clear_buf(uint8_t USARTx)
{
	switch (USARTx)
	{
	case 1:
		for (int i=0;i<BUFFER_SIZE1;i++) RxBuffer1[i]=0;
		RxCounter1=0;
		break;
	case 4:
		for (int i=0;i<BUFFER_SIZE4;i++) RxBuffer4[i]=0;
		RxCounter4=0;
		break;
	case 5:
		for (int i=0;i<BUFFER_SIZE5;i++) RxBuffer5[i]=0;
		RxCounter5=0;
		break;
	default:
		break;
	}
}


