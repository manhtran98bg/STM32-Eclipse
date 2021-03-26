
#include "stm32f0xx_it.h"

volatile uint32_t system_time_stamp=0;
volatile uint32_t msTicks=0;
extern volatile unsigned char y1,y2;
extern volatile unsigned char buffer1[100],buffer2[100];
__IO uint32_t TimingDelay;
volatile uint8_t can_flag = 0, can_mes_cnt=0;
CanRxMsg RxMessage;
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
	system_time_stamp++;
	if(msTicks !=0)
		{
			msTicks--;
		}
}
void USART1_IRQHandler(void)
{
	char c;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		c= (char)USART_ReceiveData(USART1);

		if (y1>100) y1=0;
		if ((y1<100)&&(c>=32)) buffer1[y1++]=c;
	}
}
void USART2_IRQHandler(void)
{
	char c;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		c= (char)USART_ReceiveData(USART2);
		if (y2>100) y2=0;
		if ((y2<100)&&(c>=32))
			{
				USART_SendData(USART2, c);
				buffer2[y2++]=c;
			}
	}
}
static void can_rx_message_prepare()
{
	RxMessage.StdId = 0x00;
    RxMessage.ExtId = 0x00;
    RxMessage.IDE = 0;
    RxMessage.DLC = 4;
    RxMessage.FMI = 0;
    for(int i=0;i<4;i++) RxMessage.Data[i] = 0x00;
}
void CEC_CAN_IRQHandler(void)
{
	if(CAN_GetITStatus(CAN, CAN_IT_FMP0) != RESET)
	{
		can_mes_cnt++;
		can_rx_message_prepare();
		CAN_Receive(CAN, CAN_FIFO0, &RxMessage);
	    can_flag = 1;
	    can_receive_data();
	}
}
