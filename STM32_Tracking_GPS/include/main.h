/*
 * main.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "../src/sdcard/ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "diag/Trace.h"
#include "stm32f10x.h"
#include "port_pin.h"
#include "stm32f10x_conf.h"
#include "float.h"
#include <time.h>
#define _DEBUG	1
#define _DEBUG_UART5 1
#define _STM32F103RCT6_
extern FATFS	FatFs;
extern FIL	Fil;

#endif /* MAIN_H_ */
