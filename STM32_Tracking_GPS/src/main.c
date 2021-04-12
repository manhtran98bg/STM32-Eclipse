// ----------------------------------------------------------------------------
#include <main.h>
#include "simcom/sim800.h"
#include "service/delay.h"
#include "power/power.h"
#include "sensor/dht11.h"
#include "usart/usart.h"
#include "rfid/mfrc552.h"
// ----------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

// ----------------------------------------------------------------------------
dht11_data data;
extern uint8_t RxBuffer[];
extern __IO uint8_t RxBuffer1[];
extern __IO uint8_t RxCounter1;

extern __IO uint8_t RxBuffer5[];
extern __IO uint8_t RxCounter5;

extern __IO uint8_t RxBuffer4[];
extern __IO uint8_t RxCounter4;


u8 status;
u8 str[16]; // Max_LEN = 16
uchar serNum[5];
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

static void user_led_init();
static void user_led_toggle();
static void tim4_init();
static void clk_init();
int main(int argc, char* argv[])
{
	clk_init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000);
	power_reset_sim();
	user_led_init();
	tim4_init();
	sim_gpio_init();
	usart_init();
	MFRC522_Init();
//	if	(!sim_power_on())
//	{
//		while(1);	// Sim can't start because no Power.
//	}
//	if (!sim_init())
//	{
//		while(1);	//Sim can't init, check log.
//	}
//	delay_ms(5000);
//	power_on_gps();
//	UART4_Send_String((uint8_t*)"$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
//	delay_ms(1000);
	while(1)
	{
		status = MFRC522_Request(PICC_REQIDL, str);
				if (status == MI_OK)
				{
					trace_printf("Find out a card: %x, %x\r\n",str[0],str[1]);
				}
				status = MFRC522_Anticoll(str);
						memcpy(serNum, str, 5);
						if (status == MI_OK)
						{
							trace_printf("Your card's number are: %x, %x, %x, %x, %x \r\n",serNum[0], serNum[1], serNum[2], serNum[3],serNum[4]);
						}
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
static void user_led_init()
{
	GPIO_InitTypeDef GPIO_init_struct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_init_struct.GPIO_Pin = USER_LED;
	GPIO_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_init_struct);
//	GPIO_InitTypeDef GPIO_InitStruct;
//	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStruct);
//	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}
static void user_led_toggle()
{
	GPIOA->ODR ^=USER_LED;
	dUS_tim4(65000);
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
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource()!=0x08);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1);
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
