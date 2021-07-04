/*
 * hc05.h
 *
 *  Created on: Jun 15, 2021
 *      Author: manht
 */

#ifndef HC05_HC05_H_
#define HC05_HC05_H_
#include "../main.h"
#define HC05_UART	USART1
#define HC05_UART_RCC	RCC_APB2Periph_USART1

#define HC05_TX_PIN	GPIO_Pin_9
#define HC05_RX_PIN	GPIO_Pin_10
#define HC05_GPIO_RCC	RCC_APB2Periph_GPIOA
#define HC05_GPIO	GPIOA

#define HC05_EN_PIN		GPIO_Pin_11
#define HC05_STATE_PIN	GPIO_Pin_8

#define	HC05_BUFFER_SIZE	128
typedef struct{
	char ID[6];
	int temp;
	int range;
	int dust;
}data_t;
extern data_t hc05_data;
extern char hc05_rx_buffer[];
extern uint8_t	hc05_rx_index;
extern bool rx_flag;
bool hc05_parse_data(char *rx_buffer, int *dust);
void hc05_clear_buffer();
#endif /* HC05_HC05_H_ */
