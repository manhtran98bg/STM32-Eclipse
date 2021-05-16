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
void RTC_Init();
void RTC_NVIC_Config();
void RTC_ClockConfig();
void RTC_WriteTime(uint8_t hh, uint8_t mm, uint8_t ss);
void RTC_GetTime(RTC_Time_t *Time);


#endif /* RTC_RTC_H_ */
