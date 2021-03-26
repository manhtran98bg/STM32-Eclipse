/*
 * lcd_io_gpio81.h
 *
 *  Created on: Aug 11, 2020
 *      Author: manht
 */

#ifndef STM32F0XX_LCD_IO_GPIO8_H_
#define STM32F0XX_LCD_IO_GPIO8_H_
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
/*
 * 8 bit paralell LCD GPIO driver for STM32F0
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + backlight pin
 */

//=============================================================================
/* Lcd controll pins assign (A..K, 0..15) */
#define LCD_CS            A, 8
#define LCD_RS            C, 9
#define LCD_WR            C, 8
#define LCD_RD            C, 7  /* If not used leave it that way */
#define LCD_RST           A, 9  /* If not used leave it that way */

/* Lcd data pins assign (A..K, 0..15) */
#define LCD_D0            A, 2
#define LCD_D1            A, 3
#define LCD_D2            C, 0
#define LCD_D3            C, 1
#define LCD_D4            C, 2
#define LCD_D5            C, 3
#define LCD_D6            A, 0
#define LCD_D7            A, 1

/* Backlight control
   - BL: A..K, 0..15 (if not used -> X, 0)
   - BL_ON: 0 = active low level, 1 = active high level */
#define LCD_BL            X, 0  /* If not used leave it that way */
#define LCD_BLON          0

/* wait time for LCD write and read pulse
   - First, give 10, 20 values, then lower them to speed up the program.
     (values also depend on processor speed and LCD display speed) */
#define LCD_WRITE_DELAY  1
#define LCD_READ_DELAY   1

/*=============================================================================
I/O group optimization so that GPIO operations are not performed bit by bit:
Note: If the pins are in order, they will automatically optimize.
The example belongs to the following pins:
      LCD_D0<-D14, LCD_D1<-D15, LCD_D2<-D0, LCD_D3<-D1
      LCD_D4<-E7,  LCD_D5<-E8,  LCD_D6<-E9, LCD_D7<-E10 */
#if 0
/* datapins setting to output (data direction: STM32 -> LCD) */
#define LCD_DIRWRITE { /* D0..D1, D14..D15, E7..E10 <- 0b01 */ \
GPIOD->MODER = (GPIOD->MODER & ~0b11110000000000000000000000001111) | 0b01010000000000000000000000000101; \
GPIOE->MODER = (GPIOE->MODER & ~0b00000000001111111100000000000000) | 0b00000000000101010100000000000000; }
/* datapins setting to input (data direction: STM32 <- LCD) */
#define LCD_DIRREAD { /* D0..D1, D14..D15, E7..E10 <- 0b00 */ \
GPIOD->MODER = (GPIOD->MODER & ~0b11110000000000000000000000001111); \
GPIOE->MODER = (GPIOE->MODER & ~0b00000000001111111100000000000000); }
/* datapins write, STM32 -> LCD (write I/O pins from dt data) */
#define LCD_WRITE(dt) { /* D14..15 <- dt0..1, D0..1 <- dt2..3, E7..10 <- dt4..7 */ \
GPIOD->ODR = (GPIOD->ODR & ~0b1100000000000011) | (((dt & 0b00000011) << 14) | ((dt & 0b00001100) >> 2)); \
GPIOE->ODR = (GPIOE->ODR & ~0b0000011110000000) | ((dt & 0b11110000) << 3); }
/* datapins read, STM32 <- LCD (read from I/O pins and store to dt data) */
#define LCD_READ(dt) { /* dt0..1 <- D14..15, dt2..3 <- D0..1, dt4..7 <- E7..10 */ \
dt = ((GPIOD->IDR & 0b1100000000000000) >> 14) | ((GPIOD->IDR & 0b0000000000000011) << 2) | \
     ((GPIOE->IDR & 0b0000011110000000) >> 3); }
/* Note: the keil compiler cannot use binary numbers, convert it to hexadecimal */
#endif

/* Link function for LCD peripheral */
void     LCD_Delay (uint32_t delay);
void     LCD_IO_Init(void);
void     LCD_IO_Bl_OnOff(uint8_t Bl);

void     LCD_IO_WriteCmd8(uint8_t Cmd);
void     LCD_IO_WriteCmd16(uint16_t Cmd);
void     LCD_IO_WriteData8(uint8_t Data);
void     LCD_IO_WriteData16(uint16_t Data);

void     LCD_IO_WriteCmd8DataFill16(uint8_t Cmd, uint16_t Data, uint32_t Size);
void     LCD_IO_WriteCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size);
void 	 LCD_IO_WriteCmd8MultipleData16_RGB565(uint8_t Cmd, uint16_t *pData, uint32_t Size);
void     LCD_IO_WriteCmd8MultipleData16_Monochrome(uint8_t Cmd, uint16_t *pData, uint32_t Size);
void     LCD_IO_WriteCmd16DataFill16(uint16_t Cmd, uint16_t Data, uint32_t Size);
void     LCD_IO_WriteCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size);
void     LCD_IO_WriteCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size);

void     LCD_IO_ReadCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd8MultipleData24to16(uint8_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);
void     LCD_IO_ReadCmd16MultipleData24to16(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize);

#endif /* STM32F0XX_LCD_IO_GPIO8_H_ */
