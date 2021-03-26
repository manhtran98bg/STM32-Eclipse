#include "actuator_io_hardware.h"
#include "../src/board.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"

uint16_t GPIO_OUT_PIN[MAX_APP_OUTPUT_NUM] ={
	    FAR_LIGHT_CTRL_PIN ,
        COS_LIGHT_CTRL_PIN ,
	    TURN_L_LIGHT_CTRL_PIN ,
	    TURN_R_LIGHT_CTRL_PIN ,
	    HORN_CTRL_PIN ,
		BRAKE_LIGHT_CTRL_PIN,
		BACK_LIGHT_CTRL_PIN ,
		ECO_MODE_CTRL_PIN,
		SPORT_MODE_CTRL_PIN,
		LED_CTRL1_PIN,
		LED_CTRL2_PIN,

};

 GPIO_TypeDef * GPIO_OUT_PORT[MAX_APP_OUTPUT_NUM] = {
		FAR_LIGHT_CTRL_PORT,
		COS_LIGHT_CTRL_PORT,
		TURN_L_LIGHT_CTRL_PORT,
		TURN_R_LIGHT_CTRL_PORT,
		HORN_CTRL_PORT,
		BRAKE_LIGHT_CTRL_PORT,
		BACK_LIGHT_CTRL_PORT ,
		ECO_MODE_CTRL_PORT,
		SPORT_MODE_CTRL_PORT,
		LED_CTRL1_PORT,
		LED_CTRL2_PORT,
};

 uint16_t GPIO_IN_PIN[MAX_APP_INPUT_NUM] ={
	    FAR_LIGHT_BUF_PIN ,
        COS_LIGHT_BUF_PIN ,
	    TURN_L_LIGHT_BUF_PIN ,
	    TURN_R_LIGHT_BUF_PIN ,
	    HORN_BUF_PIN ,
		BRAKE_LIGHT_BUF_PIN ,
		H_LIGHT_BUF_PIN ,
		ECO_MODE_BUF_PIN,
		SPORT_MODE_BUF_PIN,
		PUSH_START_STOP_SYSTEM_BUF_PIN,
};
 GPIO_TypeDef * GPIO_IN_PORT[MAX_APP_INPUT_NUM] = {
		FAR_LIGHT_BUF_PORT,
		COS_LIGHT_BUF_PORT,
		TURN_L_LIGHT_BUF_PORT,
		TURN_R_LIGHT_BUF_PORT,
		HORN_BUF_PORT,
		BRAKE_LIGHT_BUF_PORT ,
		H_LIGHT_BUF_PORT ,
		ECO_MODE_BUF_PORT,
		SPORT_MODE_BUF_PORT,
		PUSH_START_STOP_SYSTEM_BUF_PORT,
};

extern bool key_state;

volatile uint8_t ioc_tick = 0;
volatile uint8_t tick_count = 0;
volatile uint16_t fre_count = 0 ;
volatile uint16_t fre_on = 0;
const uint16_t fre = 400;
volatile uint8_t dim_mode = 1;
volatile uint16_t timeout = 0;
volatile uint8_t  check = 0;

TIM_TimeBaseInitTypeDef TimBase_InitStructure;
static void actuator_io_rcc_init(void);
static void actuator_io_gpio_init(void);
static void actuator_io_timer_init(void);

void actuator_io_hardware_init(){
	actuator_io_rcc_init();
	actuator_io_gpio_init();
    actuator_io_timer_init();
}

uint8_t read_gpio_in(unsigned char index_in ){
	return GPIO_ReadInputDataBit(GPIO_IN_PORT[index_in],GPIO_IN_PIN[index_in]);
}
uint8_t read_gpio_out(unsigned char index_out){
	return GPIO_ReadOutputDataBit(GPIO_OUT_PORT[index_out],GPIO_OUT_PIN[index_out]);
}

void set_gpio_out(unsigned char index_out){
	GPIO_SetBits(GPIO_OUT_PORT[index_out],GPIO_OUT_PIN[index_out]);
}

void clear_gpio_out(unsigned char index_out){
	GPIO_ResetBits(GPIO_OUT_PORT[index_out],GPIO_OUT_PIN[index_out]);
}

void reverse_gpio_out(unsigned char index_out){
    uint8_t pin_cur_state = read_gpio_out(index_out);
    GPIO_WriteBit(GPIO_OUT_PORT[index_out],GPIO_OUT_PIN[index_out],!pin_cur_state);
}


static void actuator_io_rcc_init(void){
	  	  // Enable GPIO Peripheral clock
		  RCC_AHBPeriphClockCmd(PORTA_CLK|PORTB_CLK|PORTC_CLK|PORTD_CLK|PORTE_CLK|PORTF_CLK, ENABLE);
		  RCC_APB1PeriphClockCmd(IOC_TIM_DEV_CLK,ENABLE);
		  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

}

static void actuator_io_gpio_init(void){

	GPIO_InitTypeDef GPIO_Initstructure;

		/* OUTPUT PIN */
		GPIO_Initstructure.GPIO_Mode  = GPIO_Mode_OUT;
		GPIO_Initstructure.GPIO_OType  = GPIO_OType_PP;
		GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Initstructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_Initstructure.GPIO_Pin   = FAR_LIGHT_CTRL_PIN ;
		GPIO_Init(FAR_LIGHT_CTRL_PORT, &GPIO_Initstructure);
		GPIO_Initstructure.GPIO_Pin   = COS_LIGHT_CTRL_PIN ;
		GPIO_Init(COS_LIGHT_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin   = TURN_L_LIGHT_CTRL_PIN  ;
		GPIO_Init(TURN_L_LIGHT_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin   = TURN_R_LIGHT_CTRL_PIN ;
		GPIO_Init(TURN_R_LIGHT_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin   = HORN_CTRL_PIN ;
		GPIO_Init(HORN_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = BACK_LIGHT_CTRL_PIN;
		GPIO_Init(BACK_LIGHT_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = BRAKE_LIGHT_CTRL_PIN;
		GPIO_Init(BRAKE_LIGHT_CTRL_PORT, &GPIO_Initstructure);

//		GPIO_Initstructure.GPIO_Pin = VIB_MOTOR_CTRL_PIN;
//		GPIO_Init(VIB_MOTOR_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = LED_CTRL1_PIN;
		GPIO_Init(LED_CTRL1_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = LED_CTRL2_PIN;
		GPIO_Init(LED_CTRL2_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = ECO_MODE_CTRL_PIN;
		GPIO_Init(ECO_MODE_CTRL_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = SPORT_MODE_CTRL_PIN;
		GPIO_Init(SPORT_MODE_CTRL_PORT, &GPIO_Initstructure);

		/* INPUT PIN */

		GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Initstructure.GPIO_Speed = GPIO_Speed_10MHz;

		GPIO_Initstructure.GPIO_Pin = FAR_LIGHT_BUF_PIN;
		GPIO_Init(FAR_LIGHT_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = COS_LIGHT_BUF_PIN;
		GPIO_Init(COS_LIGHT_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = TURN_L_LIGHT_BUF_PIN;
		GPIO_Init(TURN_L_LIGHT_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = TURN_R_LIGHT_BUF_PIN;
		GPIO_Init(TURN_R_LIGHT_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = HORN_BUF_PIN;
		GPIO_Init(HORN_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = BRAKE_LIGHT_BUF_PIN;
		GPIO_Init(BRAKE_LIGHT_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = H_LIGHT_BUF_PIN;
		GPIO_Init(H_LIGHT_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = ECO_MODE_BUF_PIN;
		GPIO_Init(ECO_MODE_BUF_PORT, &GPIO_Initstructure);

		GPIO_Initstructure.GPIO_Pin = PUSH_START_STOP_SYSTEM_BUF_PIN;
		GPIO_Init(PUSH_START_STOP_SYSTEM_BUF_PORT, &GPIO_Initstructure);

//		GPIO_Initstructure.GPIO_Pin = KICK_STAND_BUF_PIN;
//		GPIO_Init(KICK_STAND_BUF_PORT, &GPIO_Initstructure);

}

void actuator_io_timer_init(void){


	NVIC_InitTypeDef NVIC_InitStructure;

	TimBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimBase_InitStructure.TIM_Prescaler  = IOC_TIM_PRESCALER1;
	TimBase_InitStructure.TIM_Period  = IOC_TIM_PERIOD1;
	TimBase_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimBase_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(IOC_TIM_DEV,&TimBase_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = IOC_TIM_HANDLE_PRIORITY;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ClearFlag(IOC_TIM_DEV,TIM_FLAG_Update);
	TIM_ITConfig(IOC_TIM_DEV,TIM_IT_Update,ENABLE);
	//ioc_tim_enable();

}


void ioc_tim_enable(void){
	TIM_Cmd(IOC_TIM_DEV,ENABLE);
}
void ioc_tim_disable(void){
	TIM_Cmd(IOC_TIM_DEV,DISABLE);
	IOC_TIM_DEV->CNT = 0;
}

void ioc_tim_start_ver2(void){
	ioc_tim_disable();
	TimBase_InitStructure.TIM_Prescaler  = IOC_TIM_PRESCALER1;
	TimBase_InitStructure.TIM_Period = IOC_TIM_PERIOD1;
	TIM_TimeBaseInit(IOC_TIM_DEV,&TimBase_InitStructure);
}

void IOC_TIM_HANDLE(void) {

	if (TIM_GetITStatus(IOC_TIM_DEV, TIM_IT_Update) != RESET) {
#if 1
		ioc_tick++;
		ioc_tim_disable();

#endif
#if 0
		if (key_state == true) {
			ioc_tick++;
			TIM_ClearFlag(IOC_TIM_DEV, TIM_FLAG_Update);
			ioc_tim_disable();
		}
		else{
			if (fre_count <= fre_on) {
				GPIO_SetBits(LED_CTRL1_PORT, LED_CTRL1_PIN);
				GPIO_SetBits(LED_CTRL2_PORT, LED_CTRL2_PIN);
			} else if (fre_count < fre) {
				GPIO_ResetBits(LED_CTRL1_PORT, LED_CTRL1_PIN);
				GPIO_ResetBits(LED_CTRL2_PORT, LED_CTRL2_PIN);
			} else {
				fre_count = 0;
				if (dim_mode == 1) {
					if (++fre_on == fre)
						dim_mode = 0;
				} else {
					if (--fre_on == 0) {
						dim_mode = 1;
					}
				}
			}
			fre_count++;
		}
#endif
		TIM_ClearFlag(IOC_TIM_DEV, TIM_FLAG_Update);
	}

}


