/*
 * usart.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef USART_USART_H_
#define USART_USART_H_
#include "main.h"
#define _USE_DMA	0
#define BUFFER_SIZE1	128
#define BUFFER_SIZE4	128
#define BUFFER_SIZE5	128


/*----------------------------------------- Define UART5 ----------------------------------*/
/* UART5 For Debug*/
#define UART5_CLK	RCC_APB1Periph_UART5
#define UART5_TX_PIN	GPIO_Pin_12
#define UART5_TX_PORT	GPIOC
#define UART5_TX_CLK	RCC_APB2Periph_GPIOC

#define UART5_RX_PIN	GPIO_Pin_2
#define UART5_RX_PORT	GPIOD
#define UART5_RX_CLK	RCC_APB2Periph_GPIOD
/*----------------------------------------- Define USART1 ---------------------------------*/
/* USART1 For SIM800C*/
#define USART1_CLK	RCC_APB2Periph_USART1
#define USART1_TX_RX_PORT	GPIOA
#define USART1_TX_RX_CLK	RCC_APB2Periph_GPIOA
#define USART1_TX_PIN	GPIO_Pin_9
#define USART1_RX_PIN	GPIO_Pin_10
/*----------------------------------------- Define UART4 ----------------------------------*/
/* UART4 For GPS L70R*/
#define UART4_CLK	RCC_APB1Periph_UART4
#define UART4_TX_RX_PORT	GPIOC
#define UART4_TX_RX_CLK	RCC_APB2Periph_GPIOC
#define UART4_TX_PIN	GPIO_Pin_10
#define UART4_RX_PIN	GPIO_Pin_11

void usart_init();
void USART1_Send_String(char *str);
void USART1_Send_Array(unsigned char *str, uint8_t length);
void UART5_Send_String(char *str);
void UART5_Send_Array(unsigned char *str, uint8_t length);
void UART4_Send_String(char *str);
void USART_clear_buf(uint8_t USARTx);
#endif /* USART_USART_H_ */
