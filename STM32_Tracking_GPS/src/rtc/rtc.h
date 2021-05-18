/*
 * rtc.h
 *
 *  Created on: May 1, 2021
 *      Author: manht
 */

#ifndef RTC_RTC_H_
#define RTC_RTC_H_
#include "main.h"

typedef struct{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t old_minute;
}RTC_Time_t;
extern RTC_Time_t Time;
extern char time_buffer[];
extern struct tm time_struct;
void RTC_Init();
void RTC_NVIC_Config();
void RTC_ClockConfig();
void RTC_WriteTime(uint32_t time_stamp);
void RTC_GetTime(RTC_Time_t *Time);
uint32_t set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second);
struct tm convert_time_stamp(uint32_t t);
#endif /* RTC_RTC_H_ */
