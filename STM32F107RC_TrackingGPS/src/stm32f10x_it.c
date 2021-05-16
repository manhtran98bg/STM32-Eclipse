
#include "stm32f10x_it.h"
#include "usart/usart.h"
#include "lcd/sh1106.h"
#include "rtc/rtc.h"
#include "simcom/sim800.h"
#include "gps/gps.h"
volatile uint32_t msTicks=0;
volatile uint32_t myTicks_tim4=0;
uint32_t uwTick=0;
extern __IO char RxBuffer5[];
extern __IO uint8_t RxCounter5;

extern __IO char RxBuffer4[];
extern __IO uint8_t RxCounter4;
extern __IO uint8_t flagStart,flagStop;
extern char json_geowithtime[];
extern char time_buffer[];
extern RTC_Time_t Time;
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

void UART5_IRQHandler(void)
{
	char c;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
	{
		c = (char)USART_ReceiveData(UART5);
		if (c=='\n') {
			flagRx5 = 1;
			RxBuffer5[RxCounter5++] = c;
		}
		else {
			if (RxCounter5<64) RxBuffer5[RxCounter5++]=USART_ReceiveData(UART5);
			else RxCounter5 = 0;
		}
	}

}
void UART4_IRQHandler(void)
{
	uint8_t c;
#if !_USE_DMA
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		c = USART_ReceiveData(UART4);
		if (c=='$') {	//Start NMEA Sentence
			flagStart = 1;	//Flag indicate Start of NMEA Sentence
			RxCounter4 = 0;
			flagStop = 0;	//Flag indicate End of NMEA Sentence
		}
		if (c=='\n') {
			flagStart = 0;
			flagStop = 1;
			RMC_Parse(&gps_l70->RMC, (char*)RxBuffer4, RxCounter4);
			RMC_json_init(&gps_l70->RMC, json_geowithtime);
			gps_l70->gps_err = GPS_NO_ERR;
		}
		if (flagStart){
			if (RxCounter4<BUFFER_SIZE4) RxBuffer4[RxCounter4++]=c;	//Save Data to RxBuffer4
			else RxCounter4 = 0;
		}

	}
#else
	if (USART_GetITStatus(UART4, USART_IT_IDLE)!=RESET)
	{
		DMA_Receive_Datapack();
		trace_puts("IDLE");
	}
#endif
}
void DMA2_Channel3_IRQHandler()
{

}
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
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
		RTC_GetTime(&Time);
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
