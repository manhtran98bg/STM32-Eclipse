/*
 * gps.h
 *
 *  Created on: Apr 12, 2021
 *      Author: manht
 */

#ifndef GPS_GPS_H_
#define GPS_GPS_H_

#include "main.h"
#include "../power/power.h"
#include "../usart/usart.h"
#define _DEBUG_GPS_UART5 1

#define GPS_RST_CLK	RCC_APB2Periph_GPIOC
#define GPS_RST_PORT	GPIOC
#define GPS_RST_PIN		GPIO_Pin_5

void gps_power_on();
void gps_power_off();
void gps_reset();
uint8_t gps_read_data();
void gps_init();
#endif /* GPS_GPS_H_ */
