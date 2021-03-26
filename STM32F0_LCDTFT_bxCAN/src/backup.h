/*
 * backup.h
 *
 *  Created on: Aug 27, 2020
 *      Author: manht
 */

#ifndef BACKUP_H_
#define BACKUP_H_
#include "main.h"
void rcc_backup_init(void);
uint32_t get_backup_data(void);
void write_backup_data(uint32_t data);
#endif /* BACKUP_H_ */
