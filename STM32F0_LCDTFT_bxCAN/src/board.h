#ifndef _BOARD_H_
#define _BOARD_H_

#include "stm32f0xx.h"
#include "stm32f0xx_it.h"
#include "stdbool.h"
#include "../src/ioc/actuator_io_hardware.h"
#include "../src/usart/usart.h"
#include "../src/lcd/stm32_adafruit_lcd.h"
#include "../src/lcd/bmp.h"
#include "../src/lcd/Fonts/fonts.h"
extern const tChar Font_array[];
extern const tChar Font_array1[];
extern const BITMAPSTRUCT dot[];
extern const BITMAPSTRUCT km_distance[];
extern const BITMAPSTRUCT censius[];

extern const BITMAPSTRUCT icon_kmh[];
extern const BITMAPSTRUCT icon_sport[];
extern const BITMAPSTRUCT icon_eco[];
extern const BITMAPSTRUCT icon_far_led[];
extern const BITMAPSTRUCT icon_cos_led[];
extern const BITMAPSTRUCT icon_blank[];
extern const BITMAPSTRUCT icon_pin[];
extern const BITMAPSTRUCT icon_odo[];
extern const BITMAPSTRUCT icon_trip[];
void board_init();
#endif
