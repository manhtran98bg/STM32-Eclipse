/*
 * backup.c
 *
 *  Created on: 23 Jun 2021
 *      Author: kylin
 */
#include "backup.h"
void rcc_backup_init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	BKP_ClearFlag();
}
uint16_t get_backup_data(uint16_t BKP_DR)
{
	return BKP_ReadBackupRegister(BKP_DR);
}
void write_backup_data(uint16_t BKP_DR, uint16_t data)
{
	PWR_BackupAccessCmd(ENABLE);
	BKP_WriteBackupRegister(BKP_DR, data);
	PWR_BackupAccessCmd(DISABLE);
}
