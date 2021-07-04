/*
 * sr04.h
 *
 *  Created on: Jun 26, 2021
 *      Author: manht
 */

#ifndef SR04_SR04_H_
#define SR04_SR04_H_
#include "main.h"
#include "../service/delay.h"
#define sr04_gpio	GPIOA
#define sr04_gpio_clk	RCC_APB2Periph_GPIOA

#define sr04_echo	GPIO_Pin_5
#define sr04_trig	GPIO_Pin_6

#define sr04_trig_set  (GPIO_SetBits(sr04_gpio, sr04_trig))
#define sr04_trig_reset  (GPIO_ResetBits(sr04_gpio, sr04_trig))

extern __IO uint16_t duration;
extern __IO bool sr04_flag;
extern uint8_t range;
void sr04_init(void);
void start_tim4(void);
void stop_tim4(void);
uint8_t sr04_get_distance();
#endif /* SR04_SR04_H_ */
