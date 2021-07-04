/*
 * board.c
 *
 *  Created on: Jul 1, 2021
 *      Author: manht
 */
#include "board.h"
#include "service/delay.h"
void clock_config(){
	RCC_DeInit();
	RCC_HSICmd(DISABLE);
	RCC_HSEConfig(RCC_HSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY)==RESET);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_PLLCmd(DISABLE);
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource()!=0x08);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
}
void user_led_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef gpio_init_struct;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init_struct.GPIO_Pin = USER_LED_PIN;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOC, &gpio_init_struct);
}
void user_led_toogle(){
	GPIOC->ODR ^=USER_LED_PIN;
}
void output_pin_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = BUZZER_PIN|RELAY_PIN;
	gpio_init.GPIO_Mode =  GPIO_Mode_Out_PP;
	gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &gpio_init);
}
void buzzer_on(){
	GPIOA->ODR |=BUZZER_PIN;
}
void buzzer_beep(uint32_t duration){
	GPIOA->ODR |=BUZZER_PIN;
	delay_ms(duration);
	GPIOA->ODR &=~BUZZER_PIN;
}
void buzzer_off(){
	GPIOA->ODR &=~BUZZER_PIN;
}
void relay_on(){

	GPIO_ResetBits(GPIOA, RELAY_PIN);
}
void relay_off(){
	GPIO_SetBits(GPIOA, RELAY_PIN);
}
void btn_config(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitTypeDef gpio_struct;
	EXTI_InitTypeDef exti_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	gpio_struct.GPIO_Pin = BTN1_PIN|BTN2_PIN|BTN3_PIN;
	gpio_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_struct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BTN_GPIO, &gpio_struct);
	/* Connect EXTI13 Line to PB13 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	/* Connect EXTI14 Line to PB14 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	/* Connect EXTI14 Line to PB15 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);
	/* Configure EXTI line */
	exti_init_struct.EXTI_Line = EXTI_Line13|EXTI_Line14|EXTI_Line14|EXTI_Line15;
	exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_struct.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init_struct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init_struct);
	nvic_init_struct.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);
}

