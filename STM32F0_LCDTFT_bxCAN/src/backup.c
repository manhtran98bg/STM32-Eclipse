/*
 * backup.c
 *
 *  Created on: Aug 27, 2020
 *      Author: manht
 */
#include "main.h"
void rcc_backup_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RCC_LSEConfig(RCC_LSE_ON);
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY)==RESET);
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);
}
uint32_t get_backup_data(void)
{
	return RTC_ReadBackupRegister(RTC_BKP_DR0);
}
void write_backup_data(uint32_t data)
{
	RTC_WriteBackupRegister(RTC_BKP_DR0, data);
}

