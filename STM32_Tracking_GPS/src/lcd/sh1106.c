/*
 * sh1106.c
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */
#include "sh1106.h"

#if defined(SH1106_USE_I2C)

//func for I2C protocol

void sh1106_Reset(void) {
    /* for I2C - do nothing */
}

// Send a byte to the command register
void sh1106_WriteCommand(uint8_t byte) {
    i2c_start();
    i2c_address_direction(SH1106_I2C_ADDR  << 1, I2C_Direction_Transmitter);
    i2c_transmit(0x00);
    i2c_transmit(byte);
    i2c_stop();
}
// Send data
void sh1106_WriteData(uint8_t* buffer, size_t buff_size) {
	size_t i=0;
	i2c_start();
    i2c_address_direction(SH1106_I2C_ADDR  << 1, I2C_Direction_Transmitter);
    i2c_transmit(0x40);
    while(i<buff_size)
    {
    	i2c_transmit(*(buffer+i));
    	i++;
    }
    i2c_stop();
}
#endif
// Screenbuffer
static uint8_t SH1106_Buffer[SH1106_BUFFER_SIZE];
// Screen object
static SH1106_t SH1106;
/* Fills the Screenbuffer with values from a given buffer of a fixed length */
SH1106_Error_t sh1106_FillBuffer(uint8_t* buf, uint32_t len) {
	SH1106_Error_t ret = SH1106_ERR;
    if (len <= SH1106_BUFFER_SIZE) {
        memcpy(SH1106_Buffer,buf,len);
        ret = SH1106_OK;
    }
    return ret;
}
void sh1106_SetDisplayOn(const uint8_t on) {
    uint8_t value;
    if (on) {
        value = 0xAF;   // Display on
        SH1106.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SH1106.DisplayOn = 0;
    }
    sh1106_WriteCommand(value);
}
void sh1106_SetContrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    sh1106_WriteCommand(kSetContrastControlRegister);
    sh1106_WriteCommand(value);
}
// Fill the whole screen with the given color
void sh1106_Fill(SH1106_COLOR color) {
    /* Set memory */
    uint32_t i;

    for(i = 0; i < sizeof(SH1106_Buffer); i++) {
    	SH1106_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}
// Write the screenbuffer with changed to the screen
void sh1106_UpdateScreen(void) {
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < SH1106_HEIGHT/8; i++) {
    	sh1106_WriteCommand(0xB0 + i); // Set the current RAM page address.
    	sh1106_WriteCommand(0x00);
    	sh1106_WriteCommand(0x10);
        sh1106_WriteData(&SH1106_Buffer[SH1106_WIDTH*i],SH1106_WIDTH);
    }
}
// Initialize the oled screen
void sh1106_Init(void) {
	i2c_init();
    // Reset OLED
    sh1106_Reset();

    // Wait for the screen to boot
    delay_ms(100);

    // Init OLED
    sh1106_SetDisplayOn(0); //display off

    sh1106_WriteCommand(0x20); //Set Memory Addressing Mode
    sh1106_WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid

    sh1106_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

#ifdef SH1106_MIRROR_VERT
    sh1106_WriteCommand(0xC0); // Mirror vertically
#else
    sh1106_WriteCommand(0xC8); //Set COM Output Scan Direction
#endif

    sh1106_WriteCommand(0x00); //---set low column address
    sh1106_WriteCommand(0x10); //---set high column address

    sh1106_WriteCommand(0x40); //--set start line address - CHECK

    sh1106_SetContrast(0xFF);

#ifdef SH1106_MIRROR_HORIZ
    sh1106_WriteCommand(0xA0); // Mirror horizontally
#else
    sh1106_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SH1106_INVERSE_COLOR
    sh1106_WriteCommand(0xA7); //--set inverse color
#else
    sh1106_WriteCommand(0xA6); //--set normal color
#endif

// Set multiplex ratio.
#if (SH1106_HEIGHT == 128)
    // Found in the Luma Python lib for SH1106.
    sh1106_WriteCommand(0xFF);
#else
    sh1106_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#endif

#if (SH1106_HEIGHT == 32)
    sh1106_WriteCommand(0x1F); //
#elif (SH1106_HEIGHT == 64)
    sh1106_WriteCommand(0x3F); //
#elif (SH1106_HEIGHT == 128)
    sh1106_WriteCommand(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    sh1106_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    sh1106_WriteCommand(0xD3); //-set display offset - CHECK
    sh1106_WriteCommand(0x00); //-not offset

    sh1106_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
    sh1106_WriteCommand(0xF0); //--set divide ratio

    sh1106_WriteCommand(0xD9); //--set pre-charge period
    sh1106_WriteCommand(0x22); //

    sh1106_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
#if (SH1106_HEIGHT == 32)
    sh1106_WriteCommand(0x02);
#elif (SH1106_HEIGHT == 64)
    sh1106_WriteCommand(0x12);
#elif (SH1106_HEIGHT == 128)
    sh1106_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    sh1106_WriteCommand(0xDB); //--set vcomh
    sh1106_WriteCommand(0x20); //0x20,0.77xVcc

    sh1106_WriteCommand(0x8D); //--set DC-DC enable
    sh1106_WriteCommand(0x14); //
    sh1106_SetDisplayOn(1); //--turn on SSD1306 panel

    // Clear screen
    sh1106_Fill(Black);

    // Flush buffer to screen
    sh1106_UpdateScreen();

    // Set default values for screen object
    SH1106.CurrentX = 0;
    SH1106.CurrentY = 0;
    SH1106.Initialized = 1;
}
//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void sh1106_DrawPixel(uint8_t x, uint8_t y, SH1106_COLOR color) {
    if(x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if(SH1106.Inverted) {
        color = (SH1106_COLOR)!color;
    }

    // Draw in the right color
    if(color == White) {
    	SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] |= 1 << (y % 8);
    } else {
    	SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
    }
}
// Draw 1 char to the screen buffer
// ch       => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color    => Black or White
char sh1106_WriteChar(uint8_t x, uint8_t y, char ch, FontDef Font, SH1106_COLOR color) {
    uint32_t i, b, j;

    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;
    SH1106.CurrentX = x;
    SH1106.CurrentY = y;
    // Check remaining space on current line
    if (SH1106_WIDTH < (SH1106.CurrentX + Font.FontWidth) ||
    		SH1106_HEIGHT < (SH1106.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }
    // Use the font to write
    for(i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for(j = 0; j < Font.FontWidth; j++) {
            if((b << j) & 0x8000)  {
                sh1106_DrawPixel(SH1106.CurrentX + j, (SH1106.CurrentY + i), (SH1106_COLOR) color);
            } else {
            	sh1106_DrawPixel(SH1106.CurrentX + j, (SH1106.CurrentY + i), (SH1106_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SH1106.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}
// Write full string to screenbuffer
char sh1106_WriteString(uint8_t x, uint8_t y, char* str, FontDef Font, SH1106_COLOR color, Align_t Align) {
    switch (Align) {
		case NO_ALIGN:
			SH1106.CurrentX = x;
			SH1106.CurrentY = y;
			break;
		case ALIGN_LEFT:
			SH1106.CurrentX = 2;
			SH1106.CurrentY = y;
			break;
		case ALIGN_RIGHT:
			SH1106.CurrentX = 130-Font.FontWidth*strlen(str);
			SH1106.CurrentY = y;
			break;
		case ALIGN_CENTER:
			SH1106.CurrentX = (uint8_t)(130-Font.FontWidth*strlen(str))/2;
			SH1106.CurrentY = y;
			break;
		default:
			break;
	}
    // Write until null-byte
    while (*str) {
        if (sh1106_WriteChar(SH1106.CurrentX, SH1106.CurrentY, *str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}
// Position the cursor
void sh1106_SetCursor(uint8_t x, uint8_t y) {
	SH1106.CurrentX = x;
	SH1106.CurrentY = y;
}
// Draw line by Bresenhem's algorithm
void sh1106_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SH1106_COLOR color) {
  int32_t deltaX = abs(x2 - x1);
  int32_t deltaY = abs(y2 - y1);
  int32_t signX = ((x1 < x2) ? 1 : -1);
  int32_t signY = ((y1 < y2) ? 1 : -1);
  int32_t error = deltaX - deltaY;
  int32_t error2;

  sh1106_DrawPixel(x2, y2, color);
    while((x1 != x2) || (y1 != y2))
    {
    	sh1106_DrawPixel(x1, y1, color);
    	error2 = error * 2;
    	if(error2 > -deltaY)
    	{
    		error -= deltaY;
    		x1 += signX;
    	}
    	else
    	{
    		/*nothing to do*/
    	}

    	if(error2 < deltaX)
    	{
    		error += deltaX;
    		y1 += signY;
    	}
    	else
    	{
    		/*nothing to do*/
    	}
    }
  return;
}
void sh1106_DrawBitmap(uint8_t x, uint8_t y, BitmapDef bitmap)
{
	SH1106.CurrentX = x;
	SH1106.CurrentY = y;
	__IO uint8_t ptr;
	uint8_t b;
	int col=0, row = 1,i=0;
	if ((SH1106.CurrentX+bitmap.BitmapWidth)>SH1106_WIDTH || (SH1106.CurrentY+bitmap.BitmapHeight)>SH1106_HEIGHT)
		return;
	b = bitmap.BitmapWidth/8;
	for (row=0;row<bitmap.BitmapHeight;row++)	//Scan N row.
	{
		for(col=0;col<b;col++)	//Scan one row.
		{
			ptr = bitmap.data[row+col];
			for ( i = 0;i<8;i++){
				if ((bitmap.data[row*b+col]<<i) & 0x80) {
					sh1106_DrawPixel(SH1106.CurrentX, SH1106.CurrentY, (SH1106_COLOR) White);
				}
				else  {
					sh1106_DrawPixel(SH1106.CurrentX, SH1106.CurrentY, (SH1106_COLOR) Black);
				}
				SH1106.CurrentX++;
			}
		}
		SH1106.CurrentX = x;
		SH1106.CurrentY++;
	}
}
void sh1106_Clear(SH1106_COLOR color)
{
	sh1106_Fill(color);
	sh1106_UpdateScreen();
	sh1106_SetCursor(2, 0);
}
