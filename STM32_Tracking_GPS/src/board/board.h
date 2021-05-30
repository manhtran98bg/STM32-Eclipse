/*
 * board.h
 *
 *  Created on: May 11, 2021
 *      Author: manht
 */

#ifndef BOARD_BOARD_H_
#define BOARD_BOARD_H_
#include "../main.h"
enum freq{
	T_DEVICE_TEMP = 0,
	T_ENVI_TEMP,
	T_ENVI_HUM,
	T_DEVHICE_LOCATION,
	T_BAT_VOL,
	T_VEHICLE_RPM
};
//typedef enum{
//	ERROR,
//	OK,
//}state_t;
//typedef struct{
//	char name[10];
//	bool	available;
//	state_t	state;
//}periph_t;
//
//typedef struct {
//	periph_t	periph[5];
//	bool	board_state;
//}board_t;
//extern board_t board;
void clk_init();
void tim5_init();
void tim4_init();
void user_led_toggle();
void user_led_init();
void btn_init();
#endif /* BOARD_BOARD_H_ */
