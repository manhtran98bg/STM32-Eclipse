/*
 * driver.c
 *
 *  Created on: 23 Jun 2021
 *      Author: kylin
 */
#include "driver.h"
driver_t driver;
uint32_t driver_get_bkp_time(){
	__IO uint32_t active_t_l=0;
	__IO uint32_t active_t_h=0;
	active_t_h = get_backup_data(BKP_DR4)<<16;
	active_t_l = get_backup_data(BKP_DR5);
	return active_t_h|active_t_l;
}
void driver_write_bkp_time(uint32_t active_time){
	__IO uint32_t active_t_l=0;
	__IO uint32_t active_t_h=0;
	active_t_h = (active_time & 0xffff0000 )>>16;
	active_t_l = active_time & 0x0000ffff;
	write_backup_data(BKP_DR4, active_t_h);
	write_backup_data(BKP_DR5, active_t_l);
}
void driver_get_bkp_id(uint8_t *id){
	uint16_t bkp_data = 0;
	uint16_t bkp_reg[3]={BKP_DR6,BKP_DR7,BKP_DR8};
	uint8_t i;
	for (i=0;i<3;i++){
		bkp_data = get_backup_data(bkp_reg[i]);
		id[2*i]=(bkp_data&0xff00)>>8;
		id[2*i+1]=bkp_data&0x00ff;
	}
}
void driver_write_bkp_id(uint8_t *id){
	uint8_t i=0;
	uint16_t bkp_data = 0;
	uint16_t bkp_reg[3]={BKP_DR6,BKP_DR7,BKP_DR8};
	for (i=0;i<3;i++){
		bkp_data = (uint16_t)(id[2*i]<<8)|id[2*i+1];
		write_backup_data(bkp_reg[i], bkp_data);
	}
}
bool driver_get_bkp_active(){
	return get_backup_data(BKP_DR3);
}
void driver_write_bkp_active(bool is_active){
	write_backup_data(BKP_DR3, is_active);
}
void driver_payload_checkin(char *driver_id, RMC_Data *RMC, char *buffer){
	char year_buf[5]={0};
	char mon_buf[5]={0};
	char day_buf[5]={0};
	char hour_buf[5]={0};
	char min_buf[5]={0};
	char sec_buf[5]={0};
	//YYMMDD
	sprintf(year_buf,"20%d", RMC->Date.year);
	if (RMC->Date.month<10) sprintf(mon_buf,"0%d", RMC->Date.month);
	else sprintf(mon_buf,"%d", RMC->Date.month);
	if (RMC->Date.day<10) sprintf(day_buf,"0%d", RMC->Date.day);
	else sprintf(day_buf,"%d", RMC->Date.day);
	//HHMMSS
	if (RMC->Time.hh<10) sprintf(hour_buf,"0%d", RMC->Time.hh);
	else sprintf(hour_buf,"%d", RMC->Time.hh);
	if (RMC->Time.mm<10) sprintf(min_buf,"0%d", RMC->Time.mm);
	else sprintf(min_buf,"%d", RMC->Time.mm);
	if (RMC->Time.ss<10) sprintf(sec_buf,"0%d", RMC->Time.ss);
	else sprintf(sec_buf,"%d", RMC->Time.ss);
	sprintf(buffer,"{\"code\":\"%s\","
			       "\"lat\":\"%s\","
				   "\"lng\":\"%s\","
				   "\"time\":\"%s-%s-%sT%s:%s:%s+00:00\"}",driver_id,RMC->Lat.lat_dec_degree,
				   RMC->Lon.lon_dec_degree,
				   year_buf,mon_buf,day_buf,
				   hour_buf,min_buf,sec_buf);
}
void driver_payload_checkout(char *driver_id, RMC_Data *RMC,uint32_t active_time, char *buffer){
	char year_buf[5]={0};
	char mon_buf[5]={0};
	char day_buf[5]={0};
	char hour_buf[5]={0};
	char min_buf[5]={0};
	char sec_buf[5]={0};
	//YYMMDD
	sprintf(year_buf,"20%d", RMC->Date.year);
	if (RMC->Date.month<10) sprintf(mon_buf,"0%d", RMC->Date.month);
	else sprintf(mon_buf,"%d", RMC->Date.month);
	if (RMC->Date.day<10) sprintf(day_buf,"0%d", RMC->Date.day);
	else sprintf(day_buf,"%d", RMC->Date.day);
	//HHMMSS
	if (RMC->Time.hh<10) sprintf(hour_buf,"0%d", RMC->Time.hh);
	else sprintf(hour_buf,"%d", RMC->Time.hh);
	if (RMC->Time.mm<10) sprintf(min_buf,"0%d", RMC->Time.mm);
	else sprintf(min_buf,"%d", RMC->Time.mm);
	if (RMC->Time.ss<10) sprintf(sec_buf,"0%d", RMC->Time.ss);
	else sprintf(sec_buf,"%d", RMC->Time.ss);
	sprintf(buffer,"{\"code\":\"%s\","
			       "\"lat\":\"%s\","
				   "\"lng\":\"%s\","
				   "\"time\":\"%s-%s-%sT%s:%s:%s+00:00\","
				   "\"active_time\":\"%ld\"}",driver_id,RMC->Lat.lat_dec_degree,
				   RMC->Lon.lon_dec_degree,
				   year_buf,mon_buf,day_buf,
				   hour_buf,min_buf,sec_buf,active_time);
}
