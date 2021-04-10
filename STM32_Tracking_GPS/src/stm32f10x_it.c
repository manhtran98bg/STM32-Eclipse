
#include "stm32f10x_it.h"

volatile uint32_t msTicks=0;
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
