/*
 * sh1106_font.h
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */

#ifndef SH1106_FONT_H_
#define SH1106_FONT_H_

#include "sh1106_conf.h"
#include <stdint.h>
typedef struct {
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;
#ifdef SH1106_INCLUDE_FONT_6x8
extern FontDef Font_6x8;
#endif
#ifdef SH1106_INCLUDE_FONT_7x10
extern FontDef Font_7x10;
#endif
#ifdef SH1106_INCLUDE_FONT_11x18
extern FontDef Font_11x18;
#endif
#ifdef SH1106_INCLUDE_FONT_16x26
extern FontDef Font_16x26;
#endif

#endif /* SH1106_FONT_H_ */
