/*
 * main.c
 *
 *  Created on: Aug 12, 2020
 *      Author: manhtran
 *     	Firmware IOC.
 */
#include "main.h"
#include "board.h"
#include "backup.h"
#include "../src/ioc/ioc.h"
#include "../src/ioc/actuator_io_hardware.h"
#include "../src/can/can_receive.h"
#define TEST_IO 0
#define RESET_BACKUP_DATA 0
void update_io(void);
void test_io(void);
ioc_t vehicle_ioc;
uint32_t distance;
float distance_tmp1;
extern void display_odo(uint32_t distance);
extern void display_trip(uint32_t distance);
void app_init()
{
	board_init();
	ioc_init(&vehicle_ioc);
	rcc_backup_init();
#if RESET_BACKUP_DATA
	write_backup_data(0);
	delay_ms(1000);
#endif
	distance = get_backup_data();
	display_trip((uint32_t)distance_tmp1);
	display_odo(distance);
}
int main(void)
{
	app_init();
	while (1)
	{
		can_send_data();
		delay_ms(1000);
		update_io();
	}
	return 0;
}

void update_io(void)
{
	ioc_update_device(&vehicle_ioc);
}
void test_io(void)
{
#if TEST_IO
	GPIO_SetBits(FAR_LIGHT_CTRL_PORT, FAR_LIGHT_CTRL_PIN);
	GPIO_SetBits(COS_LIGHT_CTRL_PORT, COS_LIGHT_CTRL_PIN);
	GPIO_SetBits(HORN_CTRL_PORT, HORN_CTRL_PIN);
	GPIO_SetBits(TURN_L_LIGHT_CTRL_PORT, TURN_L_LIGHT_CTRL_PIN);
	GPIO_SetBits(TURN_R_LIGHT_CTRL_PORT, TURN_R_LIGHT_CTRL_PIN);
	GPIO_SetBits(BACK_LIGHT_CTRL_PORT, BACK_LIGHT_CTRL_PIN);
	GPIO_SetBits(BRAKE_LIGHT_CTRL_PORT, BRAKE_LIGHT_CTRL_PIN);
	GPIO_SetBits(ECO_MODE_CTRL_PORT, ECO_MODE_CTRL_PIN);
	GPIO_SetBits(SPORT_MODE_CTRL_PORT, SPORT_MODE_CTRL_PIN);
	GPIO_SetBits(LED_CTRL1_PORT, LED_CTRL1_PIN);
	GPIO_SetBits(LED_CTRL2_PORT, LED_CTRL2_PIN);
	delay_ms(100);
	GPIO_ResetBits(FAR_LIGHT_CTRL_PORT, FAR_LIGHT_CTRL_PIN);
	GPIO_ResetBits(COS_LIGHT_CTRL_PORT, COS_LIGHT_CTRL_PIN);
	GPIO_ResetBits(HORN_CTRL_PORT, HORN_CTRL_PIN);
	GPIO_ResetBits(TURN_L_LIGHT_CTRL_PORT, TURN_L_LIGHT_CTRL_PIN);
	GPIO_ResetBits(TURN_R_LIGHT_CTRL_PORT, TURN_R_LIGHT_CTRL_PIN);
	GPIO_ResetBits(BACK_LIGHT_CTRL_PORT, BACK_LIGHT_CTRL_PIN);
	GPIO_ResetBits(BRAKE_LIGHT_CTRL_PORT, BRAKE_LIGHT_CTRL_PIN);
	GPIO_ResetBits(ECO_MODE_CTRL_PORT, ECO_MODE_CTRL_PIN);
	GPIO_ResetBits(SPORT_MODE_CTRL_PORT, SPORT_MODE_CTRL_PIN);
	GPIO_ResetBits(LED_CTRL1_PORT, LED_CTRL1_PIN);
	GPIO_ResetBits(LED_CTRL2_PORT, LED_CTRL2_PIN);
	delay_ms(100);
#endif
}





