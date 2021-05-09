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
#include "lcd/sh1106.h"
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
//const char IP_Address[]="broker.hivemq.com";
const char IP_Address[]="broker.emqx.io";
uint16_t port=1883;
SIM800_t *sim800;
RMC_Data *RMC;
RTC_Time_t Time;
MQTTString topicString[10] = MQTTString_initializer;
char RMC_test[]="$GPRMC,013732.000,A,3150.7238,N,11711.7278,E,10.34,0.00,220413,,,A*68\r\n";
int requestedQoSs[3]={0,0,0};
uchar serNum[5];
char mqttTxBuffer[128]={0};
char topic_pub[50]={0};
char topic_buff[10][60]={{0}};
char json_geowithtime[100]={0};
uint32_t t_check_connection = 0;
uint8_t nosignal_check = 0;
#define _USE_SIM	0
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
static void user_led_init();
static void user_led_toggle();
static void tim4_init();
static void tim5_init();
static void clk_init();
static void btn_init();
static void board_init();
void init_var(SIM800_t* sim800);
void init_topic(MQTTString *topicList,char *topic_buff, char *IMEI);
void first_pub_topic(MQTTString *topicList);
int main(int argc, char* argv[])
{
	unsigned char *topic=(unsigned char*)calloc(64,sizeof(unsigned char));
	unsigned char *payload=(unsigned char*)calloc(256,sizeof(unsigned char));
	sim800=(SIM800_t*)malloc(sizeof(SIM800_t));
	RMC = (RMC_Data*)malloc(sizeof(RMC_Data));
	init_var(sim800);
	RMC_Parse(RMC, RMC_test, strlen(RMC_test));
	RMC_json_init(RMC, json_geowithtime);
	sprintf(mqttTxBuffer,"Lat:%d.%ld",RMC->Lat.lat_dec_degree.int_part,RMC->Lat.lat_dec_degree.dec_part);
	board_init();
	sh1106_Init();
	sh1106_SetCursor(2, 10);
	sh1106_WriteString("XIN CHAO", Font_6x8, White);
	sh1106_UpdateScreen();
	sh1106_SetCursor(2, 20);
	sh1106_WriteString("Cac ban", Font_6x8, White);
	sh1106_UpdateScreen();
	while(1);

#if _USE_SIM
	if (sim800->power_state == ON) {
		sim_init(sim800);
		init_topic(topicString, (char*)topic_buff, (char*)sim800->sim_id.imei);
		sim_set_TCP_connection();
		if (sim_connect_server(sim800,10,5000)){
			delay_ms(500);
			MQTT_Connect(sim800);
			delay_ms(500);
			first_pub_topic(topicString);
		}
	}
	t_check_connection = millis();
#endif
	while(1)
	{
#if _USE_SIM
		if (millis() - t_check_connection>=10000) {
			t_check_connection = millis();
			sim_nosignal_handler(sim800);

		}
		if (nosignal_check==1) {
			sim_reconnect_handler(sim800);
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
			sim800->signal_condition = sim_check_signal_condition(sim800, 500);
			memset(mqttTxBuffer,0,sizeof(mqttTxBuffer));
			sprintf(mqttTxBuffer,"Time:%d:%d:%d Date:%d:%d:%d",RMC->Time.hh,RMC->Time.mm,RMC->Time.ss,
															   RMC->Date.day,RMC->Date.month,RMC->Date.year);
			if (sim800->mqttServer.connect)	{
				MQTT_Pub(topicString[8].cstring,json_geowithtime);
			}
//			sprintf(mqttTxBuffer,"Lat:%d:%d:%d Lon:%d:%d:%d",RMC->Lat.lat_dd,RMC->Lat.lat_mm,RMC->Lat.lat_mmmm,
//															 RMC->Lon.lon_ddd,RMC->Lon.lon_mm,RMC->Lon.lon_mmmm);
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
	sim800->sim.apn = "m-wap";
	sim800->sim.apn_user = "mms";
	sim800->sim.apn_pass = "mms";
	sim800->mqttServer.host = (char*)IP_Address;
	sim800->mqttServer.port = port;
	sim800->mqttServer.connect = false;
	sim800->mqttClient.username = "user";
	sim800->mqttClient.pass = "user";
	sim800->mqttClient.clientID = "Client01";
	sim800->mqttClient.keepAliveInterval = 10;
	sim800->power_state = OFF;
	sim800->tcp_connect = false;
	sim800->sim_id.model_id ="SIMCOM_SIM800C";
	sim800->sim_id.manufacturer_id = "SIMCOM_Ltd";
}
void init_topic(MQTTString *topicList,char *topic_buff, char *IMEI)
{

	sprintf(topic_buff,"mandevices/GSHT_%s/$name",IMEI);
	topicList[0].cstring = topic_buff;
	sprintf(topic_buff+60,"mandevices/GSHT_%s/$state",IMEI);
	topicList[1].cstring = topic_buff+60;
	sprintf(topic_buff+120,"mandevices/GSHT_%s/$homie",IMEI);
	topicList[2].cstring = topic_buff+120;
	sprintf(topic_buff+180,"mandevices/GSHT_%s/$node",IMEI);
	topicList[3].cstring = topic_buff+180;
	sprintf(topic_buff+240,"mandevices/GSHT_%s/environment/temperature/$name",IMEI);
	topicList[4].cstring = topic_buff+240;
	sprintf(topic_buff+300,"mandevices/GSHT_%s/environment/temperature/$datatype",IMEI);
	topicList[5].cstring = topic_buff+300;
	sprintf(topic_buff+360,"mandevices/GSHT_%s/environment/humidity/$name",IMEI);
	topicList[6].cstring = topic_buff+360;
	sprintf(topic_buff+420,"mandevices/GSHT_%s/environment/humidity/$datatype",IMEI);
	topicList[7].cstring = topic_buff+420;
	sprintf(topic_buff+480,"mandevices/GSHT_%s/device/location",IMEI);
	topicList[8].cstring = topic_buff+480;
}
void first_pub_topic(MQTTString *topicList)
{
	MQTT_Pub(topicList[0].cstring, "Giam sat hanh trinh");
	delay_ms(50);
	MQTT_Pub(topicList[1].cstring, "ready");
	delay_ms(50);
	MQTT_Pub(topicList[2].cstring, "4.0.0");
	delay_ms(50);
	MQTT_Pub(topicList[3].cstring, "enviroment,device");
	delay_ms(50);
}
static void board_init()
{
	clk_init();
	RTC_Init();
	user_led_init();
	btn_init();
	tim4_init();
	power_reset_sim();
	sim_gpio_init();
	sim_power_off(sim800);
	usart_init();
	MFRC522_Init();
	gps_init();
#if _USE_SIM
	sim_power_on(sim800);
#endif
}
static void btn_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitTypeDef gpio_init_struct;
	EXTI_InitTypeDef exti_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	gpio_init_struct.GPIO_Pin = USER_BTN1|USER_BTN2|USER_BTN3;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &gpio_init_struct);
	/* Connect EXTI12 Line to PB12 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	/* Connect EXTI13 Line to PB13 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	/* Connect EXTI14 Line to PB14 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	/* Configure EXTI line */
	exti_init_struct.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14;
	exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_struct.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init_struct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init_struct);
	nvic_init_struct.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0x0F;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);

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
