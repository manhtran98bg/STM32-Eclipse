/*
 * board.h
 *
 *  Created on: 17 Jun 2021
 *      Author: kylin
 */

#ifndef BOARD_H_
#define BOARD_H_
#include "main.h"
#define USER_LED_PIN	GPIO_Pin_9

extern void rcc_config();
extern void user_led_config();
extern void user_led_toggle();
extern void tim4_init();
#endif /* BOARD_H_ */
