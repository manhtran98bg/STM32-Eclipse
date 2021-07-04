/*
 * board.h
 *
 *  Created on: Jun 15, 2021
 *      Author: manht
 */
#include "main.h"
#ifndef BOARD_H_
#define BOARD_H_

#define USER_LED_PIN	GPIO_Pin_13

#define RELAY_PIN		GPIO_Pin_4
#define BUZZER_PIN		GPIO_Pin_5

#define BTN1_PIN		GPIO_Pin_13
#define BTN2_PIN		GPIO_Pin_14
#define BTN3_PIN		GPIO_Pin_15

#define BTN_GPIO		GPIOB


void clock_config();
void user_led_init();
void user_led_toogle();
void output_pin_init();
void buzzer_on();
void buzzer_off();
void relay_on();
void relay_off();
void btn_config();
void buzzer_beep(uint32_t duration);
#endif /* BOARD_H_ */
