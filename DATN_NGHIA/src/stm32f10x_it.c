#include "stm32f10x_it.h"
#include "usart/usart.h"
#include "hc05/hc05.h"
volatile uint32_t msTicks=0;
volatile uint32_t myTicks_tim4=0;
volatile uint32_t uwTick=0;
extern bool spk_state;
extern bool temp_flag;
extern bool dust_flag;
extern stop_flag;
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
void USART2_IRQHandler(){
	unsigned char c;
	if(USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) != RESET){
		c = USART_ReceiveData(DEBUG_UART);
		if (c=='\n') {
			debug_rx_buffer[debug_rx_buffer_index]=c;
			hc05_send_cmd(debug_rx_buffer);
			debug_clear_buffer();
		}
		else {
			if (debug_rx_buffer_index<DEBUG_BUFFER_SIZE) debug_rx_buffer[debug_rx_buffer_index++]=c;	//Save Data to gps_buffer
			else debug_rx_buffer_index = 0;
		}
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
		if (c=='\n'){
			rx_flag = 0;
//			debug_send_string(hc05_rx_buffer);
//			debug_send_chr('\n');
			hc05_parse_data(hc05_rx_buffer, hc05_rx_index, &hc05_data);
			hc05_clear_buffer();
		}
		if (rx_flag){
			if (hc05_rx_index<HC05_BUFFER_SIZE) hc05_rx_buffer[hc05_rx_index++]=c;
			else hc05_rx_index = 0;
		}
	}
}
void EXTI15_10_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line13)!=RESET)
	{
		debug_send_string("log: STOP by USER\n");
		 EXTI_ClearITPendingBit(EXTI_Line13);
		 stop_flag = true;
		 relay_off();
	}
	if(EXTI_GetITStatus(EXTI_Line14)!=RESET)
	{
		debug_send_string("log: RESET by USER\n");
		 EXTI_ClearITPendingBit(EXTI_Line14);
		 stop_flag= false;
		 relay_on();
	}
	if(EXTI_GetITStatus(EXTI_Line15)!=RESET)
	{
		 EXTI_ClearITPendingBit(EXTI_Line15);
	}
}
