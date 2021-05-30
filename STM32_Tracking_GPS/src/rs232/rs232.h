/*
 * rs232.h
 *
 *  Created on: May 23, 2021
 *      Author: manht
 */

#ifndef RS232_RS232_H_
#define RS232_RS232_H_
#include "../main.h"
#include "../gps/gps.h"
#define RS232_BUFFER_SIZE	256
#define _USE_RS232				1

/*----------------------------------------- Define UART RS232 ----------------------------------*/
/* USART2 For RS232*/
#define RS232_UART				USART2
#define RS232_UART_RCC			RCC_APB1Periph_USART2
#define RS232_GPIO_RCC			RCC_APB2Periph_GPIOA
#define RS232_GPIO				GPIOA

#define RS232_TX			GPIO_Pin_2
#define RS232_RX			GPIO_Pin_3

typedef struct {
	char cmd[5];
	uint8_t	data_type;
	UTC_Date date;
}rx_frame_t;

typedef struct{
	const char cmd[6];
	uint8_t	data_type;
	int data_len;
	uint8_t check_sum;
}tx_frame_t;
void rs232_init();
void rs232_send_string(char *str);
void rs232_rx_parse(uint8_t *buffer,rx_frame_t *rs232_frame);
void rs232_tx_handler(rx_frame_t *rs232_frame);
uint8_t calc_checksum(uint8_t *buffer, int data_len);
#endif /* RS232_RS232_H_ */
