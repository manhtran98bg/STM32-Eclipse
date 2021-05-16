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
	int int_part;
	long dec_part;
}dec_degree;
typedef struct{
    uint8_t	lat_dd;
    uint8_t	lat_mm;
    int	lat_mmmm;
    dec_degree lat_dec_degree;
}Latitude;

typedef struct{
    int lon_ddd;
    uint8_t	lon_mm;
    int	lon_mmmm;
    dec_degree lon_dec_degree;
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
typedef enum {
	GPS_NO_PWR = 0,
	GPS_NO_RES,
	GPS_NO_ERR,
}gps_error_t;
typedef struct {
	gps_error_t gps_err;
	bool gps_pwr_state;
	RMC_Data RMC;
}gps_t;
typedef struct{
    char *data;
    int datalen;
}strArray;
extern gps_t *gps_l70;
void gps_power_on();
void gps_power_off();
void gps_reset();
uint8_t  gps_read_data(gps_t *gps);
void gps_init();
bool RMC_Parse(RMC_Data *RMC, char *RMC_Sentence, int RMC_len);
void RMC_json_init(RMC_Data *RMC, char *buffer);
#endif /* GPS_GPS_H_ */
