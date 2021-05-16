/*
 * gps.c
 *
 *  Created on: Apr 12, 2021
 *      Author: manht
 */

#include "gps.h"
__IO uint8_t flagStart = 0,flagStop = 0;
extern __IO uint8_t RxBuffer4[];
extern __IO uint8_t RxCounter4;
extern char dmaRxbuffer[];
extern char json_geowithtime[];
void gps_power_on()
{
	power_on_gps();
}
void gps_power_off()
{
	power_off_gps();
}
void gps_reset()
{
	GPIO_SetBits(GPS_RST_PORT, GPS_RST_PIN);
	delay_ms(20);
	GPIO_ResetBits(GPS_RST_PORT, GPS_RST_PIN);
}
static void gps_rst_pin_init()
{
	RCC_APB2PeriphClockCmd(GPS_RST_CLK, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPS_RST_PIN;
	GPIO_InitStruct.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPS_RST_PORT, &GPIO_InitStruct);
	GPIO_ResetBits(GPS_RST_PORT, GPS_RST_PIN);
}
void gps_init()
{
	gps_rst_pin_init();
	gps_power_on();
	gps_l70->gps_err = GPS_NO_RES;
	delay_ms(1000);
	UART4_Send_String((char*)"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
//	UART4_Send_String((char*)"$PMTK251,115200*1F\r\n");
	USART_clear_buf(4);
}
void gps_set_baudrate(int baud)
{
	char cmdBuf[25];
	sprintf(cmdBuf,"$PMTK251,%d*1F\r\n",baud);
	UART4_Send_String(cmdBuf);
}
uint8_t  gps_read_data(gps_t *gps)
{
	if (flagStop)
	{
		RMC_Parse(&gps->RMC, (char*)RxBuffer4, RxCounter4);
		RMC_json_init(&gps->RMC, json_geowithtime);
#if _DEBUG
		trace_puts((char*)RxBuffer4);
#endif
#if _DEBUG_GPS_UART5
		UART5_Send_String((char*)RxBuffer4);
		USART_SendData(UART5, '\n');
#endif
		USART_clear_buf(4);
		flagStop=0;
		return 1;
	}
	return 0;
}
static bool RMC_GetDate(RMC_Data *RMC, char *Date_str)
{
	int i=0;
	char dd[3]={0};
	char mm[3]={0};
	char yy[3]={0};
	for (i=0;i<2;i++) dd[i]=Date_str[i];
	RMC->Date.day = atoi(dd);
	for (i=0;i<2;i++) mm[i]=Date_str[i+2];
	RMC->Date.month = atoi(mm);
	for (i=0;i<2;i++) yy[i]=Date_str[i+4];
	RMC->Date.year = atoi(yy);
	return 1;
}
static bool RMC_GetTime(RMC_Data *RMC, char *Time_str)
{
	int i=0;
	char hour[3]={0};
	char minute[3]={0};
	char second[3]={0};
	char tik[4]={0};
	for (i=0;i<2;i++) hour[i]=Time_str[i];
	RMC->Time.hh = atoi(hour);
	for (i=0;i<2;i++) minute[i]=Time_str[i+2];
	RMC->Time.mm = atoi(minute);
	for (i=0;i<2;i++) second[i]=Time_str[i+4];
	RMC->Time.ss = atoi(second);
	for (i=0;i<3;i++) tik[i]=Time_str[i+7];
	RMC->Time.sss = atoi(tik);
	return 1;
}
static bool RMC_GetLatitude(RMC_Data *RMC,char *Lat_str)
{
	char lat_dd[3]={0};
	char lat_mm[3]={0};
	char lat_mmmm[5]={0};
	double M_m,D_d;
	int i=0,j=0;
	if (strlen(Lat_str)==0)
	{
		RMC->Lat.lat_dd = 0;
		RMC->Lat.lat_mm = 0;
		RMC->Lat.lat_mmmm = 0;
		RMC->Lat.lat_dec_degree.int_part=0;
		RMC->Lat.lat_dec_degree.dec_part=0;
	}
	else
	{
		for (i=0;i<2;i++) lat_dd[j++]=Lat_str[i];
		j=0;
		for (i=0;i<2;i++) lat_mm[j++]=Lat_str[i+2];
		j=0;
		for (i=0;i<4;i++) lat_mmmm[j++]=Lat_str[i+5];
		RMC->Lat.lat_dd = atoi(lat_dd);
		RMC->Lat.lat_mm = atoi(lat_mm);
		RMC->Lat.lat_mmmm = atoi(lat_mmmm);
		M_m = RMC->Lat.lat_mm + (double)RMC->Lat.lat_mmmm/10000.0;
		D_d = (double)M_m/60.0;
		RMC->Lat.lat_dec_degree.int_part = RMC->Lat.lat_dd;
		RMC->Lat.lat_dec_degree.dec_part =(long) (D_d*100000000);
	}
	return 1;
}

static bool RMC_GetLongitude(RMC_Data *RMC,char *Lon_str)
{
	char lon_ddd[4]={0};
	char lon_mm[3]={0};
	char lon_mmmm[5]={0};
	double M_m,D_d;
	int i=0,j=0;
	if (strlen(Lon_str)==0)
	{
		RMC->Lon.lon_ddd = 0;
		RMC->Lon.lon_mm = 0;
		RMC->Lon.lon_mmmm = 0;
		RMC->Lon.lon_dec_degree.int_part=0;
		RMC->Lon.lon_dec_degree.dec_part=0;
	}
	else
	{
		for (i=0;i<3;i++) lon_ddd[j++]=Lon_str[i];
		j=0;
		for (i=0;i<2;i++) lon_mm[j++]=Lon_str[i+3];
		j=0;
		for (i=0;i<4;i++) lon_mmmm[j++]=Lon_str[i+6];
		RMC->Lon.lon_ddd = atoi(lon_ddd);
		RMC->Lon.lon_mm = atoi(lon_mm);
		RMC->Lon.lon_mmmm = atoi(lon_mmmm);
		M_m = RMC->Lon.lon_mm + (double)RMC->Lon.lon_mmmm/10000.0;
		D_d = (double)M_m/60.0;
		RMC->Lon.lon_dec_degree.int_part = RMC->Lon.lon_ddd;
		RMC->Lon.lon_dec_degree.dec_part =(long) (D_d*100000000);
	}
	return 1;
}
/* Parse data to json GeoWithTime
 * Param: 	RMC: RMC data structure pointer
 * 			buffer: Pointer to buffer array.
 * Return:	No return
 * 			GeoWithTime: Data Json:
 * 			{
 * 				"long":"D.d",
 * 				"lat":"D.d",
 * 				"time":"UTC DateTime ISO 8601"
 * 			}
 */
void RMC_json_init(RMC_Data *RMC, char *buffer)
{
	sprintf(buffer,"{\"lng\":\"%d.%ld\","
				   "\"lat\":\"%d.%ld\","
				   "\"time\":\"20%d-0%d-%dT0%d:%d:%d+00:00\"}",RMC->Lon.lon_dec_degree.int_part,RMC->Lon.lon_dec_degree.dec_part,
				   RMC->Lat.lat_dec_degree.int_part,RMC->Lat.lat_dec_degree.dec_part,
				   RMC->Date.year,RMC->Date.month,RMC->Date.day,
				   RMC->Time.hh,RMC->Time.mm,RMC->Time.ss);
}
static double str2float(char *str)
{
	int int_part = 0,dec_part = 0;
	unsigned int i=0,j=0,lt=1;
	char int_chr[10]={0};
	char dec_chr[10]={0};
	while (str[i]!='.') {
	    int_chr[i]=str[i];
	    i++;
	}
	i++;
	while (i<strlen(str)){
	    dec_chr[j++]=str[i];
	    i++;
	}
	int_part = atoi(int_chr);
    dec_part = atoi(dec_chr);
    for (i=0;i<strlen(dec_chr);i++) lt=lt*10;
    return (double)(int_part+(double)dec_part/lt);
}
bool RMC_Parse(RMC_Data *RMC, char *RMC_Sentence, int RMC_len)
{
    int i=0,j=0,k=0;
    char temp[15]={0};
    if (RMC_Sentence[0]!='$') return false;
    while (i<RMC_len)
    {
    	for (int index=0;index<15;index++)temp[index]=0;
        while((i<RMC_len)&&(RMC_Sentence[i]!=','))
        {
           temp[j++]=RMC_Sentence[i];
           i++;
        }
        j=0;
        i++;
        if (k==0) strcpy(RMC->ID,temp);
        if (k==1) RMC_GetTime(RMC, temp);
        if (k==2) strcpy(RMC->Data_Valid,temp);
        if (k==3) RMC_GetLatitude(RMC,temp);
        if (k==4) strcpy(RMC->Lat_dir,temp);
        if (k==5) RMC_GetLongitude(RMC,temp);
        if (k==6) strcpy(RMC->Lon_dir,temp);
        if (k==7) RMC->Speed = str2float(temp);
        if (k==9) RMC_GetDate(RMC, temp);
        k++;
    }
    return true;
}
void DMA_Receive_Datapack(void)
{
	static size_t old_pos;
	size_t pos;
	/* Calculate current position in buffer */
	pos = 512 - DMA_GetCurrDataCounter(DMA2_Channel3);
	if (pos != old_pos) { /* Check change in received data */
		if (pos > old_pos) { /* Current position is over previous one */
			/* We are in "linear" mode */
			/* Process data directly by subtracting "pointers" */
			usart_process_data(&dmaRxbuffer[old_pos], pos - old_pos);
		} else {
			/* We are in "overflow" mode */
			/* First process data to the end of buffer */
			usart_process_data(&dmaRxbuffer[old_pos], 512 - old_pos);
			/* Check and continue with beginning of buffer */
			if (pos > 0) {
				usart_process_data(&dmaRxbuffer[0], pos);
			}
		}
		old_pos = pos;                          /* Save current position as old */
	}
}
void usart_process_data(const void* data, size_t len) {
	const uint8_t* d = data;
	char tmp[128]={0};
    /*
     * This function is called on DMA TC and HT events, aswell as on UART IDLE (if enabled) line event.
     *
     * For the sake of this example, function does a loop-back data over UART in polling mode.
     * Check ringbuff RX-based example for implementation with TX & RX DMA transfer.
     */
		for (int i=0;i<len;i++)
		{
			tmp[i]=*d;
			d++;
		}
		trace_puts(tmp);
}
