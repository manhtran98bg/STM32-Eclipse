#include "can_hardware.h"
#include "string.h"
CAN_Receive_Handle CANx_Receive_Handle;

static CAN_InitTypeDef CAN_InitStructure;
static void can_hardware_init_clk(void);
static void can_hardware_init_gpio(void);
static void can_hardware_init_module(void);
static void can_hardware_init_nvic(void);

void can_set_receive_handle(CAN_Receive_Handle handle)
{
	CANx_Receive_Handle = handle;
}
static void can_hardware_init_clk(void) {
	RCC_APB1PeriphClockCmd(CAN_DEV_CLK, ENABLE);
	RCC_AHBPeriphClockCmd(CAN_TX_PORT_CLK, ENABLE);

}

static void can_hardware_init_gpio(void) {

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_PinAFConfig(CAN_TX_PORT, CAN_TX_PINSOURCE, GPIO_AF_4);
	GPIO_PinAFConfig(CAN_RX_PORT, CAN_RX_PINSOURCE, GPIO_AF_4);

	GPIO_InitStructure.GPIO_Pin = CAN_TX_PIN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN_TX_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN_RX_PORT, &GPIO_InitStructure);


	GPIO_SetBits(CAN_TX_PORT,CAN_TX_PIN);
	GPIO_ResetBits(CAN_TX_PORT,CAN_TX_PIN);
	GPIO_SetBits(CAN_RX_PORT,CAN_RX_PIN);
	GPIO_ResetBits(CAN_RX_PORT,CAN_RX_PIN);
}

static void can_hardware_init_module(void) {

	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	CAN_DeInit(CAN);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    /* buadrate = pclk/(BS1+1 + BS2+1 + 1)/Prescaler  */
	/* this configure is corresponding to 500 kbs */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_13tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
	CAN_InitStructure.CAN_Prescaler = 6;
	if(!CAN_Init(CAN, &CAN_InitStructure))
	{
		can_error_handle();
	}

	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);

	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN, CAN_IT_FMP0, ENABLE);

}

static void can_hardware_init_nvic(void) {

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = CAN_IRQN;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPriority = CAN_IRQN_PRIORITY;
	NVIC_Init(&NVIC_InitStructure);
}
void can_error_handle(void)
{
	while(1)
	{
	}
}
void can_hardware_init(void) {
	can_hardware_init_clk();
	can_hardware_init_gpio();
	can_hardware_init_module();
	can_hardware_init_nvic();
}



