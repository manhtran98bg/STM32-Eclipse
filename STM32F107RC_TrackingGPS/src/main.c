// ----------------------------------------------------------------------------
#include <main.h>
#include "simcom/sim800.h"
#include "service/delay.h"
#include "power/power.h"
#include "sensor/dht11.h"
#include "usart/usart.h"
#include "rfid/mfrc552.h"
#include "gps/gps.h"
// ----------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

// ----------------------------------------------------------------------------
dht11_data data;
u8 status;
u8 str[16]; // Max_LEN = 16
//const char IP_Address[]="11247f7ffbf04962859c1bbd167dc340.s1.eu.hivemq.cloud";
const char IP_Address[]="broker.hivemq.com";
//const char IP_Address[]="118.68.132.242";
uint16_t port=1883;
uchar serNum[5];
SIM800_t *sim800;
sim_t	*sim_APN;
__IO uint8_t flagRx5 = 0;
char mqttBuffer[128]={0};

extern __IO char RxBuffer5[];
extern __IO char RxBuffer1[];
extern __IO uint8_t RxCounter5;
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
static void user_led_init();
static void user_led_toggle();
static void tim4_init();
static void clk_init();
void init_var(SIM800_t* sim800);

int main(int argc, char* argv[])
 {
	sim800=(SIM800_t*)malloc(sizeof(SIM800_t));
	init_var(sim800);
	clk_init();
	power_reset_sim();
	user_led_init();
	tim4_init();
	sim_gpio_init();
	sim_power_off();
	usart_init();
//	MFRC522_Init();
	gps_init();
	if	(!sim_power_on())
	{
		while(1);	// Sim can't start because no Power.
	}
	if (!sim_init(sim800))
	{
		while(1);	//Sim can't init, check log.
	}
	if (!sim_set_TCP_connection())
	{
		while(1);
	}
	if (!sim_connect_server(sim800)){
		while(1);	//Sim can't connect to server.
	}
	MQTT_connect(sim800);
	for(int i=0;i<3;i++){
		sprintf(mqttBuffer,"Count I=%d",i);
		MQTT_Pub((char*)"testtopic/1", mqttBuffer);
	};
	if (!sim_disconnect_server(sim800))
	{
		while(1);
	}
	if (!sim_detach_gprs(sim800))
	{
		while(1);
	}
	while(1)
	{
//			if (gps_read_data()){
//				user_led_toggle();
//			}
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
	sim800->mqttClient.clientID = "TestSub";
	sim800->mqttClient.keepAliveInterval = 120;
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
	//GPIOA->ODR ^=USER_LED;
	GPIO_SetBits(GPIOA, USER_LED);
	dUS_tim4(50000);
	GPIO_ResetBits(GPIOA, USER_LED);
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