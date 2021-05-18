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

extern char RxBuffer5[];
extern uint8_t RxCounter5;

void usart_init();
void UART5_Send_String(char *str);
void UART5_Send_Array(unsigned char *str, uint8_t length);
void USART_clear_buf(uint8_t USARTx);
#endif /* USART_USART_H_ */
