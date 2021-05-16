// ----------------------------------------------------------------------------
#include <main.h>
#include "simcom/sim800.h"
#include "service/delay.h"
#include "power/power.h"
#include "sensor/dht11.h"
#include "sensor/ds18b20.h"
#include "usart/usart.h"
#include "rfid/mfrc552.h"
#include "gps/gps.h"
#include "rtc/rtc.h"
#include "lcd/sh1106.h"
#include "board/board.h"
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
gps_t *gps_l70;
RTC_Time_t Time;
MQTTString topicString[20] = MQTTString_initializer;
char RMC_test[]="$GPRMC,104100.000,A,2059.6764,N,10552.0151,E,10.34,0.00,100521,,,A*68\r\n";
int requestedQoSs[3]={0,0,0};
uchar serNum[5];
char mqttTxBuffer[128]={0};
char topic_pub[50]={0};
char topic_buff[20][60]={{0}};
char json_geowithtime[100]={0};
char time_buffer[10]={0};
uint32_t t_check_connection = 0;
uint32_t t_lcd_update = 0;
uint8_t nosignal_check = 0;
bool _1sflag = false;
#define _USE_SIM	1
//#define _USE_TEST_RMC
#define _TEST_DS18B20
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
static void board_init();
static void sim_start();
static void lcd_start();
static void gps_start();
static void lcd_update();

void init_var();
void init_topic(MQTTString *topicList,char *topic_buff, char *IMEI);
void first_pub_topic(MQTTString *topicList);

int main(int argc, char* argv[])
{
	unsigned char *topic=(unsigned char*)calloc(64,sizeof(unsigned char));
	unsigned char *payload=(unsigned char*)calloc(256,sizeof(unsigned char));
	//Memory allocate SIM800 and GPS L70 Struct.
	sim800=(SIM800_t*)malloc(sizeof(SIM800_t));
	gps_l70=(gps_t*)malloc(sizeof(gps_l70));
	char payload_buf[20]={0};
	//Init first variable.
	init_var();
#ifdef _USE_TEST_RMC
	RMC_Parse(&gps_l70->RMC, RMC_test, strlen(RMC_test));
	RMC_json_init(&gps_l70->RMC, json_geowithtime);
#endif
	board_init();
	RTC_GetTime(&Time);
#ifdef _TEST_DS18B20
	ds18_config(RES_9BIT);
#endif
	lcd_start();
	sim_start();
	gps_start();
	delay_ms(1000);
	sh1106_Clear(Black);
	sh1106_UpdateScreen();
	while(1)
	{
		if (_1sflag == true )
		{
			sh1106_WriteString(50, 3, time_buffer, Font_6x8, White, ALIGN_RIGHT);
			sh1106_UpdateScreen();
			_1sflag = false;
		}
		if (millis()-t_lcd_update>2000)
		{
			ds18_read_temp(&ds18b20);
			sprintf(payload_buf,"%d",(int)ds18b20.temp);
			lcd_update();
			t_lcd_update = millis();
		}
#if _USE_SIM
		if (sim800->power_state == ON)	//Neu module SIM bat
		{
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
		}
		if (gps_l70->gps_pwr_state == true)	//Neu Module GPS bat
		{
			if (gps_read_data(gps_l70)){
				sim800->signal_condition = sim_check_signal_condition(sim800, 200);
				if (sim800->mqttServer.connect)	{
					if (gps_l70->RMC.Data_Valid[0]!='V') MQTT_Pub(topicString[11].cstring,json_geowithtime);
					MQTT_Pub(topicString[12].cstring,payload_buf);
					MQTT_Pub(topicString[7].cstring,sim800->rssi);
				}
			}
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

void init_var()
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
	sim800->mqttClient.keepAliveInterval = 30;
	sim800->power_state = OFF;
	sim800->tcp_connect = false;
	sim800->signal_condition = NOSIGNAL;
	sim800->sim_err = NO_PWR;
	sim800->sim_id.model_id ="SIMCOM_SIM800C";
	sim800->sim_id.manufacturer_id = "SIMCOM_Ltd";
	gps_l70->gps_pwr_state = false;
	gps_l70->gps_err = GPS_NO_PWR;
	Time.old_minute = 0;
	_1sflag = true;
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

	sprintf(topic_buff+240,"mandevices/GSHT_%s/sim/$state",IMEI);
	topicList[4].cstring = topic_buff+240;

	sprintf(topic_buff+300,"mandevices/GSHT_%s/sim/manufacturer_id",IMEI);
	topicList[5].cstring = topic_buff+300;

	sprintf(topic_buff+360,"mandevices/GSHT_%s/sim/model_id",IMEI);
	topicList[6].cstring = topic_buff+360;

	sprintf(topic_buff+420,"mandevices/GSHT_%s/sim/signal_strength",IMEI);
	topicList[7].cstring = topic_buff+420;

	sprintf(topic_buff+480,"mandevices/GSHT_%s/sim/imei",IMEI);
	topicList[8].cstring = topic_buff+480;

	sprintf(topic_buff+540,"mandevices/GSHT_%s/environment/temperature",IMEI);
	topicList[9].cstring = topic_buff+540;

	sprintf(topic_buff+600,"mandevices/GSHT_%s/environment/humidity",IMEI);
	topicList[10].cstring = topic_buff+600;

	sprintf(topic_buff+660,"mandevices/GSHT_%s/device/location",IMEI);
	topicList[11].cstring = topic_buff+660;

	sprintf(topic_buff+720,"mandevices/GSHT_%s/device/temperature",IMEI);
	topicList[12].cstring = topic_buff+720;

	sprintf(topic_buff+780,"mandevices/GSHT_%s/battery/rate_voltage",IMEI);
	topicList[13].cstring = topic_buff+780;

	sprintf(topic_buff+840,"mandevices/GSHT_%s/vehicle/rpm",IMEI);
	topicList[14].cstring = topic_buff+840;
}
void first_pub_topic(MQTTString *topicList)
{
	MQTT_Pub(topicList[0].cstring, "Giam sat hanh trinh");
	delay_ms(50);
	MQTT_Pub(topicList[1].cstring, "ready");
	delay_ms(50);
	MQTT_Pub(topicList[2].cstring, "4.0.0");
	delay_ms(50);
	MQTT_Pub(topicList[3].cstring, "enviroment,device,battery,vehicle,driver,gps,sim");
	delay_ms(50);
	MQTT_Pub(topicList[4].cstring, "ready");
	delay_ms(50);
	MQTT_Pub(topicList[5].cstring, "SIMCOM_Ltd");
	delay_ms(50);
	MQTT_Pub(topicList[6].cstring, "SIMCOM_SIM800C");
	delay_ms(50);
	MQTT_Pub(topicList[8].cstring, sim800->sim_id.imei);
	delay_ms(50);
}
static void sim_start(){
#if _USE_SIM
	if(sim_init(sim800)){	//Khoi tao sim thanh cong
		sim_error_handler();
		init_topic(topicString, (char*)topic_buff, (char*)sim800->sim_id.imei);
		sh1106_WriteString(2, 10, "-SETUP TCP:", Font_6x8, White, ALIGN_LEFT);
		sh1106_UpdateScreen();
		if(sim_set_TCP_connection()){
			sh1106_WriteString(2, 10, "TCP_OK", Font_6x8, White, ALIGN_RIGHT);
			sh1106_WriteString(2, 20, "-SERVER CONNECT:", Font_6x8, White, ALIGN_LEFT);
			sh1106_UpdateScreen();
			if (sim_connect_server(sim800,10,5000)){	//Ket noi server thanh cong
				sh1106_WriteString(2, 20, "OK", Font_6x8, White, ALIGN_RIGHT);
				sh1106_WriteString(2, 30, "-BROKER CONNECT:", Font_6x8, White, ALIGN_LEFT);
				sh1106_UpdateScreen();
				delay_ms(500);
				if (MQTT_Connect(sim800)){	//Ket noi MQTT Broker thanh cong
					sh1106_WriteString(2, 30, "OK", Font_6x8, White, ALIGN_RIGHT);
					sh1106_UpdateScreen();
					delay_ms(500);
					first_pub_topic(topicString);
				}
				else{	//ket noi mqtt broker fail
					sh1106_WriteString(2, 30, "FAIL", Font_6x8, White, ALIGN_RIGHT);
					sh1106_UpdateScreen();
				}
			}
			else {	//Ket noi server khong thanh cong
				sh1106_WriteString(2, 20, "FAIL", Font_6x8, White, ALIGN_RIGHT);
				sh1106_UpdateScreen();
			}
		}
		else {
			sh1106_WriteString(2, 10, "TCP_FAIL:", Font_6x8, White, ALIGN_RIGHT);
			sh1106_UpdateScreen();
		}
	}
	//khoi tao sim khong thanh cong
	sim_error_handler();
	t_check_connection = millis();
#endif
}
static void lcd_start()
{
	sh1106_Init();
	sh1106_DrawBitmap(2, 0, Screen);
	sh1106_UpdateScreen();
	delay_ms(1000);
	sh1106_Clear(Black);
	sh1106_UpdateScreen();
	sh1106_DrawBitmap(33, 0, Gps_logo);
	sh1106_WriteString(2,40,"GPS TRACKING", Font_6x8, White,ALIGN_CENTER);
	sh1106_WriteString(2,50,"STARTING SYSTEM", Font_6x8, White, ALIGN_CENTER);
	sh1106_UpdateScreen();
	delay_ms(1000);
	sh1106_Clear(Black);
	sh1106_UpdateScreen();
	sh1106_WriteString(2, 0, "CHECK SIM:", Font_6x8, White, ALIGN_LEFT);
	sh1106_UpdateScreen();
}
static void lcd_update(){
	sim_power_status(sim800);
	sh1106_DrawLine(2, 15, 130, 15, White);
	char buf[20]={0};
	if (sim800->signal_condition<=NOSIGNAL) sh1106_DrawBitmap(2, 0, Gsm_signal[4]);
	if (sim800->signal_condition==MARGINAL) sh1106_DrawBitmap(2, 0, Gsm_signal[3]);
	if (sim800->signal_condition==OK) sh1106_DrawBitmap(2, 0, Gsm_signal[2]);
	if (sim800->signal_condition==GOOD) sh1106_DrawBitmap(2, 0, Gsm_signal[1]);
	if (sim800->signal_condition==EXCELLENT) sh1106_DrawBitmap(2, 0, Gsm_signal[0]);
	if (sim800->tcp_connect == true) sh1106_DrawBitmap(25, 0, Gsm_signal[5]);
	else sh1106_DrawBitmap(25, 0, Gsm_signal[6]);
	if (sim800->mqttServer.connect==true) sh1106_DrawBitmap(55, 0, Server_connect[0]);
	else sh1106_DrawBitmap(55, 0, Server_connect[1]);
	if (gps_l70->RMC.Data_Valid[0]!='V') sh1106_DrawBitmap(40, 0, Gps_signal[0]);
	else sh1106_DrawBitmap(40, 0, Gps_signal[1]);
	sprintf(buf,"Temperature: %d *C",(int)ds18b20.temp);
	sh1106_WriteString(2, 17, buf, Font_7x10, White, ALIGN_LEFT);
	sh1106_UpdateScreen();
}
static void gps_start()
{
	if (sim800->power_state == OFF) {
		gps_l70->gps_err = GPS_NO_PWR;
		gps_l70->gps_pwr_state = false;
	}
	else {
		gps_l70->gps_pwr_state = true;
	}
	sh1106_WriteString(2, 40, "CHECK_GPS", Font_6x8, White, ALIGN_LEFT);
	if (gps_l70->gps_err == GPS_NO_PWR){
		sh1106_WriteString(2, 40, "NO_POWER", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (gps_l70->gps_err == GPS_NO_RES){
		sh1106_WriteString(2, 40, "NO_RES", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
	if (gps_l70->gps_err == GPS_NO_ERR){
		sh1106_WriteString(2, 40, "NO_ERROR", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
		return;
	}
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
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
