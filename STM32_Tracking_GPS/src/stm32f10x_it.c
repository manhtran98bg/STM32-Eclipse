
#include "stm32f10x_it.h"
#include "usart/usart.h"
volatile uint32_t msTicks=0;
volatile uint32_t myTicks_tim4=0;
extern __IO uint8_t RxBuffer1[];
extern __IO uint8_t RxCounter1;

extern __IO uint8_t RxBuffer5[];
extern __IO uint8_t RxCounter5;

extern __IO uint8_t RxBuffer4[];
extern __IO uint8_t RxCounter4;
extern __IO uint8_t flagStart,flagStop;
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
void UART5_IRQHandler(void)
{
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
	{
		if (RxCounter5<64) RxBuffer5[RxCounter5++]=USART_ReceiveData(UART5);
		else RxCounter5 = 0;
	}
}
void UART4_IRQHandler(void)
{
	uint8_t c;
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
		}
		if (flagStart){
			if (RxCounter4<BUFFER_SIZE4) RxBuffer4[RxCounter4++]=c;	//Save Data to RxBuffer4
			else RxCounter4 = 0;
		}

	}
}
void USART1_IRQHandler(void)
{
	uint16_t c;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
//		c = USART_ReceiveData(USART1);
//		if (RxCounter1<BUFFER_SIZE1) RxBuffer1[RxCounter1++]=c;
//		else RxCounter1 = 0;
		Sim800_RxCallBack();
	}
}
