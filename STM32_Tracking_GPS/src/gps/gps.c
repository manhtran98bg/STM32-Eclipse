/*
 * gps.c
 *
 *  Created on: Apr 12, 2021
 *      Author: manht
 */

#include "gps.h"
__IO uint8_t flagStart = 0,flagStop = 0;
extern __IO uint8_t RxBuffer4[];

void gps_power_on()
{
	power_on_gps();
}
void gps_power_off()
{
	power_off_gps();
}
uint8_t  gps_read_data()
{
	if (flagStop)
	{
#if _DEBUG
		trace_puts((char*)RxBuffer4);
#endif
#if _DEBUG_GPS_UART5
		UART5_Send_String((uint8_t*)RxBuffer4);
		USART_SendData(UART5, '\n');
#endif
		USART_clear_buf(4);
		flagStop=0;
		return 1;
	}
	return 0;
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
	delay_ms(1000);
	UART4_Send_String((uint8_t*)"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
	USART_clear_buf(4);
}
