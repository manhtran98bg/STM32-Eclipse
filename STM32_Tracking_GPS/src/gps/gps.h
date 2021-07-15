/*
 * gps.h
 *
 *  Created on: Apr 12, 2021
 *      Author: manht
 */

#ifndef GPS_GPS_H_
#define GPS_GPS_H_

#include "../main.h"
#include "../power/power.h"
#include "../usart/usart.h"
#include "../simcom/sim800.h"
#include "../sdcard/ff.h"
#include "../rtc/rtc.h"
#include "../sdcard/sdmm.h"
#include "../func.h"
/*----------------------------------------- Define UART4 ----------------------------------*/
#define GPS_RST_CLK		RCC_APB2Periph_GPIOC
#define GPS_RST_PORT	GPIOC
#define GPS_RST_PIN		GPIO_Pin_5

/* UART4 For GPS L70R*/
#define GPS_UART			UART4
#define GPS_UART_CLK		RCC_APB1Periph_UART4
#define GPS_UART_GPIO		GPIOC
#define GPS_UART_GPIO_CLK	RCC_APB2Periph_GPIOC
#define GPS_UART_GPIO_TX	GPIO_Pin_10
#define GPS_UART_GPIO_RX	GPIO_Pin_11
#define GPS_BUFFER_SIZE		256

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
    char lat_dec_degree[16];
}Latitude;

typedef struct{
    int lon_ddd;
    uint8_t	lon_mm;
    int	lon_mmmm;
    char lon_dec_degree[16];
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
typedef enum {
	GPS_INITING=0,
	GPS_INITED,
}gps_state_t;

typedef struct {
	gps_error_t gps_err;
	bool gps_pwr_state;
	RMC_Data RMC;
	uint32_t gps_baudrate;
	bool gps_response;
	gps_state_t gps_state;
}gps_t;
typedef struct{
    char *data;
    int datalen;
}strArray;
extern gps_t gps_l70;
extern uint8_t flagStart,flagStop;
extern char json_geowithtime[];
extern char json_geovelowithtime[];
extern char gps_buffer[];
extern uint16_t gps_buffer_index;

void gps_power_on();
void gps_power_off();
void gps_reset();
uint8_t  gps_read_data(gps_t *gps);
void gps_init(gps_t *gps);
void gps_check_current_baud(gps_t *gps);
void gps_set_baudrate(uint32_t baud);
void gps_RxCallback(gps_t *gps);
bool RMC_Parse(RMC_Data *RMC, char *RMC_Sentence, int RMC_len);
void RMC_json_init(RMC_Data *RMC, char *buffer, bool option);
void gps_uart_send_string(char *str);
void gps_uart_clear_buffer();
void gps_uart_send_char( char chr);
void gps_uart_send_array(unsigned char *str, uint8_t length);
#endif /* GPS_GPS_H_ */
