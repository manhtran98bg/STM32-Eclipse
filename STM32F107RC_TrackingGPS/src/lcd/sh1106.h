/*
 * sh1106.h
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */
#ifndef _SH1106_H_
#define _SH1106_H_

#include "main.h"
#include "sh1106_conf.h"
#include "sh1106_font.h"
#include "../i2c/i2c.h"
// SH1106 OLED height in pixels
#ifndef SH1106_HEIGHT
#define SH1106_HEIGHT          64
#endif

// SH1106 width in pixels
#ifndef SH1106_WIDTH
#define SH1106_WIDTH           128
#endif

#ifndef SH1106_BUFFER_SIZE
#define SH1106_BUFFER_SIZE   SH1106_WIDTH * SH1106_HEIGHT / 8
#endif

typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} SH1106_COLOR;

typedef enum {
	SH1106_OK = 0x00,
	SH1106_ERR = 0x01  // Generic error.
} SH1106_Error_t;

// Struct to store transformations
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
    uint8_t DisplayOn;
} SH1106_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} SH1106_VERTEX;
// Procedure definitions
void sh1106_Init(void);
void sh1106_Fill(SH1106_COLOR color);
void sh1106_UpdateScreen(void);
void sh1106_DrawPixel(uint8_t x, uint8_t y, SH1106_COLOR color);
char sh1106_WriteChar(char ch, FontDef Font, SH1106_COLOR color);
char sh1106_WriteString(char* str, FontDef Font, SH1106_COLOR color);
void sh1106_SetCursor(uint8_t x, uint8_t y);
void sh1106_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color);
void sh1106_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SH1106_COLOR color);
void sh1106_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, SH1106_COLOR color);
void sh1106_Polyline(const SH1106_VERTEX *par_vertex, uint16_t par_size, SH1106_COLOR color);
void sh1106_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color);
/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */
void sh1106_SetContrast(const uint8_t value);
/**
 * @brief Set Display ON/OFF.
 * @param[in] on 0 for OFF, any for ON.
 */
void sh1106_SetDisplayOn(const uint8_t on);
/**
 * @brief Reads DisplayOn state.
 * @return  0: OFF.
 *          1: ON.
 */
uint8_t sh1106_GetDisplayOn();

// Low-level procedures
void sh1106_Reset(void);
void sh1106_WriteCommand(uint8_t byte);
void sh1106_WriteData(uint8_t* buffer, size_t buff_size);
SH1106_Error_t ssd1306_FillBuffer(uint8_t* buf, uint32_t len);

#endif // __SSD1306_H__
