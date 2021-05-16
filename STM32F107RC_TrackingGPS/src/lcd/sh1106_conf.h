/*
 * sh1106_conf.h
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */

#ifndef _SH1106_CONF_H_
#define _SH1106_CONF_H_

// Choose a microcontroller family
#define STM32F0
//#define STM32F1
//#define STM32F4
//#define STM32L0
//#define STM32L4
//#define STM32F3
//#define STM32H7
//#define STM32F7

// Choose a bus
#define SH1106_USE_I2C
//#define SH1106_USE_SPI

// I2C Configuration
#define SH1106_I2C_PORT        I2C1
#define SH1106_I2C_ADDR        0x3C

// SPI Configuration
//#define SH1106_SPI_PORT        hspi1
//#define SH1106_CS_Port         OLED_CS_GPIO_Port
//#define SH1106_CS_Pin          OLED_CS_Pin
//#define SH1106_DC_Port         OLED_DC_GPIO_Port
//#define SH1106_DC_Pin          OLED_DC_Pin
//#define SH1106_Reset_Port      OLED_Res_GPIO_Port
//#define SH1106_Reset_Pin       OLED_Res_Pin

// Mirror the screen if needed
 #define SH1106_MIRROR_VERT
 #define SH1106_MIRROR_HORIZ

// Set inverse color if needed
// # define SH1106_INVERSE_COLOR

// Include only needed fonts
#define SH1106_INCLUDE_FONT_6x8
#define SH1106_INCLUDE_FONT_7x10
#define SH1106_INCLUDE_FONT_11x18
#define SH1106_INCLUDE_FONT_16x26
#define SH1106_INCLUDE_BITMAP
// Some OLEDs don't display anything in first two columns.
// In this case change the following macro to 130.
// The default value is 128.
 #define SH1106_WIDTH           130

// The height can be changed as well if necessary.
// It can be 32, 64 or 128. The default value is 64.
// #define SH1106_HEIGHT          64


#endif /* LCD_SH1106_CONF_H_ */
