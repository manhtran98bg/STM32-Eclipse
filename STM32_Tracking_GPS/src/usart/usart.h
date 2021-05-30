/*
 * usart.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef USART_USART_H_
#define USART_USART_H_
#include "../main.h"

#define DEBUG_BUFFER_SIZE	128
#define _USE_DEBUG_UART		1

/*----------------------------------------- Define UART DEBUG ----------------------------------*/
/* UART5 For Debug*/
#define DEBUG_UART				UART5
#define DEBUG_UART_RCC			RCC_APB1Periph_UART5
#define DEBUG_UART_GPIO_RCC		RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD

#define DEBUG_UART_TX			GPIO_Pin_12
#define DEBUG_UART_GPIO_TX		GPIOC
#define DEBUG_UART_RX			GPIO_Pin_2
#define DEBUG_UART_GPIO_RX		GPIOD

extern char debug_rx_buffer[];
extern uint8_t debug_rx_buffer_index;

void usart_init();
void debug_send_chr(unsigned char chr);
void debug_send_string(char *str);
void debug_send_array(unsigned char *str, uint8_t length);
void debug_clear_buffer();
#endif /* USART_USART_H_ */
