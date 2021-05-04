/*
 * rtc.c
 *
 *  Created on: May 1, 2021
 *      Author: manht
 */

#include "rtc.h"
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
		RTC_SetCounter(0x00);
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
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
void RTC_WriteTime(uint8_t hh, uint8_t mm, uint8_t ss)
{
	PWR->CR|=(uint16_t)PWR_CR_DBP;
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	RTC_EnterConfigMode();
	/* Change the current time */
	RTC_SetCounter(hh*3600 + mm*60 + ss);
	RTC_ExitConfigMode();
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	PWR->CR&=~(uint16_t)PWR_CR_DBP;
}
void RTC_GetTime(RTC_Time_t *Time)
{
	uint32_t Time_Var = RTC_GetCounter();
	/* Reset RTC Counter when Time is 23:59:59 */
	if (Time_Var == 0x0001517F)
	{
		RTC_SetCounter(0x0);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}
	/* Compute  hours */
	Time->hour = Time_Var / 3600;
	/* Compute minutes */
	Time->minute = (Time_Var % 3600) / 60;
	/* Compute seconds */
	Time->second = (Time_Var % 3600) % 60;

}
