#include "board.h"
static void lcd_draw_init_screen()
{
	BSP_LCD_Init();
	BSP_LCD_DrawRect(0, 0, 480, 320, 2);
	for (int i=0;i<2;i++)
	{
		BSP_LCD_DrawHLine(0, 55+i, 480);
	}
	for (int i=0;i<2;i++)
	{
		BSP_LCD_DrawHLine(0, 255+i, 480);
	}
	BSP_LCD_DrawBitmap_Grayscale4(325, 180, (uint8_t *)icon_kmh);
	BSP_LCD_DrawBitmap_Grayscale4(170, 105, (uint8_t *)Font_array[0].image);
	BSP_LCD_DrawBitmap_Grayscale4(245, 105, (uint8_t *)Font_array[0].image);
	BSP_LCD_DrawBitmap_Grayscale4(55, 260, (uint8_t *)icon_pin);
	BSP_LCD_DrawBitmap_Grayscale4(210, 260, (uint8_t *)icon_pin);
	BSP_LCD_DrawBitmap_Grayscale4(360, 260, (uint8_t *)icon_pin);
	BSP_LCD_DrawBitmap_Grayscale4(50, 9, (uint8_t *)icon_odo);
	BSP_LCD_DrawBitmap_Grayscale4(375, 9, (uint8_t *)icon_trip);
	BSP_LCD_DrawBitmap_Grayscale4(80, 180, (uint8_t *)censius);
}
void board_init(){
	SystemInit();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority(SysTick_IRQn, 3);
	actuator_io_hardware_init();
	can_hardware_init();
//	USART_Config();
	lcd_draw_init_screen();

}
