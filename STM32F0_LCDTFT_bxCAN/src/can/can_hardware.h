
#ifndef BOARD_CAN_HARDWARE_H_
#define BOARD_CAN_HARDWARE_H_

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_can.h"


#define CAN_DEV_CLK				RCC_APB1Periph_CAN

#define CAN_TX_PIN				GPIO_Pin_12
#define CAN_TX_PORT				GPIOA
#define CAN_TX_PORT_CLK			RCC_AHBPeriph_GPIOA
#define CAN_TX_PINSOURCE		GPIO_PinSource12

#define CAN_RX_PIN				GPIO_Pin_11
#define CAN_RX_PORT				GPIOA
#define CAN_RX_PORT_CLK			RCC_AHBPeriph_GPIOA
#define CAN_RX_PINSOURCE		GPIO_PinSource11

//#define CAN_STANDBY_PIN			GPIO_Pin_7
//#define CAN_STANDBY_PORT		GPIOB
//#define CAN_STANDBY_PORT_CLK	RCC_AHBPeriph_GPIOB

#define CAN_PORT				GPIOA
#define CAN_HANDLE				CEC_CAN_IRQHandler

#define CAN_IRQN				CEC_CAN_IRQn
#define CAN_IRQN_PRIORITY		0
#define CAN_PRE_DEFAULT			6

typedef void (*CAN_Receive_Handle)(CanRxMsg* p_msg);
void can_hardware_init(void);
void can_set_receive_handle(CAN_Receive_Handle handle);
void can_set_baudrate(uint32_t baud_rate);
void can_error_handle(void);

#endif /* BOARD_CAN_HARDWARE_H_ */
