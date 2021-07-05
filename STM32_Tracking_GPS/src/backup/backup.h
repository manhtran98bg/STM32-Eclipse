/*
 * backup.h
 *
 *  Created on: 23 Jun 2021
 *      Author: kylin
 */

#ifndef BACKUP_BACKUP_H_
#define BACKUP_BACKUP_H_
#include "main.h"
void rcc_backup_init(void);
uint16_t get_backup_data(uint16_t BKP_DR);
void write_backup_data(uint16_t BKP_DR, uint16_t data);


#endif /* BACKUP_BACKUP_H_ */
