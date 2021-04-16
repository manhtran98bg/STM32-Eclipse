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
#define _DEBUG_AT_UART5	1
typedef enum {
	ALREADY_CONNECT,
	NOT_CONNECT
}state;
typedef struct server{
	char *IP;
	char *Port;
	state state;
}server;

void sim_gpio_init();
uint8_t sim_power_on();
void sim_power_off();
uint8_t sim_init();
uint8_t sim_connect_server(server* myServer);
uint8_t sim_disconnect_server(server *myServer);
uint8_t sim_set_TCP_connection();
#endif /* SIMCOM_SIM800_H_ */
