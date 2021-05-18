/*
 * spi.c
 *
 *  Created on: May 17, 2021
 *      Author: manht
 */

#include "spi.h"

void spi_init(void){
	GPIO_InitTypeDef gpio_init_struct;
	SPI_InitTypeDef	SPI_InitStruct;
	//Enable SPI & GPIO clock
	RCC_APB1PeriphClockCmd(SD_SPI_RCC, ENABLE);
	RCC_APB2PeriphClockCmd(SD_SPI_GPIO_RCC, ENABLE);

	gpio_init_struct.GPIO_Pin = SD_SPI_MOSI|SD_SPI_SCK;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(SD_SPI_GPIO, &gpio_init_struct);
	gpio_init_struct.GPIO_Pin = SD_SPI_MISO;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SD_SPI_GPIO, &gpio_init_struct);

	gpio_init_struct.GPIO_Pin = SD_SPI_CS;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SD_SPI_GPIO, &gpio_init_struct);

	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SD_SPI, &SPI_InitStruct);
	SPI_Cmd(SD_SPI, ENABLE);
}
uint8_t spi_send_byte(uint8_t data)
{
	while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE)==RESET);
	SPI_I2S_SendData(SD_SPI,data);
	while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE)==RESET);
	return SPI_I2S_ReceiveData(SD_SPI);
}
