/*
 * gps.c
 *
 *  Created on: Apr 12, 2021
 *      Author: manht
 */

#include "gps.h"
uint8_t flagStart = 0,flagStop = 0;
char gps_buffer[GPS_BUFFER_SIZE];
uint16_t gps_buffer_index = 0;
extern FATFS	FatFs;
extern FIL	Fil;
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
static void gps_uart_clk_init()
{
	/*Enable UART clock and GPIO clock*/
	RCC_APB1PeriphClockCmd(GPS_UART_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(GPS_UART_GPIO_CLK, ENABLE);
}
static void gps_uart_gpio_init()
{
	GPIO_InitTypeDef gpio_init_structure;
	//Config UART4: PC10 = TX, PC11 = RX
	gpio_init_structure.GPIO_Pin = GPS_UART_GPIO_TX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPS_UART_GPIO, &gpio_init_structure);
	gpio_init_structure.GPIO_Pin = GPS_UART_GPIO_RX;
	gpio_init_structure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPS_UART_GPIO, &gpio_init_structure);
}
static void gps_uart_nvic_init()
{
	NVIC_InitTypeDef nvic_init_structure;
	nvic_init_structure.NVIC_IRQChannel = UART4_IRQn;
	nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_Init(&nvic_init_structure);
}
static void gps_uart_module_init(uint32_t baudrate)
{
	USART_InitTypeDef usart_init_structure;
	/* Baud rate = baudrate, 8-bit data, One stop bit
	* No parity, Do both Rx and Tx, No HW flow control
	*/
	USART_DeInit(GPS_UART);
	usart_init_structure.USART_BaudRate = baudrate;
	usart_init_structure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init_structure.USART_Parity = USART_Parity_No;
	usart_init_structure.USART_StopBits = USART_StopBits_1;
	usart_init_structure.USART_WordLength = USART_WordLength_8b;
	USART_Init(GPS_UART, &usart_init_structure);
	/* Enable RXNE interrupt */
	USART_ITConfig(GPS_UART, USART_IT_RXNE, ENABLE);
	/*Enable USART	 */
	USART_Cmd(GPS_UART, ENABLE);
}
void gps_check_current_baud(gps_t *gps)
{
	uint32_t baud[2]={115200,9600};
	uint32_t time_out=0;
	char buff_log[128]={0};
	int i = 0;
	//Check current baudrate of gps l70
	for (i=0;i<2;i++)
	{
		gps->gps_response = false;	//Defaut No Reponse
		time_out = 0;
		gps_uart_module_init(baud[i]);
		while ((!gps->gps_response)&&(time_out<10))
		{
			time_out++;
			delay_ms(200);
		}
		if (gps->gps_response) {
#if _DEBUG
			trace_write((char*)"log:", strlen("log:"));
			sprintf(buff_log,"Check curren baudrate gps L70: BAUD_RATE = %ld",baud[i]);
			trace_puts(buff_log);
#endif
#if _DEBUG_UART5
			sprintf(buff_log,"Check curren baudrate gps L70: BAUD_RATE = %ld\n",baud[i]);
			debug_send_string(buff_log);
#endif
			gps->gps_baudrate = baud[i];
			break;
		}
	}
}
void gps_init(gps_t *gps)
{
	char log[128]={0};
	gps_rst_pin_init();
	gps_power_on();
	if (sim800.power_state == OFF) {
		gps->gps_err = GPS_NO_PWR;
		gps->gps_pwr_state = false;
#if	_DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("GPS Power State: OFF");
#endif
#if _DEBUG_UART5
		debug_send_string("GPS Power State: OFF\n");
#endif
	}
	if (sim800.power_state==ON){
		gps->gps_err = GPS_NO_RES;
		gps->gps_baudrate = 9600;	//Default Baudrate;
		gps->gps_state = GPS_INITING;
		gps->gps_pwr_state = true;
#if	_DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("GPS Power State: ON");
#endif
#if _DEBUG_UART5
		debug_send_string("GPS Power State: ON\n");
#endif
		gps_uart_clk_init();
		gps_uart_gpio_init();
		gps_uart_nvic_init();
		gps_check_current_baud(gps);
		if (gps->gps_baudrate == 9600) {
#if _DEBUG_UART5
			debug_send_string("Change BAUD_RATE to 115200.\n");
#endif
			gps_set_baudrate(115200);
			delay_ms(500);
			gps_uart_clear_buffer();
			gps_check_current_baud(gps);
		}
		gps_uart_send_string((char*)"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
		delay_ms(500);
		gps->gps_state = GPS_INITED;
		gps_uart_clear_buffer();
		delay_ms(1000);
		while(!flagStop);
		RTC_WriteTime(set_time(gps->RMC.Date.year+2000,
				gps->RMC.Date.month,
				gps->RMC.Date.day,
				gps->RMC.Time.hh,
				gps->RMC.Time.mm,
				gps->RMC.Time.ss));
		sprintf(log,"GPS Time:%d-%d-%d\nGPS Date: %d/%d/%d",gps->RMC.Time.hh,
													gps->RMC.Time.mm,
													gps->RMC.Time.ss,
													gps->RMC.Date.day,
													gps->RMC.Date.month,
													gps->RMC.Date.year+2000);
#if	_DEBUG
		trace_write((char*)"log:", strlen("log:"));
		trace_puts(log);
		trace_write((char*)"log:", strlen("log:"));
		trace_puts("RTC Sync: OK");
#endif
#if _DEBUG_UART5
		debug_send_string(log);
		debug_send_string((char*)"\n");
		debug_send_string("RTC Sync: OK\n");
#endif
	}
}
/*	Function for gps l70 Usart */

/**
  * @brief  Clear uart buffer of sim800
  * @param 	None
  * @retval None
  */
void gps_uart_clear_buffer()
{
	for (int i=0;i<GPS_BUFFER_SIZE;i++) gps_buffer[i]=0;
	gps_buffer_index=0;
}
/**
  * @brief  Send a string to uart
  * @param 	str : Pointer to String or Array.
  * @retval None
  */
void gps_uart_send_string(char *str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(GPS_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(GPS_UART,*str);
		str++;
	}
}
/**
  * @brief  Send a char to uart
  * @param 	chr: char need to send
  * @retval None
  */
void gps_uart_send_char( char chr)
{
	while(USART_GetFlagStatus(GPS_UART,USART_FLAG_TXE) == RESET);
	USART_SendData(GPS_UART,chr);
}
/**
  * @brief  Send array data to uart
  * @param 	str: pointer to array.
  * @param	length: data length
  * @retval None
  */
void gps_uart_send_array(unsigned char *str, uint8_t length)
{
	for(int i=0;i<length;i++)
	{
		while(USART_GetFlagStatus(GPS_UART,USART_FLAG_TXE) == RESET);
		USART_SendData(GPS_UART,*(str+i));
	}
}

void gps_set_baudrate(uint32_t baud)
{
	char cmdBuf[25];
	sprintf(cmdBuf,"$PMTK251,%ld*1F\r\n",baud);
	gps_uart_send_string(cmdBuf);
}
uint8_t  gps_read_data(gps_t *gps)
{
	if (flagStop)
	{
#if _DEBUG
		trace_puts((char*)gps_buffer);
#endif
#if _USE_DEBUG_UART
		debug_send_string((char*)gps_buffer);
		debug_send_chr('\n');
#endif
		gps_uart_clear_buffer();
		flagStop=0;
		return 1;
	}
	return 0;
}
void gps_RxCallback(gps_t *gps){
	unsigned char c;
	UINT	bw;
	FRESULT	fr;
	int i=0;
	c = USART_ReceiveData(GPS_UART);
	if (c=='$') {	//Start NMEA Sentence
		flagStart = 1;	//Flag indicate Start of NMEA Sentence
		gps_buffer_index = 0;
		flagStop = 0;	//Flag indicate End of NMEA Sentence
		gps->gps_response = true;
	}
	if (c=='\n') {
		flagStart = 0;
		flagStop = 1;
		if (gps->gps_state == GPS_INITED){
			RMC_Parse(&gps->RMC, (char*)gps_buffer, gps_buffer_index);
			RMC_json_init(&gps->RMC, json_geowithtime);
//			sprintf(sd_buffer,"%d-%d-%d\n",gps->RMC.Time.hh,
//										gps->RMC.Time.mm,
//										gps->RMC.Time.ss);
//			fr = f_open(&Fil, "DATALOG.txt", FA_WRITE|FA_OPEN_APPEND);
//			if (fr == FR_OK) {
//					f_write(&Fil, sd_buffer, strlen(sd_buffer), &bw);	/* Write data to the file */
//					fr = f_close(&Fil);
//			}

		}
		gps->gps_err = GPS_NO_ERR;
	}
	if (flagStart){
		if (gps_buffer_index<GPS_BUFFER_SIZE) gps_buffer[gps_buffer_index++]=c;	//Save Data to gps_buffer
		else gps_buffer_index = 0;
	}
	if (flagStop){
		gps_uart_clear_buffer();
	}
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
static void fix_gps_time(RMC_Data *RMC)
{
	int day_of_mon[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	if (RMC->Time.hh+7>23) {
		RMC->Time.hh = RMC->Time.hh+7-24;
		RMC->Date.day = RMC->Date.day+1;
	}
	else RMC->Time.hh = RMC->Time.hh+7;
	if (RMC->Date.day>day_of_mon[RMC->Date.month-1]){
		RMC->Date.day = 1;
		RMC->Date.month = RMC->Date.month+1;
	}
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
        if (k==7&&strlen(temp)>0) {
        	RMC->Speed = str2float(temp);
        }
        if (k==9) RMC_GetDate(RMC, temp);
        k++;
    }
    fix_gps_time(RMC);
    return true;
}

