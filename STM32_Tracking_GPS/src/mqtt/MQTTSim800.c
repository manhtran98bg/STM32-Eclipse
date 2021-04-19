/*
 * MQTTSim800.c
 *
 *  Created on: Apr 20, 2021
 *      Author: manht
 */
#include "MQTTSim800.h"

extern SIM800_t SIM800;

uint8_t rx_data = 0;
__IO uint8_t rx_buffer[1460] = {0};
uint16_t rx_index = 0;

uint8_t mqtt_receive = 0;
__IO char mqtt_buffer[1460] = {0};
uint16_t mqtt_index = 0;
/**
 * Call back function for release read SIM800 UART buffer.
 * @param NONE
 * @return NONE
 */
void Sim800_RxCallBack(void) {
    rx_buffer[rx_index++] = rx_data;
    if (SIM800.mqttServer.connect == 0) {
        if (strstr((char *) rx_buffer, "\r\n") != NULL && rx_index == 2) {
            rx_index = 0;
        } else if (strstr((char *) rx_buffer, "\r\n") != NULL) {
            memcpy(mqtt_buffer, rx_buffer, sizeof(rx_buffer));
            clearRxBuffer();
            if (strstr(mqtt_buffer, "DY CONNECT\r\n")) {
                SIM800.mqttServer.connect = 0;
            } else if (strstr(mqtt_buffer, "CONNECT\r\n")) {
                SIM800.mqttServer.connect = 1;
            }
        }
    }
    if (strstr((char *) rx_buffer, "CLOSED\r\n") || strstr((char *) rx_buffer, "ERROR\r\n")) {
        SIM800.mqttServer.connect = 0;
    }
    if (SIM800.mqttServer.connect == 1 && rx_data == 48) {
        mqtt_receive = 1;
    }
    if (mqtt_receive == 1) {
        mqtt_buffer[mqtt_index++] = rx_data;
        if (mqtt_index > 1 && mqtt_index - 1 > mqtt_buffer[1]) {
            MQTT_Receive((unsigned char *) mqtt_buffer);
            clearRxBuffer();
            clearMqttBuffer();
        }
        if (mqtt_index >= sizeof(mqtt_buffer)) {
            clearMqttBuffer();
        }
    }
    if (rx_index >= sizeof(mqtt_buffer)) {
        clearRxBuffer();
        clearMqttBuffer();
    }
    rx_data = USART_ReceiveData(UART_SIM800);
}
/**
 * Clear SIM800 UART RX buffer.
 * @param NONE
 * @return NONE
 */
void clearRxBuffer(void) {
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}
/**
 * Clear MQTT buffer.
 * @param NONE
 * @return NONE
 */
void clearMqttBuffer(void) {
    mqtt_receive = 0;
    mqtt_index = 0;
    memset(mqtt_buffer, 0, sizeof(mqtt_buffer));
}
/**
 * Send AT command to SIM800 over UART.
 * @param command the command to be used the send AT command
 * @param reply to be used to set the correct answer to the command
 * @param delay to be used to the set pause to the reply
 * @return error, 0 is OK
 */
int SIM800_SendCommand(char *command, char *reply, uint16_t delay) {
    USART1_Send_String((char*)command);
    delay_ms(delay);
    if (strstr(mqtt_buffer, reply) != NULL) {
        clearRxBuffer();
        return 0;
    }
    clearRxBuffer();
    return 1;
}
/**
 * initialization SIM800.
 * @param NONE
 * @return error status, 0 - OK
 */
int MQTT_Init(void) {
    SIM800.mqttServer.connect = 0;
    int error = 0;
    char str[32] = {0};
    SIM800_SendCommand("AT\r\n", "OK\r\n", CMD_DELAY);
    SIM800_SendCommand("ATE0\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CIPSHUT\r\n", "SHUT OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CGATT=1\r\n", "OK\r\n", CMD_DELAY);
    error += SIM800_SendCommand("AT+CIPMODE=1\r\n", "OK\r\n", CMD_DELAY);

    snprintf(str, sizeof(str), "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", SIM800.sim.apn, SIM800.sim.apn_user,
             SIM800.sim.apn_pass);
    error += SIM800_SendCommand(str, "OK\r\n", CMD_DELAY);

    error += SIM800_SendCommand("AT+CIICR\r\n", "OK\r\n", CMD_DELAY);
    SIM800_SendCommand("AT+CIFSR\r\n", "", CMD_DELAY);
//    if (error == 0) {
//        MQTT_Connect();
//        return error;
//    } else {
//        return error;
//    }
}
/**
 * Receive message from MQTT broker
 * @param receive mqtt bufer
 * @return NONE
 */
void MQTT_Receive(unsigned char *buf) {
    memset(SIM800.mqttReceive.topic, 0, sizeof(SIM800.mqttReceive.topic));
    memset(SIM800.mqttReceive.payload, 0, sizeof(SIM800.mqttReceive.payload));
    MQTTString receivedTopic;
    unsigned char *payload;
    MQTTDeserialize_publish(&SIM800.mqttReceive.dup, &SIM800.mqttReceive.qos, &SIM800.mqttReceive.retained,
                            &SIM800.mqttReceive.msgId,
                            &receivedTopic, &payload, &SIM800.mqttReceive.payloadLen, buf,
                            sizeof(buf));
    memcpy(SIM800.mqttReceive.topic, receivedTopic.lenstring.data, receivedTopic.lenstring.len);
    SIM800.mqttReceive.topicLen = receivedTopic.lenstring.len;
    memcpy(SIM800.mqttReceive.payload, payload, SIM800.mqttReceive.payloadLen);
    SIM800.mqttReceive.newEvent = 1;
}
