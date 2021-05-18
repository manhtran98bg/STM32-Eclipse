/*
 * bitmap.h
 *
 *  Created on: May 10, 2021
 *      Author: manht
 */
#include "sh1106_conf.h"
#include <stdint.h>
typedef struct {
	uint8_t BitmapWidth;    /*!< Bitmap width in pixels */
	uint8_t BitmapHeight;   /*!< Bitmap height in pixels */
	const uint8_t *data; /*!< Pointer to data font data array */
} BitmapDef;
#ifdef SH1106_INCLUDE_BITMAP
extern BitmapDef Screen;
extern BitmapDef Gsm_signal[];
extern BitmapDef Gps_signal[];
extern BitmapDef Server_connect[];
extern BitmapDef Gps_logo;
#endif

