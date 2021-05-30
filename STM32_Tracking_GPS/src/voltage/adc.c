/*
 * adc.c
 *
 *  Created on: May 20, 2021
 *      Author: manht
 */

#include "adc.h"
__IO uint16_t ADCConvertedValue;
static void adc_clk_init()
{
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	/* Enable peripheral clocks ------------------------------------------------*/
	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(VOLTAGE_ADC_DMA_RCC, ENABLE);
	/* Enable ADC1 and GPIOC clock */
	RCC_APB2PeriphClockCmd(VOLTAGE_ADC_RCC | VOLTAGE_ADC_GPIO_RCC, ENABLE);
}
static void adc_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = VOLTAGE_ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(VOLTAGE_ADC_GPIO, &GPIO_InitStructure);
}
static void adc_module_init()
{
	ADC_InitTypeDef ADC_InitStructure;
#if _USE_ADC_DMA
	DMA_InitTypeDef DMA_InitStructure;
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
#endif
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(VOLTAGE_ADC, &ADC_InitStructure);
	/* ADC1 regular channel14 configuration */
	ADC_RegularChannelConfig(VOLTAGE_ADC, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5);
	/* Enable ADC1 DMA */
	ADC_DMACmd(VOLTAGE_ADC, ENABLE);
	/* Enable ADC1 */
	ADC_Cmd(VOLTAGE_ADC, ENABLE);
	/* Enable ADC1 reset calibration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
void adc_init()
{
	adc_clk_init();
	adc_gpio_init();
	adc_module_init();
}
void get_vbat(float *vbat){
	float adc_vol = 0;
	__IO int sum_adc_val =  0;
	__IO int adc_val=0;
	while (ADCConvertedValue==0);
	for (int i=0;i<10;i++){
		delay_ms(5);
		sum_adc_val = sum_adc_val + ADCConvertedValue;
	}
	adc_val =  (int) sum_adc_val/10;
	adc_vol =(float)(adc_val/4096.0)*3.23;
	*vbat = (float)adc_vol*11.0;
	return ;
}
