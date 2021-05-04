// ----------------------------------------------------------------------------
#include <main.h>
#include "simcom/sim800.h"
#include "service/delay.h"
#include "power/power.h"
#include "sensor/dht11.h"
#include "usart/usart.h"
#include "rfid/mfrc552.h"
#include "gps/gps.h"
#include "rtc/rtc.h"
// ----------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

// ----------------------------------------------------------------------------
dht11_data data;
u8 status;
u8 str[16]; // Max_LEN = 16
// Khai bao cac bien cho SIM800 && MQTT broker
const char IP_Address[]="broker.hivemq.com";
uint16_t port=1883;
SIM800_t *sim800;
sim_t	*sim_APN;
RMC_Data *RMC;
RTC_Time_t Time;
MQTTString topicString[3] = MQTTString_initializer;
//char RMC_test[]="$GPRMC,013732.000,A,3150.7238,N,11711.7278,E,10.34,0.00,220413,,,A*68\r\n";
int requestedQoSs[3]={0,0,0};
uchar serNum[5];
char mqttTxBuffer[128]={0};
uint32_t t_check_connection = 0;
#define _USE_SIM	1
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
static void user_led_init();
static void user_led_toggle();
static void tim4_init();
static void tim5_init();
static void clk_init();
static void board_init();
void init_var(SIM800_t* sim800);
int main(int argc, char* argv[])
{
	unsigned char *topic=(unsigned char*)calloc(64,sizeof(unsigned char));
	unsigned char *payload=(unsigned char*)calloc(256,sizeof(unsigned char));
	sim800=(SIM800_t*)malloc(sizeof(SIM800_t));
	RMC = (RMC_Data*)malloc(sizeof(RMC_Data));
	topicString[0].cstring = (char*)"testtopic/Rx1";
	topicString[1].cstring = (char*)"testtopic/Rx2";
	topicString[2].cstring = (char*)"testtopic/Rx3";
	init_var(sim800);
	board_init();
#if _USE_SIM
	if (sim800->power_state == ON) {
		sim_init(sim800);
		sim_set_TCP_connection();
		sim_connect_server(sim800,10,5000);
	}
	MQTT_connect(sim800);
	MQTT_Pub((char*)"testtopic/Tx1",(char*) "Testmesage");
	delay_ms(500);
	MQTT_Sub(topicString, requestedQoSs,3);
	delay_ms(500);
	t_check_connection = millis();
#endif
	while(1)
	{
#if _USE_SIM
		if (millis()- t_check_connection >=10000)
		{
			t_check_connection = millis();
#if _DEBUG_UART5
			UART5_Send_String((char*)"Check connection with MQTT Broker");
			UART5_Send_String("\n");
#endif
			if (MQTT_PingReq(sim800)==0){
#if _DEBUG_UART5
				UART5_Send_String((char*)"Reconnecting...");
				UART5_Send_String("\n");
#endif
				if (sim_current_connection_status()==CONNECT_OK) {
					sim_disconnect_server(sim800);
					sim_connect_server(sim800,10,5000);
					MQTT_connect(sim800);
				}
				else if (sim_current_connection_status()== TCP_CLOSED)
				{
					sim_connect_server(sim800,10,5000);
					MQTT_connect(sim800);
				}
			}
		}
		if (sim800->mqttReceive.newEvent==1)
		{
			topic = sim800->mqttReceive.topic;
			payload = (sim800->mqttReceive.payload);
			sim800->mqttReceive.newEvent = 0;
#if _DEBUG
			trace_puts((char*)topic);
			trace_puts((char*)payload);
#elif _DEBUG_UART5
			UART5_Send_String((char*)topic);
			UART5_Send_String("\n");
			UART5_Send_String((char*)payload);
			UART5_Send_String("\n");
#endif
		}
		else if (gps_read_data(RMC)){
			memset(mqttTxBuffer,0,sizeof(mqttTxBuffer));
			sprintf(mqttTxBuffer,"Time:%d:%d:%d Date:%d:%d:%d",RMC->Time.hh,RMC->Time.mm,RMC->Time.ss,
															   RMC->Date.day,RMC->Date.month,RMC->Date.year);
			MQTT_Pub((char*)"testtopic/Tx1",(char*) mqttTxBuffer);
		}
#endif
//		status = MFRC522_Request(PICC_REQIDL, str);
//				if (status == MI_OK)
//				{
//					trace_printf("Find out a card: %x, %x\r\n",str[0],str[1]);
//				}
//				status = MFRC522_Anticoll(str);
//						memcpy(serNum, str, 5);
//						if (status == MI_OK)
//						{
//							trace_printf("Your card's number are: %x, %x, %x, %x, %x \r\n",serNum[0], serNum[1], serNum[2], serNum[3],serNum[4]);
//						}
//		if (dht11_read_data(&data))
//		{
//#if _DEBUG
//			trace_printf("Success, Temp = %d.%d; Hum = %d.%d\n",data.Temp_Byte1,data.Temp_Byte2,data.RH_Byte1,data.RH_Byte2);
//#endif
//		}
//		else {
//#if _DEBUG
//			trace_puts("ERROR");
//#endif
//		}
//		delay_ms(1000);
	}
	return 0;
}
void init_var(SIM800_t* sim800)
{
	sim_APN=(sim_t*)malloc(sizeof(sim_t));
	sim_APN->apn = "m-wap";
	sim_APN->apn_user = "mms";
	sim_APN->apn_pass = "mms";
	sim800->sim = sim_APN;
	sim800->mqttServer.host = (char*)IP_Address;
	sim800->mqttServer.port = port;
	sim800->mqttServer.connect = false;
	sim800->mqttClient.username = "user";
	sim800->mqttClient.pass = "user";
	sim800->mqttClient.clientID = "Client01";
	sim800->mqttClient.keepAliveInterval = 120;
	sim800->power_state = OFF;
}
static void board_init()
{
	clk_init();
	RTC_Init();
	user_led_init();
	tim4_init();
	power_reset_sim();
	sim_gpio_init();
	sim_power_off(sim800);
	usart_init();
	MFRC522_Init();
	gps_init();
	sim_power_on(sim800);
}
static void user_led_init()
{
	GPIO_InitTypeDef GPIO_init_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_init_struct.GPIO_Pin = USER_LED;
	GPIO_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_init_struct);
}
static void user_led_toggle()
{
	GPIOA->ODR ^=(uint32_t)(USER_LED);
}
/*	TIMER 4 Config 1uS, SysClock = 72Mhz
 * 	Prescaler = 71
 */
static void tim4_init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 71;
	TIM_TimeBaseInitStruct.TIM_Period = 0xffff-1;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
}
static void tim5_init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 7199;
	TIM_TimeBaseInitStruct.TIM_Period = 40000;
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStruct);
	TIM_ClearFlag(TIM5, TIM_FLAG_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	NVIC_EnableIRQ(TIM5_IRQn);
	TIM_Cmd(TIM5, ENABLE);
}
static void clk_init()
{
	RCC_DeInit();
	RCC_HSICmd(DISABLE);
	RCC_HSEConfig(RCC_HSE_ON);
	while(RCC_GetFlagStatus(RCC_FLAG_HSERDY)==RESET);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);
	RCC_PLLCmd(DISABLE);
	RCC_PREDIV1Config(RCC_PREDIV1_Source_HSE, RCC_PREDIV1_Div1);
	RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource()!=0x08);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
