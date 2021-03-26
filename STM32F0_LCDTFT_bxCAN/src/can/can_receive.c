/*
 * can_receive.c
 *
 *  Created on: Aug 20, 2020
 *      Author: manht
 */
#include "can_receive.h"
#include "../src/board.h"
#include "../src/backup.h"
#define DISPLAY_TEMP	1
#define DISPLAY_VOL		1
extern volatile uint8_t can_flag;
extern CanRxMsg RxMessage;
extern volatile uint8_t can_mes_cnt;
uint32_t torque,temp=0,speed=0,error,dcin=0;
float distance_tmp;
extern uint32_t distance;
extern float distance_tmp1;
/* Dialmeter Banh xe 0.47m */
#define RPM_TO_KMH	0.088548 //(1/60)*3.14*dialmeter*3.6
#define RPM_TO_MS	0.024597 //(1.60)*3.14*dialmeter
static uint32_t can_data_process(uint8_t *dat)
{
	uint32_t tmp=0;
	tmp |=(uint32_t)(*dat);
	dat++;
	tmp |=(uint32_t)(*dat<<8);
	dat++;
	tmp |=(uint32_t)(*dat<<16);
	dat++;
	tmp |=(uint32_t)(*dat<<24);
	return tmp;
}
static void can_check_mc_state(uint32_t state)
{
	switch (state)
	{
	case SMC_ERR_NONE:
		USART1_Send_String((uint8_t*)"SMC_ERR_NONE\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[0].image);
		break;
	case SMC_ERR_OC:
		USART1_Send_String((uint8_t*)"SMC_ERR_OC\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[1].image);
		break;
	case SMC_ERR_OV:
		USART1_Send_String((uint8_t*)"SMC_ERR_OV\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[2].image);
		break;
	case SMC_ERR_UV:
		USART1_Send_String((uint8_t*)"SMC_ERR_UV\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[3].image);
		break;
	case SMC_ERR_HALL_ANGLE:
		USART1_Send_String((uint8_t*)"SMC_ERR_HALL_ANGLE\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[4].image);
		break;
	case SMC_ERR_HALL_CABLE1:
		USART1_Send_String((uint8_t*)"SMC_ERR_HALL_CABLE1\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[5].image);
		break;
	case SMC_ERR_HALL_CABLE2:
		USART1_Send_String((uint8_t*)"SMC_ERR_HALL_CABLE2\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 120, (uint8_t*)Font_array1[6].image);
		break;
	case SMC_ERR_HALL_CABLE3:
		USART1_Send_String((uint8_t*)"SMC_ERR_HALL_CABLE3\n");
		BSP_LCD_DrawBitmap_Grayscale4(50, 125, (uint8_t*)Font_array1[7].image);
		break;
	case SMC_ERR_TEST:
		USART1_Send_String((uint8_t*)"SMC_ERR_TEST\n");
		BSP_LCD_DrawBitmap_Grayscale4(55, 125, (uint8_t*)Font_array1[8].image);
		break;
	default:
		break;
	}
}
void can_receive_data()
{
	if (can_flag)
	{
		if (RxMessage.StdId == 581) torque = can_data_process(RxMessage.Data);
		else if (RxMessage.StdId == 681) speed = can_data_process(RxMessage.Data);
		else if (RxMessage.StdId == 781) temp = can_data_process(RxMessage.Data);
		else if (RxMessage.StdId == 881) error = can_data_process(RxMessage.Data);
		else if (RxMessage.StdId == 981) dcin = can_data_process(RxMessage.Data);
		can_flag = 0;
	}
}
static uint8_t rpm_to_kmh(uint32_t rpm)
{
	return rpm*RPM_TO_KMH;
}
static float rpm_to_ms(uint32_t rpm)
{
	return rpm*RPM_TO_MS;
}
static void display_speed(uint32_t speed)
{
	uint8_t chuc,dvi;
	chuc = rpm_to_kmh(speed)/10 ;
	dvi = rpm_to_kmh(speed) % 10;
	BSP_LCD_DrawBitmap_Grayscale4(170, 105, (uint8_t *)Font_array[chuc].image);
	BSP_LCD_DrawBitmap_Grayscale4(245, 105, (uint8_t *)Font_array[dvi].image);
}
void display_odo(uint32_t distance)
{
	uint32_t km;
	uint16_t m;
	uint8_t int_part1,int_part2,int_part3,int_part4;
	uint8_t dec_part1;
	km = distance/1000;
	m = distance % 1000;
	int_part1=km/1000;
	int_part2=(km%1000)/100;
	int_part3=(km%100)/10;
	int_part4=(km%10);
	dec_part1=m/100;
	BSP_LCD_DrawBitmap_Grayscale4(55, 27, (uint8_t*)Font_array1[int_part1].image);
	BSP_LCD_DrawBitmap_Grayscale4(70, 27, (uint8_t*)Font_array1[int_part2].image);
	BSP_LCD_DrawBitmap_Grayscale4(85, 27, (uint8_t*)Font_array1[int_part3].image);
	BSP_LCD_DrawBitmap_Grayscale4(100, 27, (uint8_t*)Font_array1[int_part4].image);
	BSP_LCD_DrawBitmap_Grayscale4(115, 27, (uint8_t*)dot);
	BSP_LCD_DrawBitmap_Grayscale4(127, 27, (uint8_t*)Font_array1[dec_part1].image);
	BSP_LCD_DrawBitmap_Grayscale4(142, 27, (uint8_t*)km_distance);
}
void display_trip(uint32_t distance)
{
	uint32_t km;
	uint16_t m;
	uint8_t int_part1,int_part2;
	uint8_t dec_part1;
	km = distance/1000;
	m = distance % 1000;
	int_part1=km/10;
	int_part2=km%10;
	dec_part1=m/100;
	BSP_LCD_DrawBitmap_Grayscale4(360, 27, (uint8_t*)Font_array1[int_part1].image);
	BSP_LCD_DrawBitmap_Grayscale4(375, 27, (uint8_t*)Font_array1[int_part2].image);
	BSP_LCD_DrawBitmap_Grayscale4(390, 27, (uint8_t*)dot);
	BSP_LCD_DrawBitmap_Grayscale4(402, 27, (uint8_t*)Font_array1[dec_part1].image);
	BSP_LCD_DrawBitmap_Grayscale4(417, 27, (uint8_t*)km_distance);
}
void display_temp(uint32_t temp)
{
	uint8_t chuc, dvi;
	chuc = temp / 10;
	dvi = temp % 10;
	BSP_LCD_DrawBitmap_Grayscale4(50, 180, (uint8_t*)Font_array1[chuc].image);
	BSP_LCD_DrawBitmap_Grayscale4(65, 180, (uint8_t*)Font_array1[dvi].image);
}
void display_voltage(uint32_t voltage)
{
	uint8_t int_part1,int_part2,int_part3;
	int_part1 = voltage/100;
	int_part2 = (voltage%100)/10;
	int_part3 = (voltage%10);
	BSP_LCD_DrawBitmap_Grayscale4(50, 150, (uint8_t*)Font_array1[int_part1].image);
	BSP_LCD_DrawBitmap_Grayscale4(65, 150, (uint8_t*)Font_array1[int_part2].image);
	BSP_LCD_DrawBitmap_Grayscale4(80, 150, (uint8_t*)Font_array1[int_part3].image);
}
static void can_send_data_uart(void)
{
	char buf[20];
	sprintf(buf,"%lu,",torque);
	USART1_Send_String((uint8_t*)buf);
	sprintf(buf,"%lu,",speed);
	USART1_Send_String((uint8_t*)buf);
	sprintf(buf,"%lu,",temp);
	USART1_Send_String((uint8_t*)buf);
	sprintf(buf,"%lu,",dcin);
	USART1_Send_String((uint8_t*)buf);
	can_check_mc_state(error);

}
void can_send_data(void)
{
	if (can_mes_cnt >= 5)
	{
		can_send_data_uart();
		distance_tmp +=rpm_to_ms(speed);
		distance_tmp1 +=rpm_to_ms(speed);
		if (distance_tmp>100)
		{
			distance+=distance_tmp;
			distance_tmp = 0;
			write_backup_data(distance);
		}
		display_speed(speed);
		display_odo(distance);
		display_trip((uint32_t)distance_tmp1);
#if DISPLAY_TEMP
		display_temp(temp);
#endif
#if DISPLAY_VOL
		display_voltage(dcin);
#endif
		can_mes_cnt =0;
	}
}

