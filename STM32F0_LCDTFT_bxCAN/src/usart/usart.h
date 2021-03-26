/*
 * USART.h
 *
 *  Created on: Jul 23, 2020
 *      Author: manht
 */

#ifndef USART_H_
#define USART_H_
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_usart.h"
#define USART1_DEV_CLK	RCC_APB2Periph_USART1
#define USART1_PORT_CLK	RCC_AHBPeriph_GPIOB
#define USART1_PORT		GPIOB
#define USART1_AF	GPIO_AF_0

#define USART1_TX_PORT	GPIOB;
#define USART1_TX_PIN	GPIO_Pin_6
#define USART1_TX_PINSOURCE	GPIO_PinSource6


#define USART1_RX_PORT	GPIOB;
#define USART1_RX_PIN	GPIO_Pin_7
#define USART1_RX_PINSOURCE	GPIO_PinSource7

#define USART2_DEV_CLK	RCC_APB1Periph_USART2
#define USART2_PORT_CLK	RCC_AHBPeriph_GPIOA
#define USART2_PORT		GPIOA
#define USART2_AF	GPIO_AF_1

#define USART2_TX_PORT	GPIOA
#define USART2_TX_PIN	GPIO_Pin_2
#define USART2_TX_PINSOURCE	GPIO_PinSource2


#define USART2_RX_PORT	GPIOA;
#define USART2_RX_PIN	GPIO_Pin_3
#define USART2_RX_PINSOURCE	GPIO_PinSource3


void USART_Config();
void USART1_Send_String(uint8_t *str);
void USART1_clear_buf();
void USART1_Send_Char(unsigned char chr);
void USART2_Send_String(uint8_t *str);
void USART2_clear_buf();
void USART2_Send_Char(unsigned char chr);
#endif /* USART_H_ */
