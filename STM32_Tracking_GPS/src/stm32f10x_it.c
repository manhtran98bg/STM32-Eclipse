
#include "stm32f10x_it.h"
#include "usart/usart.h"
#include "lcd/sh1106.h"
#include "rtc/rtc.h"
#include "simcom/sim800.h"
#include "gps/gps.h"
#include "rfid/mfrc552.h"
#include "sdcard/sdmm.h"
#include "rs232/rs232.h"
volatile uint32_t msTicks=0;
volatile uint32_t myTicks_tim4=0;
uint32_t uwTick=0;

extern bool _1sflag;
extern bool board_state;
void NMI_Handler(void)
{

}
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}


void PendSV_Handler(void)
{
}
void SysTick_Handler(void)
{
	uwTick++;
	if(msTicks !=0)
	{
		msTicks--;
	}
}
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update)!=RESET)
	{
		TIM_ClearFlag(TIM4, TIM_FLAG_Update);
	}
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update)!=RESET)
	{
		TIM_ClearFlag(TIM5, TIM_FLAG_Update);
	}
}

void UART5_IRQHandler(void)
{
	unsigned char c;
	if(USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) != RESET)
	{
			c = USART_ReceiveData(DEBUG_UART);
			USART_SendData(UART5, c);
	}

}
void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(GPS_UART, USART_IT_RXNE) != RESET)
	{
		gps_RxCallback(&gps_l70);
	}
}
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(SIM_UART, USART_IT_RXNE) != RESET)
	{
		Sim800_RxCallBack();
	}
}
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(RS232_UART, USART_IT_RXNE) != RESET)
	{
		rs232_rx_callback();
	}
}
void RTC_IRQHandler(void)
{
	int i=0;
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);
		if ((rfid.present == true)&&(rfid.t_out>0)) rfid.t_out--;	//Cho t_out de doc RFID
		time_struct = convert_time_stamp(RTC_GetCounter());
		Time.second = time_struct.tm_sec;
		Time.minute = time_struct.tm_min;
		Time.hour = time_struct.tm_hour;
		Time.year = time_struct.tm_year;
		Time.mon = time_struct.tm_mon;
		Time.day = time_struct.tm_mday;
		if (sdcard.mount==true){
			if (Time.old_day!=Time.day){
				Time.old_day=Time.day;
				create_directory(directory,&time_struct);	//Kiem tra neu sang ngay moi=>tao duong dan.
			}
		}
		if (board_state ==true){
			if (gps_data_count==0){
				gps_data_count++;
				memset(sd_buffer,0,128);
				create_time_str(&Time, time_str);
				sprintf(&sd_buffer[0],"%s, %d,",time_str,(int)gps_l70.RMC.Speed);
			}
			else if (gps_data_count==29){
				gps_data_count = 0;
				i = strlen (sd_buffer);
				sprintf(&sd_buffer[i],"%d\n",(int)gps_l70.RMC.Speed);
				trace_puts(sd_buffer);
				write2file(directory, strlen(directory), "SPEED.LOG",sd_buffer, strlen (sd_buffer));
			}
			else {
				i = strlen (sd_buffer);
				sprintf(&sd_buffer[i],"%d,",(int)gps_l70.RMC.Speed);
				gps_data_count++;
			}
		}
		if (Time.minute<10) sprintf(time_buffer,"%d:0%d",Time.hour,Time.minute);
		else sprintf(time_buffer,"%d:%d",Time.hour,Time.minute);
		if (Time.old_minute != Time.minute){
			_1sflag = true;
			Time.old_minute = Time.minute;
		}
	    /* Wait until last write operation on RTC registers has finished */
	    RTC_WaitForLastTask();
	}
}
void EXTI15_10_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line12)!=RESET)
	{
		 EXTI_ClearITPendingBit(EXTI_Line12);
	}
	if(EXTI_GetITStatus(EXTI_Line13)!=RESET)
	{
		 EXTI_ClearITPendingBit(EXTI_Line13);
	}
	if(EXTI_GetITStatus(EXTI_Line14)!=RESET)
	{
		 EXTI_ClearITPendingBit(EXTI_Line14);
	}
}
void EXTI0_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
	{
		 EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

