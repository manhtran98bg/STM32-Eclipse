/*
 * driver.h
 *
 *  Created on: 23 Jun 2021
 *      Author: kylin
 */

#ifndef BACKUP_DRIVER_H_
#define BACKUP_DRIVER_H_
#include "main.h"
#include "backup.h"
#include "gps/gps.h"
typedef struct{
	bool is_active;
	uint8_t id[6];
	uint32_t active_time;	//sec
}driver_t;
extern driver_t driver;
bool driver_get_bkp_active();
void driver_write_bkp_active(bool is_active);
uint32_t driver_get_bkp_time();
void driver_write_bkp_time(uint32_t active_time);
void driver_get_bkp_id(uint8_t *id);
void driver_write_bkp_id(uint8_t *id);
void driver_json_payload(char *driver_id, RMC_Data *RMC, char *buffer);
void driver_payload_checkin(char *driver_id, RMC_Data *RMC, char *buffer);
void driver_payload_checkout(char *driver_id, RMC_Data *RMC,uint32_t active_time, char *buffer);
#endif /* BACKUP_DRIVER_H_ */
