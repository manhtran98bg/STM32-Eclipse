
#include "stm32f10x_it.h"
#include "usart/usart.h"
#include "lcd/sh1106.h"
#include "rtc/rtc.h"
#include "simcom/sim800.h"
#include "gps/gps.h"
volatile uint32_t msTicks=0;
volatile uint32_t myTicks_tim4=0;
uint32_t uwTick=0;



bool flagRx5=0;
extern bool _1sflag;
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

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) != RESET)
	{
	}
}
void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(GPS_UART, USART_IT_RXNE) != RESET)
	{
		gps_RxCallback();
	}
}
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		Sim800_RxCallBack();
	}
}
void RTC_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);
		time_struct = convert_time_stamp(RTC_GetCounter());
		Time.second = time_struct.tm_sec;
		Time.minute = time_struct.tm_min;
		Time.hour = time_struct.tm_hour;
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
