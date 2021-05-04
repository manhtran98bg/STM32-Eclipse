/*
 * gps.h
 *
 *  Created on: Apr 12, 2021
 *      Author: manht
 */

#ifndef GPS_GPS_H_
#define GPS_GPS_H_

#include "main.h"
#include "../power/power.h"
#include "../usart/usart.h"
#define _DEBUG_GPS_UART5 1

#define GPS_RST_CLK	RCC_APB2Periph_GPIOC
#define GPS_RST_PORT	GPIOC
#define GPS_RST_PIN		GPIO_Pin_5

typedef struct{
    uint8_t hh;
    uint8_t mm;
    uint8_t ss;
    int sss;
}UTC_Time;

typedef struct{
    uint8_t	day;
    uint8_t	month;
    uint8_t year;
}UTC_Date;

typedef struct{
    uint8_t	lat_dd;
    uint8_t	lat_mm;
    int	lat_mmmm;
}Latitude;

typedef struct{
    int lon_ddd;
    uint8_t	lon_mm;
    int	lon_mmmm;
}Longitude;

typedef struct{
    char ID[7];
    UTC_Time Time;
    char Data_Valid[2];
    Latitude Lat;
    char Lat_dir[2];
    Longitude Lon;
    char Lon_dir[2];
    double Speed;
    char COG[5];
    UTC_Date Date;
}RMC_Data;

typedef struct{
    char *data;
    int datalen;
}strArray;

void gps_power_on();
void gps_power_off();
void gps_reset();
uint8_t  gps_read_data(RMC_Data *RMC);
void gps_init();
bool RMC_Parse(RMC_Data *RMC, char *RMC_Sentence, int RMC_len);
#endif /* GPS_GPS_H_ */
