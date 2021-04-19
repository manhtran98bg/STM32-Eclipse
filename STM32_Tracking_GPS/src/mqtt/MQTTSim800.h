/*
 * MQTTSim800.h
 *
 *  Created on: Apr 20, 2021
 *      Author: manht
 */

#ifndef MQTT_MQTTSIM800_H_
#define MQTT_MQTTSIM800_H_
#include "main.h"
#include "MQTTPacket.h"
#include "../usart/usart.h"
#include "../service/delay.h"
// === CONFIG ===
#define UART_SIM800 USART1
#define FREERTOS    0
#define CMD_DELAY   2000
// ==============

typedef struct {
    char *apn;
    char *apn_user;
    char *apn_pass;
} sim_t;

typedef struct {
    char *host;
    uint16_t port;
    uint8_t connect;
} mqttServer_t;

typedef struct {
    char *username;
    char *pass;
    char *clientID;
    unsigned short keepAliveInterval;
} mqttClient_t;

typedef struct {
    uint8_t newEvent;
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgId;
    unsigned char payload[64];
    int payloadLen;
    unsigned char topic[64];
    int topicLen;
} mqttReceive_t;

typedef struct {
    sim_t sim;
    mqttServer_t mqttServer;
    mqttClient_t mqttClient;
    mqttReceive_t mqttReceive;
} SIM800_t;


void Sim800_RxCallBack(void);

void clearRxBuffer(void);

void clearMqttBuffer(void);

int SIM800_SendCommand(char *command, char *reply, uint16_t delay);

int MQTT_Init(void);

void MQTT_Connect(void);

void MQTT_Pub(char *topic, char *payload);

void MQTT_PingReq(void);

void MQTT_Sub(char *topic);

void MQTT_Receive(unsigned char *buf);

#endif /* MQTT_MQTTSIM800_H_ */
