/*
 * sim800.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef SIMCOM_SIM800_H_
#define SIMCOM_SIM800_H_
#include <main.h>
#include <string.h>
#define _DEBUG_AT_CMD 1
#define _DEBUG_SIM_UART5	1
#define _DEBUG_AT_UART5	0
void sim_gpio_init();
uint8_t sim_power_on();
void sim_power_off();
uint8_t sim_init();
uint8_t sim_connect_server(const char* IP_adr, const char* Port);
uint8_t sim_check_connection_state(char* IP, char* Port);
uint8_t sim_disconnect_server(char* IP, char* Port);
#endif /* SIMCOM_SIM800_H_ */
