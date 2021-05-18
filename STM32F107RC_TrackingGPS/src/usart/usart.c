/*
 * usart.c
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */
#include "usart.h"


char RxBuffer5[BUFFER_SIZE5];
uint8_t RxCounter5=0;

static void usart_clk_init(void);
static void usart_gpio_init(void);
static void usart_module_init(void);
static void usart_nvic_init(void);

static void usart_clk_init(void)
{
	/*Enable UART clock and GPIO clock*/
	RCC_APB1PeriphClockCmd(UART5_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(UART5_TX_CLK|UART5_RX_CLK, ENABLE);
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
	USART_Init(UART5, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(UART5, ENABLE);
}
static void usart_nvic_init(void)
{
	NVIC_InitTypeDef nvic_init_structure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	nvic_init_structure.NVIC_IRQChannel = UART5_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_Init(&nvic_init_structure);
#if _USE_DMA
	NVIC_EnableIRQ(DMA2_Channel3_IRQn);
#endif
}
//static void uart4_dma_init()
//{
//	DMA_InitTypeDef DMA_InitStructure;
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
//	DMA_DeInit(DMA2_Channel3);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dmaRxbuffer;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
//	DMA_InitStructure.DMA_BufferSize = 512;
//
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//	DMA_Init(DMA2_Channel3, &DMA_InitStructure);
//	DMA_Cmd(DMA2_Channel3, ENABLE);
//}
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
	for(int i=0;i<length;i++)
	{
		while(USART_GetFlagStatus(UART5,USART_FLAG_TXE) == RESET);
		USART_SendData(UART5,*(str+i));
	}
}
void USART_clear_buf(uint8_t USARTx)
{
	switch (USARTx)
	{
	case 5:
		for (int i=0;i<BUFFER_SIZE5;i++) RxBuffer5[i]=0;
		RxCounter5=0;
		break;
	default:
		break;
	}
}


