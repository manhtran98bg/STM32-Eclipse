/*
 * i2c.h
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */

#ifndef I2C_I2C_H_
#define I2C_I2C_H_
#include "main.h"

#define I2C1_RCC		RCC_APB1Periph_I2C1
#define I2C_GPIO_RCC	RCC_APB2Periph_GPIOB
#define I2C_GPIO		GPIOB
#define I2C_PIN_SDA		GPIO_Pin_7
#define I2C_PIN_SCL		GPIO_Pin_6

void i2c_init();
void i2c_start(void);
void i2c_stop(void);
void i2c_address_direction(uint8_t address, uint8_t direction);
void i2c_transmit(uint8_t byte);
uint8_t i2c_receive_ack();
uint8_t i2c_receive_nack();
void i2c_write(uint8_t address, uint8_t data);
void i2c_send_array(uint8_t address, uint8_t *buffer, size_t buffer_size);
void i2c_read(uint8_t address, uint8_t* data);
#endif /* I2C_I2C_H_ */
