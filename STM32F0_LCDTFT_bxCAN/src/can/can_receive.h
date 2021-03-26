/*
 * can_receive.h
 *
 *  Created on: Aug 20, 2020
 *      Author: manht
 */

#ifndef CAN_RECEIVE_H_
#define CAN_RECEIVE_H_

#include "stdio.h"
#include "stdint.h"
#include "stm32f0xx_can.h"
#include "../src/usart/usart.h"
/*None error*/
#define SMC_ERR_NONE                                                0x00000000

/*Undervolt.*/
#define SMC_ERR_UV                                                  0x00033220

/*Overvoltage*/
#define SMC_ERR_OV                                                  0x00063210

/*Power Module over-temperature*/
#define SMC_ERR_BOARD_OT                                            0x000B4210

/*Current measurement offset*/
#define SMC_ERR_SENSOR_OFFSET                                       0x00115210

/*Overcurrent*/
#define SMC_ERR_OC                                                  0x00122310

/*Hall angle error*/
#define SMC_ERR_HALL_ANGLE                                          0x00137305

/*Hall speed error*/
#define SMC_ERR_HALL_SPEED                                          0x00147303

/*Hall A cable error*/
#define SMC_ERR_HALL_CABLE1                                         0x00157303
/*Hall B cable error*/
#define SMC_ERR_HALL_CABLE2                                         0x00167305
/*Hall C cable error*/
#define SMC_ERR_HALL_CABLE3                                         0x00177305
/*error test*/
#define SMC_ERR_TEST						    					0x00000001

void can_receive_data(void);
void can_send_data(void);

#endif /* CAN_RECEIVE_H_ */
