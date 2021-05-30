/*
 * dht11.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef SENSOR_DHT11_H_
#define SENSOR_DHT11_H_
#include "../main.h"
#include "../service/delay.h"

typedef struct dht11_data{
	u8 RH_Byte1;
	u8 RH_Byte2;
	u8 Temp_Byte1;
	u8 Temp_Byte2;
	u8 Check_sum;
}dht11_data;
u8 dht11_read_data(dht11_data *dat);
#endif /* SENSOR_DHT11_H_ */
