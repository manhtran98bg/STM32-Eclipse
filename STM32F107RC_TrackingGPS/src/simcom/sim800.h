/*
 * sim800.h
 *
 *  Created on: Apr 10, 2021
 *      Author: manht
 */

#ifndef SIMCOM_SIM800_H_
#define SIMCOM_SIM800_H_
#include <main.h>
#include <string.h>
#include "../mqtt/MQTTPacket.h"
#define _DEBUG_AT_CMD 0
#define _DEBUG_SIM_UART5	1
#define _DEBUG_AT_UART5	0
typedef enum {
	IP_INITIAL,
	IP_START,
	IP_CONFIG,
	IP_GPRSACT,
	IP_STATUS,
	TCP_CONNECTING,
	CONNECT_OK,
	TCP_CLOSING,
	TCP_CLOSED,
	PDP_DEACT,
	DEFAUT
}state;

typedef struct {
    char *apn;
    char *apn_user;
    char *apn_pass;
} sim_t;

typedef struct {
    char *host;
    uint16_t port;
    bool connect;
} mqttServer_t;

typedef struct {
    char *username;
    char *pass;
    char *clientID;
    unsigned short keepAliveInterval;
} mqttClient_t;

typedef struct {
    bool newEvent;
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
    sim_t *sim;
    mqttServer_t mqttServer;
    mqttClient_t mqttClient;
    mqttReceive_t mqttReceive;
    state	simState;
} SIM800_t;

void sim_gpio_init();
uint8_t sim_power_on();
void sim_power_off();
void sim_send_cmd(char* cmd, uint16_t ms);
uint8_t sim_init(SIM800_t *sim800);
uint8_t sim_connect_server(SIM800_t *sim800);
uint8_t sim_disconnect_server(SIM800_t *sim800);
uint8_t sim_detach_gprs(SIM800_t *sim800);
uint8_t sim_set_TCP_connection();
uint8_t sim_send_message(unsigned char* message, uint8_t datalen);
state sim_current_connection_status();
void MQTT_Pub(char *topic, char *payload);
void MQTT_connect(SIM800_t *sim800);
#endif /* SIMCOM_SIM800_H_ */
