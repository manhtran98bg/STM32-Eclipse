/*
 * sim800.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef SIMCOM_SIM800_H_
#define SIMCOM_SIM800_H_
#include <main.h>
void sim_gpio_init();
void sim_power_on();
void sim_power_off();
uint8_t sim_power_status();

#endif /* SIMCOM_SIM800_H_ */
