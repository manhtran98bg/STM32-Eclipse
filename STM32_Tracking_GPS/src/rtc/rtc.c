/*
 * rtc.c
 *
 *  Created on: May 1, 2021
 *      Author: manht
 */

#include "rtc.h"
const uint8_t daysInMonth [] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
char time_str[10]={0};				//Chuoi luu thoi gian dinh dang hh:mm:ss;
int gps_speed_count = 0;
int gps_location_count = 0;
void RTC_Init()
{
	RTC_NVIC_Config();
	if(BKP_ReadBackupRegister(BKP_DR1)!=0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		 * 		       the first time the program is executed) */
#if _DEBUG
		trace_puts("RTC not yet configured....");
#endif
		/* RTC Configuration */
		RTC_ClockConfig();
		RTC_SetCounter(set_time(2021, 5, 17, 17, 42, 20));
#if _DEBUG
		trace_puts("RTC configured....");
#endif
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	    /* Check if the Power On Reset flag is set */
	    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
	    {
#if _DEBUG
	    	trace_puts("Power On Reset occurred....");
#endif
	    }
	    /* Check if the Pin Reset flag is set */
	    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
	    {
#if _DEBUG
	    	trace_puts("External Reset occurred....");
#endif
	    }
#if _DEBUG
	    trace_puts("No need to configure RTC....");
#endif
	    /* Wait for RTC registers synchronization */
	    RTC_WaitForSynchro();
	    /* Enable the RTC Second */
	    RTC_ITConfig(RTC_IT_SEC, ENABLE);
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	    RCC_ClearFlag();
	}
}
void RTC_NVIC_Config()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void RTC_ClockConfig()
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP, ENABLE);
	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
	/* Reset Backup Domain */
	BKP_DeInit();
	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET);
	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}
void RTC_WriteTime(uint32_t time_stamp)
{
	PWR->CR|=(uint16_t)PWR_CR_DBP;
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	RTC_EnterConfigMode();
	/* Change the current time */
	RTC_SetCounter(time_stamp);
	RTC_ExitConfigMode();
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	PWR->CR&=~(uint16_t)PWR_CR_DBP;
}
void RTC_GetTime(RTC_Time_t *Time)
{
	uint32_t Time_Var = RTC_GetCounter();
	/* Compute  hours */
	Time->hour = Time_Var / 3600;
	/* Compute minutes */
	Time->minute = (Time_Var % 3600) / 60;
	/* Compute seconds */
	Time->second = (Time_Var % 3600) % 60;
}
/*Funtion for TimeStamp*/
uint32_t set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
    struct tm time_struct;
    time_t m_time;
    time_struct.tm_year = year - 1900; //1900
    time_struct.tm_mon = month;
    time_struct.tm_mday = day;
    time_struct.tm_hour = hour;
    time_struct.tm_min = minute;
    time_struct.tm_sec = second;
    m_time = mktime(&time_struct);// Convert to timestamp
    return m_time;
}
struct tm convert_time_stamp(uint32_t t)
{
    struct tm time_struct;
    uint32_t yOff, m, d, hh, mm, ss;
    t -= 946684800;//SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff)
    {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m)
    {
        uint8_t daysPerMonth = daysInMonth[m - 1];
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
    time_struct.tm_year = (yOff + 100);
    time_struct.tm_mon = m - 1;
    time_struct.tm_mday = d;
    time_struct.tm_hour = hh;
    time_struct.tm_min = mm;
    time_struct.tm_sec = ss;
//    mktime(&time_struct);
    return time_struct;
}
void date_time2str(char *buffer, struct tm *time_struct)
{
	char year[15]={0};
	char mon[15]={0};
	char day[15]={0};
	char hour[15]={0};
	char min[15]={0};
	char sec[15]={0};
	sprintf(year,"%d",time_struct->tm_year+1900);
	if (time_struct->tm_mon<10) sprintf(mon,"0%d",time_struct->tm_mon);
	else sprintf(mon,"%d",time_struct->tm_mon);
	if (time_struct->tm_mday<10) sprintf(day,"0%d",time_struct->tm_mday);
	else sprintf(day,"%d",time_struct->tm_mday);
	if (time_struct->tm_hour<10) sprintf(hour,"0%d",time_struct->tm_hour);
	else sprintf(hour,"%d",time_struct->tm_hour);
	if (time_struct->tm_min<10) sprintf(min,"0%d",time_struct->tm_min);
	else sprintf(min,"%d",time_struct->tm_min);
	if (time_struct->tm_sec<10) sprintf(sec,"0%d",time_struct->tm_sec);
	else sprintf(sec,"%d",time_struct->tm_sec);
	sprintf(buffer,"%s-%s-%sT%s-%s-%s+07:00",year,mon,day,hour,min,sec);

}
/* Tao chuoi Time co dinh dang hh:mm:ss */
void create_time_str(RTC_Time_t *time, char *time_str)
{
	uint8_t i = 0;
	if (time->hour<10) sprintf(&time_str[0],"0%d:",time->hour);
	else sprintf(&time_str[0],"%d:",time->hour);
	i = strlen(time_str);
	if (time->minute<10) sprintf(&time_str[i],"0%d:",time->minute);
	else sprintf(&time_str[i],"%d:",time->minute);
	i = strlen(time_str);
	if (time->second<10) sprintf(&time_str[i],"0%d",time->second);
	else sprintf(&time_str[i],"%d",time->second);
}
