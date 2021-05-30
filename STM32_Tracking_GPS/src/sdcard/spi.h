/*
 * spi.h
 *
 *  Created on: May 17, 2021
 *      Author: manht
 */

#ifndef SDCARD_SPI_H_
#define SDCARD_SPI_H_

#include "../main.h"
#include "../service/delay.h"

#define SD_SPI			SPI3
#define SD_SPI_RCC		RCC_APB1Periph_SPI3
#define SD_SPI_GPIO		GPIOB
#define SD_SPI_GPIO_RCC	RCC_APB2Periph_GPIOB

#define SD_SPI_MOSI		GPIO_Pin_5
#define SD_SPI_MISO		GPIO_Pin_4
#define SD_SPI_SCK		GPIO_Pin_3
#define SD_SPI_CS		GPIO_Pin_1

void spi_init(void);
uint8_t spi_send_byte(uint8_t data);
#endif /* SDCARD_SPI_H_ */
