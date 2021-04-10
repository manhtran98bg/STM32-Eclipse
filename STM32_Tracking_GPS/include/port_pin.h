/*
 * port_pin.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef PORT_PIN_H_
#define PORT_PIN_H_

#include "stm32f10x.h"

#define USER_LED	GPIO_Pin_8				//PA8

#define USER_BTN1	GPIO_Pin_12				//PB12
#define USER_BTN2	GPIO_Pin_13				//PB13
#define USER_BTN3	GPIO_Pin_14				//PB14

#define CTRL_PWR_GPS	GPIO_Pin_15			//PA15 : Pin to Power ON TLV70233, LOW = POWER ON
#define CTRL_PWR_GSM	GPIO_Pin_9			//PB9 : Pin to Power OFF TPS54360, HIGH = POWER OFF

#define NPN_SENSOR_IN	GPIO_Pin_0				//PC0: DAT Pin NPN SENSOR
#define DHT11_IN		GPIO_Pin_2				//PC2: DAT Pin DHT11
#define DS18B20_IN		GPIO_Pin_3				//PC3: DAT Pin DS18B20

#define GSM_POWERKEY	GPIO_Pin_8				//PC8: POWERKEY PIN TO POWER ON SIM800, Pull low at least 1.2s and release to power on/off
#define GSM_STATUS 		GPIO_Pin_9				//PC9: POWER ON STATUS PIN OF SIM800.

#endif /* PORT_PIN_H_ */
