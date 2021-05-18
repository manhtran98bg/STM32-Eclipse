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
#define _DEBUG_AT_CMD 0		//Macro cho phep in AT command de DEBUG
#define _DEBUG_RES	  0		//Macro cho phep in Response de DEBUG
#define QoS	0

/*----------------------------------------- Define USART1 ---------------------------------*/
/* USART1 For SIM800C*/
#define SIM_UART			USART3
#define SIM_UART_CLK		RCC_APB1Periph_USART3
#define SIM_UART_GPIO		GPIOB
#define SIM_UART_GPIO_CLK	RCC_APB2Periph_GPIOB
#define SIM_UART_GPIO_TX	GPIO_Pin_10
#define SIM_UART_GPIO_RX	GPIO_Pin_11

#define SIM_BUFFER_SIZE	1460
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

typedef enum{
	NORESPONSE = -2,
	NOSIGNAL,
	MARGINAL,
	OK,
	GOOD,
	EXCELLENT
}signal_t;
typedef enum {
	OFF,
	ON
}pwr_state;
typedef struct {
    char *apn;
    char *apn_user;
    char *apn_pass;
} sim_t;
typedef struct{
	char *model_id;
	char *manufacturer_id;
	char imei[16];
}sim_id_t;
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
    unsigned char payload[256];
    int payloadLen;
    unsigned char topic[64];
    int topicLen;
} mqttReceive_t;
typedef enum{
	NO_PWR = 0,
	NO_RES,
	NO_SIM,
	NO_REG,
	NO_GPRS,
	NO_APN,
	NO_ERR,
}error_t;
typedef struct {
    sim_t sim;
    mqttServer_t mqttServer;
    mqttClient_t mqttClient;
    mqttReceive_t mqttReceive;
    pwr_state power_state;
    state	simState;
    bool tcp_connect;
	signal_t signal_condition;
	char rssi[4];
	sim_id_t sim_id;
	error_t	sim_err;
} SIM800_t;

extern __IO char RxBuffer1[];
extern __IO uint16_t RxCounter1;
extern SIM800_t *sim800;
extern MQTTString topicString[];

extern char sim_buffer[];
extern uint16_t sim_buffer_index;

void sim_gpio_init();

uint8_t sim_power_status(SIM800_t *sim800);
uint8_t sim_power_on(SIM800_t *sim800);
void sim_power_off(SIM800_t *sim800);
void sim_send_cmd(char* cmd, uint16_t ms);
uint8_t sim_init(SIM800_t *sim800);
uint8_t sim_connect_server(SIM800_t *sim800, char num_try, int timeout_ms);
uint8_t sim_disconnect_server(SIM800_t *sim800);
uint8_t sim_attach_gprs(char num_try, int timeout_ms);
uint8_t sim_detach_gprs(char num_try, int timeout_ms);
uint8_t sim_set_TCP_connection();
uint8_t sim_send_message(unsigned char* message, uint8_t datalen);
state sim_current_connection_status();
void sim_error_handler(void);
signal_t sim_check_signal_condition(SIM800_t *sim800, int timeout_ms);
void sim_reconnect_handler(SIM800_t *sim800);
uint8_t sim_nosignal_handler(SIM800_t *sim800);
void MQTT_Pub(char *topic, char *payload);
void MQTT_Sub(MQTTString *topicString, int *requestedQoSs, int topic_count) ;
uint8_t MQTT_Connect(SIM800_t *sim800);
uint8_t MQTT_PingReq(SIM800_t *sim800);
void Sim800_RxCallBack();
#endif /* SIMCOM_SIM800_H_ */
