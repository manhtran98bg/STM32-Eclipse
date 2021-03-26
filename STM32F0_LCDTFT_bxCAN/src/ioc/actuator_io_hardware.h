#ifndef ACTUATOR_IO_H_
#define ACTUATOR_IO_H_

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"

#define MAX_APP_OUTPUT_NUM					11
#define MAX_APP_INPUT_NUM					10


//  OUPUT
#define FAR_LIGHT_CTRL_PIN   			GPIO_Pin_12
#define FAR_LIGHT_CTRL_PORT   			GPIOC

#define COS_LIGHT_CTRL_PIN  			GPIO_Pin_2
#define COS_LIGHT_CTRL_PORT  			GPIOD

#define TURN_L_LIGHT_CTRL_PIN 			GPIO_Pin_5
#define TURN_L_LIGHT_CTRL_PORT 			GPIOB

#define TURN_R_LIGHT_CTRL_PIN 			GPIO_Pin_4
#define TURN_R_LIGHT_CTRL_PORT 			GPIOB

#define HORN_CTRL_PIN         			GPIO_Pin_9
#define HORN_CTRL_PORT         			GPIOB

#define BRAKE_LIGHT_CTRL_PIN			GPIO_Pin_8
#define BRAKE_LIGHT_CTRL_PORT			GPIOB

#define BACK_LIGHT_CTRL_PIN				GPIO_Pin_3
#define BACK_LIGHT_CTRL_PORT			GPIOB

/* led Signal left Pin */
#define LED_CTRL1_PIN					GPIO_Pin_10
#define LED_CTRL1_PORT					GPIOA
/* led Signal right Pin */
#define LED_CTRL2_PIN					GPIO_Pin_10
#define LED_CTRL2_PORT					GPIOC


#define ECO_MODE_CTRL_PIN				GPIO_Pin_15
#define ECO_MODE_CTRL_PORT				GPIOA

#define SPORT_MODE_CTRL_PIN				GPIO_Pin_13
#define SPORT_MODE_CTRL_PORT			GPIOC


//#define VIB_MOTOR_CTRL_PIN				GPIO_Pin_2
//#define VIB_MOTOR_CTRL_PORT				GPIOB

// INPUT
#define FAR_LIGHT_BUF_PIN   			GPIO_Pin_0
#define FAR_LIGHT_BUF_PORT   			GPIOB

#define COS_LIGHT_BUF_PIN  				GPIO_Pin_1
#define COS_LIGHT_BUF_PORT  			GPIOB

#define TURN_L_LIGHT_BUF_PIN 			GPIO_Pin_10
#define TURN_L_LIGHT_BUF_PORT 			GPIOB

#define TURN_R_LIGHT_BUF_PIN 			GPIO_Pin_11
#define TURN_R_LIGHT_BUF_PORT 			GPIOB

#define HORN_BUF_PIN         			GPIO_Pin_6
#define HORN_BUF_PORT         			GPIOC

#define BRAKE_LIGHT_BUF_PIN				GPIO_Pin_2
#define BRAKE_LIGHT_BUF_PORT			GPIOB

#define H_LIGHT_BUF_PIN         		GPIO_Pin_12
#define H_LIGHT_BUF_PORT         		GPIOB

#define ECO_MODE_BUF_PIN				GPIO_Pin_14
#define ECO_MODE_BUF_PORT				GPIOB

#define SPORT_MODE_BUF_PIN				GPIO_Pin_15
#define SPORT_MODE_BUF_PORT				GPIOB

#define PUSH_START_STOP_SYSTEM_BUF_PIN	GPIO_Pin_13
#define PUSH_START_STOP_SYSTEM_BUF_PORT	GPIOB

//#define KICK_STAND_BUF_PIN				GPIO_Pin_1
//#define KICK_STAND_BUF_PORT				GPIOC


#define PORTA_CLK                       RCC_AHBPeriph_GPIOA
#define PORTB_CLK                       RCC_AHBPeriph_GPIOB
#define PORTC_CLK                       RCC_AHBPeriph_GPIOC
#define PORTD_CLK                       RCC_AHBPeriph_GPIOD
#define PORTE_CLK                       RCC_AHBPeriph_GPIOE
#define PORTF_CLK                       RCC_AHBPeriph_GPIOF




// DEFINE FOR INDEX FOR OUTPUT_PINS
#define FAR_LIGHT_CTRL_INDEX				0
#define COS_LIGHT_CTRL_INDEX				1
#define	TURN_L_LIGHT_CTRL_INDEX				2
#define	TURN_R_LIGHT_CTRL_INDEX				3
#define	HORN_CTRL_INDEX					    4
#define	BRAKE_LIGHT_CTRL_INDEX				5
#define	BACK_LIGHT_CTRL_INDEX				6
#define	ECO_MODE_CTRL_INDEX					7
#define	SPORT_MODE_CTRL_INDEX				8
#define VIB_MOTOR_CTRL_INDEX				9
#define	LED_CTRL1_INDEX						10
#define LED_CTRL2_INDEX						11

// DEFINE FOR INDEX FOR INPUT_PINS
#define FAR_LIGHT_BUF_INDEX					0
#define COS_LIGHT_BUF_INDEX					1
#define	TURN_L_LIGHT_BUF_INDEX				2
#define	TURN_R_LIGHT_BUF_INDEX				3
#define	HORN_BUF_INDEX					    4
#define	BRAKE_BUF_INDEX						5
#define	H_LIGHT_BUF_INDEX					6
#define ECO_MODE_BUF_INDEX					7
#define SPORT_MODE_BUF_INDEX				8
#define START_BUF_INDEX						9

#define IOC_TIM_DEV						    TIM2
#define IOC_TIM_PRESCALER1					2499
#define IOC_TIM_PERIOD1						9999
#define IOC_TIM_PRESCALER2					3
#define IOC_TIM_PERIOD2						299
#define IOC_TIM_DEV_CLK						RCC_APB1Periph_TIM2
#define IOC_TIM_HANDLE			  			TIM2_IRQHandler
#define IOC_TIM_HANDLE_PRIORITY				2


extern volatile uint8_t ioc_tick ;
void actuator_io_hardware_init();
uint8_t read_gpio_in(unsigned char );
void set_gpio_out(unsigned char);
void clear_gpio_out(unsigned char);
void reserve_gpio_out(unsigned char);
void ioc_tim_enable(void);
void ioc_tim_disable(void);
void ioc_tim_start_ver2(void);


#endif
