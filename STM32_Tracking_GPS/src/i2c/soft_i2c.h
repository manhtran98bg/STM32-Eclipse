/*
 * soft_i2c.h
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */

#ifndef I2C_SOFT_I2C_H_
#define I2C_SOFT_I2C_H_

#include "../main.h"
#include "../service/delay.h"

#define SI2C_GPIO	GPIOB

#define SI2C_GPIO_RCC	RCC_APB2Periph_GPIOB
#define SI2C_GPIO_SDA	GPIO_Pin_6
#define SI2C_GPIO_SCL	GPIO_Pin_7

#define SDA_0 GPIO_ResetBits(SI2C_GPIO, SI2C_GPIO_SDA)
#define SDA_1 GPIO_SetBits(SI2C_GPIO, SI2C_GPIO_SDA)
#define SCL_0 GPIO_ResetBits(SI2C_GPIO, SI2C_GPIO_SCL)
#define SCL_1 GPIO_SetBits(SI2C_GPIO, SI2C_GPIO_SCL)
#define SDA_VAL (GPIO_ReadInputDataBit(SI2C_GPIO, SI2C_GPIO_SDA))

void soft_i2c_init(void);
void soft_i2c_start(void);
void soft_i2c_stop(void);
uint8_t soft_i2c_write(uint8_t u8Data);
uint8_t soft_i2c_read(uint8_t u8Ack);

#endif /* I2C_SOFT_I2C_H_ */
