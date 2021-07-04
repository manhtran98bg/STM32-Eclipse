#include "main.h"
#include "../src/sr04/sr04.h"
#include "../src/hc05/hc05.h"
volatile uint32_t msTicks=0;
volatile uint32_t myTicks_tim4=0;
volatile uint32_t uwTick=0;
extern int dust;
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


void USART1_IRQHandler(){
	unsigned char c;
	if(USART_GetITStatus(HC05_UART, USART_IT_RXNE) != RESET){
		c = USART_ReceiveData(HC05_UART);
		if (c=='$'){
			rx_flag = 1;
			hc05_rx_index = 0;
		}
		if (c=='#'){
			rx_flag = 0;
			hc05_parse_data(hc05_rx_buffer, &dust);
			hc05_clear_buffer();
		}
		if (rx_flag){
			if (hc05_rx_index<HC05_BUFFER_SIZE) hc05_rx_buffer[hc05_rx_index++]=c;
			else hc05_rx_index = 0;
		}
	}
}
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
	}
}
void EXTI9_5_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line5)!=RESET)
	{
		 EXTI_ClearITPendingBit(EXTI_Line5);
		 duration = TIM_GetCounter(TIM4);
		 stop_tim4();
		 sr04_flag = 1;
	}
}
