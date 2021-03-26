
#ifndef __STM32F0XX_IT_H
#define __STM32F0XX_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "../src/usart/usart.h"
#include "../src/can/can_hardware.h"
#include "../src/can/can_receive.h"
#include "stdlib.h"
extern volatile uint32_t system_time_stamp;
extern volatile uint32_t msTicks;


void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void USART1_IRQHandler(void);
void CEC_CAN_IRQHandler(void);
void USART2_IRQHandler(void);

/*void PPP_IRQHandler(void);*/

#ifdef __cplusplus
}
#endif

#endif /* __STM32F0XX_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
