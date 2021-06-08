/*
 * adc.h
 *
 *  Created on: May 20, 2021
 *      Author: manht
 */

#ifndef VOLTAGE_ADC_H_
#define VOLTAGE_ADC_H_
#include "../main.h"
#include "../service/delay.h"

#define VOLTAGE_ADC				ADC1
#define VOLTAGE_ADC_RCC			RCC_APB2Periph_ADC1
#define VOLTAGE_ADC_GPIO		GPIOC
#define VOLTAGE_ADC_GPIO_RCC	RCC_APB2Periph_GPIOC
#define VOLTAGE_ADC_PIN			GPIO_Pin_1

#define VOLTAGE_ADC_DMA		DMA1
#define VOLTAGE_ADC_DMA_RCC	RCC_AHBPeriph_DMA1
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define _USE_ADC_DMA	1


extern __IO uint16_t ADCConvertedValue;

void adc_init();
void get_vbat(float *vbat);
#endif /* VOLTAGE_ADC_H_ */
