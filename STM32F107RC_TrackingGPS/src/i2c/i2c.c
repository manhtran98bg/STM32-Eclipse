/*
 * i2c.c
 *
 *  Created on: May 9, 2021
 *      Author: manht
 */

#ifndef I2C_I2C_C_
#define I2C_I2C_C_

#include "i2c.h"
void i2c_init()
{
	// Initialization struct
	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef	GPIO_InitStruct;
	// Initialize I2C
	RCC_APB1PeriphClockCmd(I2C1_RCC, ENABLE);
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C_InitStruct);
	I2C_Cmd(I2C1, ENABLE);
	// Initialize GPIO as open drain alternate function
	GPIO_InitStruct.GPIO_Pin = I2C_PIN_SDA|I2C_PIN_SCL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct);
}
void i2c_start()
{
	// Wait until I2Cx is not busy anymore
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    // Generate start condition
    I2C_GenerateSTART(I2C1, ENABLE);
    // Wait for I2C EV5.
    // It means that the start condition has been correctly released
    // on the I2C bus (the bus is free, no other devices is communicating))
    while (!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
}
void i2c_stop()
{
    // Generate I2C stop condition
    I2C_GenerateSTOP(I2C1, ENABLE);
    // Wait until I2C stop condition is finished
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF));
}
void i2c_address_direction(uint8_t address, uint8_t direction)
{
    // Send slave address
    I2C_Send7bitAddress(I2C1, address, direction);

    // Wait for I2C EV6
    // It means that a slave acknowledges his address
    if (direction == I2C_Direction_Transmitter)
    {
        while (!I2C_CheckEvent(I2C1,
        		I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    }
    else if (direction == I2C_Direction_Receiver)
    {
        while (!I2C_CheckEvent(I2C1,
        		I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}
void i2c_transmit(uint8_t byte)
{
    // Send data byte
    I2C_SendData(I2C1, byte);
    // Wait for I2C EV8_2.
    // It means that the data has been physically shifted out and
    // output on the bus)
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}
uint8_t i2c_receive_ack()
{
    // Enable ACK of received data
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    // Wait for I2C EV7
    // It means that the data has been received in I2C data register
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

    // Read and return data byte from I2C data register
    return I2C_ReceiveData(I2C1);
}
uint8_t i2c_receive_nack()
{
    // Disable ACK of received data
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    // Wait for I2C EV7
    // It means that the data has been received in I2C data register
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

    // Read and return data byte from I2C data register
    return I2C_ReceiveData(I2C1);
}
void i2c_write(uint8_t address, uint8_t data)
{
    i2c_start();
    i2c_address_direction(address << 1, I2C_Direction_Transmitter);
    i2c_transmit(data);
    i2c_stop();
}
void i2c_send_array(uint8_t address, uint8_t *buffer, size_t buffer_size)
{
	size_t i=0;
	i2c_start();
	i2c_address_direction(address<<1, I2C_Direction_Transmitter);
	while(i<buffer_size)
	{
		i2c_transmit(*(buffer+i));
		i++;
	}
	i2c_stop();
}
void i2c_read(uint8_t address, uint8_t* data)
{
    i2c_start();
    i2c_address_direction(address << 1, I2C_Direction_Receiver);
    *data = i2c_receive_nack();
    i2c_stop();
}
#endif /* I2C_I2C_C_ */
