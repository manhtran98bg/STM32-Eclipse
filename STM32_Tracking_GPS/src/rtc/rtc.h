/*
 * rtc.h
 *
 *  Created on: May 1, 2021
 *      Author: manht
 */

#ifndef RTC_RTC_H_
#define RTC_RTC_H_
#include "../main.h"

typedef struct{
	uint8_t	year;
	uint8_t mon;
	uint8_t day;
	uint8_t old_day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t old_minute;
}RTC_Time_t;
extern RTC_Time_t Time;
extern char time_buffer[];
extern struct tm time_struct;
extern char time_str[];				//Chuoi luu thoi gian dinh dang hh:mm:ss;
extern int gps_data_count;
void RTC_Init();
void RTC_NVIC_Config();
void RTC_ClockConfig();
void RTC_WriteTime(uint32_t time_stamp);
void RTC_GetTime(RTC_Time_t *Time);
uint32_t set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second);
struct tm convert_time_stamp(uint32_t t);
void date_time2str(char *buffer, struct tm *time_struct);
void create_time_str(RTC_Time_t *time, char *time_str);
#endif /* RTC_RTC_H_ */
