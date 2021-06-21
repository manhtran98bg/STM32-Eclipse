// ----------------------------------------------------------------------------
#include <main.h>
/*Include header file */

#include "sdcard/ff.h"
#include "sdcard/sdmm.h"
#include "simcom/sim800.h"
#include "service/delay.h"
#include "power/power.h"
#include "sensor/ds18b20.h"
#include "sensor/onewire.h"
#include "usart/usart.h"
#include "rfid/mfrc552.h"
#include "gps/gps.h"
#include "rtc/rtc.h"
#include "lcd/sh1106.h"
#include "board/board.h"
#include "voltage/adc.h"
#include "rs232/rs232.h"
// ----------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
// ----------------------------------------------------------------------------
#define _USE_SIM	1
#define _USE_LCD	0
#define _USE_SDCARD	1

// ----------------------------------------------------------------------------
u8 status;

// Khai bao cac bien cho SIM800 && MQTT broker
const char IP_Address[]="broker.emqx.io";
//const char IP_Address[]="broker.hivemq.com";
uint16_t port=1883;

SIM800_t sim800;	//Struct SIM800
gps_t gps_l70;		//Struct GPS L70
//Struct RTC
RTC_Time_t Time;
struct tm time_struct;
//Struct SDCard
FATFS	FatFs;
FIL	Fil;

//Struct Sensor
ds18b20_t ds18b20[EXPECTING_SENSORS];
TM_OneWire_t OneWire1;

//Struct RFID
RFID_t rfid;
//Khai bao cac mang luu topic publish va subcribe can khoi tao truoc.
MQTTString pub_topicList[40] = MQTTString_initializer;	//Struct array luu topic Pub
char pub_topic[25][100]={{0}};		//Pub Topic Buffer Array
MQTTString sub_topicList[40] = MQTTString_initializer;	//Struct array luu topic Sub
char sub_topic[10][100]={{0}};		//Sub Topic Buffer Array
//Khai bao cac mang luu cac topic va payload nhan duoc tu broker.
unsigned char topic[6][128]= {{0}};
unsigned char payload[6][32]={{0}};
int freq_array[6]={5,5,5,5,5,5};	//Default Interval Update Data to Broker
int requestedQoSs[2]={0};

char json_geowithtime[100]={0};		//Buffer Location and Time-Date
char time_buffer[10]={0};
uint32_t t_check_connection = 0;	//Interval to check connection with broker, default = 10s
uint32_t t_lcd_update = 0;			//Interval to update LCD, default = 2s
uint8_t nosignal_check = 0;			//Flag indicate checked no signal condition.
bool _1sflag = false;				//RTC 1s update flag.
unsigned char mqtt_buffer[NUM_SUB_TOPIC][256] = {0};
float vbat = 0;
bool sub_topic_rx_data_flag = false;
bool board_state = 0;
uint32_t time_ping_sver=0;
uint32_t time_read_data=0;
uint32_t time_pub[8]={0};
uint32_t ping_interval = 10000;
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
static void board_init();
static void sim_start();
static void lcd_start();
static void gps_start();
static void lcd_update();

void init_var();
void init_pub_topic(MQTTString *topicList,char *topic_buff, char *IMEI);
void init_sub_topic(MQTTString *topicList,char *topic_buff, char *IMEI);
void first_pub_topic(MQTTString *topicList);
uint8_t first_sub_topic (MQTTString *topicList);

void read_data_sensor_handler();
void rx_data_subtopic_handler();
void rfid_handler();
void pub_data_handler();
bool rfid_state = false;
int main(int argc, char* argv[])
{

	char payload_buf[5]={0};
	//Init first variable.
	init_var();
	board_init();
#if _USE_LCD
	lcd_start();
#endif
	sim_start();
	gps_start();
#if _USE_LCD
	delay_ms(1000);
	sh1106_Clear(Black);
	sh1106_UpdateScreen();
#endif
	for (int i=0;i<7;i++) time_pub[i] = millis();
	time_ping_sver =millis();
	time_read_data =millis();
	sub_topic_index = 0;
	board_state = true;
	uint8_t ping_cnt = 0;
	while(1)
	{
#if _USE_SIM
		if (millis()-time_ping_sver>ping_interval)
		{
			time_ping_sver = millis();
			if(MQTT_PingReq(&sim800)) {		//reset cac thong so, xac nhan van co ket noi voi broker
				#if _USE_DEBUG_UART
					debug_send_string("log: PING OK..\n");
				#endif
				sim800.mqttServer.ping_flag = true;
				ping_cnt = 0;
				ping_interval = 10000;
			}
			else {		//tang so lan ping, neu ping_count = 5 => reconnect;
				#if _USE_DEBUG_UART
					debug_send_string("log: PING FAIL..\n");
				#endif
				sim800.mqttServer.ping_flag = false;
				ping_interval = 2000;
				ping_cnt++;
			}
			if (ping_cnt>=5){
				#if _USE_DEBUG_UART
					debug_send_string("log: Run Reconnect Handler.\n");
				#endif
				sim_disconnect_server(&sim800);
				if (sim_detach_gprs(5, 1000)){
					sim_attach_gprs(5, 1000);
					if (sim_connect_server(&sim800, 5, 1000)) {
						MQTT_Connect(&sim800);
						first_pub_topic(pub_topicList);
						first_sub_topic(sub_topicList);
						ping_cnt = 0;
					}
				}
			}
		}
		if (millis()-time_read_data>2000){	//2S doc data 1 lan
			time_read_data=millis();
			read_data_sensor_handler();
		}
		if (sim800.mqttServer.connect==true){
			pub_data_handler();
		}
        if (sub_topic_rx_data_flag == true){
        	rx_data_subtopic_handler();
        }
#endif
		if (rfid.t_out==0){
			rfid_handler();
		}
	}
	return 0;
}
void rfid_handler()
{
	char sd_buffer[128]={0};
	char log[32]={0};
	uint8_t str[16]; // Max_LEN = 16
	uint8_t serNum[5];
	rfid.present = false;
	status = MFRC522_Request(PICC_REQIDL, (uchar*)str);
	if (status == MI_OK) {
#if _USE_DEBUG_UART
		debug_send_string("Find out a card:");
#endif
		buzzer_on();
		status = MFRC522_Anticoll(str);
		memcpy(serNum, str, 5);
		for (int i=0;i<5;i++) rfid.serialNumber[i]=serNum[i];
		date_time2str(sd_buffer, &time_struct);
		sprintf(&sd_buffer[strlen(sd_buffer)]," ID=%x%x%x%x%x\r\n",
				serNum[0], serNum[1], serNum[2], serNum[3],serNum[4]);
		sprintf(log,"%x%x%x%x%x",serNum[0], serNum[1], serNum[2], serNum[3],serNum[4]);
		MQTT_Pub(pub_topicList[21].cstring, log);
#if _USE_DEBUG_UART
		debug_send_string(log);
		debug_send_chr('\n');
#endif
		if (sdcard.mount == true) write2file(directory, strlen(directory), (char*)"ID.txt",sd_buffer,strlen(sd_buffer));
		rfid.t_out = 3;
		MFRC522_Halt();
		rfid.present = true;
		rfid_state = true;
	}
}
void rx_data_subtopic_handler()
{
	int i=0,j=0;
	char buffer[256]={0};
	char time_str_buffer[20]={0};
	if(sub_topic_index>=NUM_SUB_TOPIC||(millis()-timeout_rx_topic)>=3000) {//Nhan dc cac Setup Topic tu Broker
		if (sub_topic_index == 0)return;
		create_time_str(&Time, time_str_buffer);
		mqtt_receive = 0;
		memset(topic,0,6*128);
		memset(payload,0,6*32);
		for (i=0; i<sub_topic_index;i++){
			if (strlen(mqtt_buffer[i])>0) MQTT_Receive((unsigned char*)mqtt_buffer[i]);
			if(sim800.mqttReceive.topicLen>4)
				memcpy(topic[i],sim800.mqttReceive.topic,sim800.mqttReceive.topicLen-4);
			if(sim800.mqttReceive.payloadLen>0)
				memcpy(payload[i],sim800.mqttReceive.payload,sim800.mqttReceive.payloadLen);
			sprintf(buffer,"%s: %s   %s\n",time_str_buffer,topic[i],payload[i]);
			write2file(directory, strlen(directory), "REMOTE.LOG", buffer, strlen(buffer));
			delay_ms(10);
			if (rfid_state == false){
				for (j=0;j<6;j++)
					if (strcmp((char*)topic[i],pub_topicList[j+15].cstring)==0){
						MQTT_Pub((char*)topic[i],(char*) payload[i]);
						if (strstr((char*)payload[i],"undefined")) break;
						else freq_array[j]=atoi((char*)payload[i]);
						break;
					}
			}
			else {
				if (strcmp((char*)topic[i],pub_topicList[21].cstring) == 0 && strstr((char*)payload[i],"true")){
					for (int x = 0;x<3;x++){
						buzzer_on();
						delay_ms(300);
					}
				}
				else {
					for (int x = 0;x<2;x++){
						buzzer_on();
						delay_ms(100);
					}
				}
				rfid_state = false;
			}
		}
		clearMqttBuffer();
		sub_topic_rx_data_flag = false;
	}
}
void read_data_sensor_handler()
{
	ds18b20_read_temp(&OneWire1, ds18b20);
	get_vbat(&vbat);
	sim800.signal_condition = sim_check_signal_condition(&sim800, 500);

}
void pub_data_handler()
{
	char payload_buf[20]={0};
	if ((millis()-time_pub[0])>=freq_array[0]*1000){
		time_pub[0] = millis();
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub Device Temp\n");
		#endif
		sprintf(payload_buf,"%d",(int)ds18b20[0].temp);
		MQTT_Pub(pub_topicList[12].cstring,payload_buf);	//Temp Device
	}
	if ((millis()-time_pub[1])>=freq_array[1]*1000){
		time_pub[1] = millis();
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub Enviroment Temp\n");
		#endif
		sprintf(payload_buf,"%d",(int)ds18b20[1].temp);
		MQTT_Pub(pub_topicList[9].cstring,payload_buf);	//Temp Enviroment
	}
	if ((millis()-time_pub[2])>=freq_array[2]*1000){
		time_pub[2] = millis();
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub Enviroment Humidity\n");
		#endif
	}
	if ((millis()-time_pub[3])>=freq_array[3]*1000){
		time_pub[3] = millis();
		if (gps_l70.RMC.Data_Valid[0]!='V') MQTT_Pub(pub_topicList[11].cstring,json_geowithtime);
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub Device Location\n");
		#endif
	}
	if ((millis()-time_pub[4])>=freq_array[4]*1000){
		time_pub[4] = millis();
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub Batery voltage\n");
		#endif
		sprintf(payload_buf,"%d",(int)vbat);
		MQTT_Pub(pub_topicList[13].cstring,payload_buf);	//RSSI
	}
	if ((millis()-time_pub[5])>=freq_array[5]*1000){
		time_pub[5] = millis();
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub Vehicle RPM\n");
		#endif
	}
	if ((millis()-time_pub[6])>=10000){
		time_pub[6] = millis();
		#ifdef _USE_DEBUG_UART
			debug_send_string("log: Pub RSSI\n");
		#endif
		MQTT_Pub(pub_topicList[7].cstring,sim800.rssi);	//RSSI
	}


}
void init_var()
{
	sim800.sim.apn = "m-wap";
	sim800.sim.apn_user = "mms";
	sim800.sim.apn_pass = "mms";
	sim800.mqttServer.host = (char*)IP_Address;
	sim800.mqttServer.port = port;
	sim800.mqttServer.connect = false;
	sim800.mqttClient.username = "user";
	sim800.mqttClient.pass = "user";
	sim800.mqttClient.clientID = "Client01";
	sim800.mqttClient.keepAliveInterval = 120;
	sim800.power_state = OFF;
	sim800.tcp_connect = false;
	sim800.signal_condition = NOSIGNAL;
	sim800.sim_err = NO_PWR;
	sim800.sim_id.model_id ="SIMCOM_SIM800C";
	sim800.sim_id.manufacturer_id = "SIMCOM_Ltd";
	sim800.send_state = NO_SEND;
	gps_l70.gps_pwr_state = false;
	gps_l70.gps_err = GPS_NO_PWR;
	Time.old_minute = 0;
	_1sflag = true;
	rfid.t_out = 0;
	rfid.present = false;
	Time.old_day = 0;
}
void init_pub_topic(MQTTString *topicList,char *topic_buff, char *IMEI){

	sprintf(topic_buff,"mandevices/GSHT_%s/$name",IMEI);
	topicList[0].cstring = topic_buff;

	sprintf(topic_buff+100,"mandevices/GSHT_%s/$state",IMEI);
	topicList[1].cstring = topic_buff+100;

	sprintf(topic_buff+200,"mandevices/GSHT_%s/$homie",IMEI);
	topicList[2].cstring = topic_buff+200;

	sprintf(topic_buff+300,"mandevices/GSHT_%s/$node",IMEI);
	topicList[3].cstring = topic_buff+300;

	sprintf(topic_buff+400,"mandevices/GSHT_%s/sim/$state",IMEI);
	topicList[4].cstring = topic_buff+400;

	sprintf(topic_buff+500,"mandevices/GSHT_%s/sim/manufacturer_id",IMEI);
	topicList[5].cstring = topic_buff+500;

	sprintf(topic_buff+600,"mandevices/GSHT_%s/sim/model_id",IMEI);
	topicList[6].cstring = topic_buff+600;

	sprintf(topic_buff+700,"mandevices/GSHT_%s/sim/signal_strength",IMEI);
	topicList[7].cstring = topic_buff+700;

	sprintf(topic_buff+800,"mandevices/GSHT_%s/sim/imei",IMEI);
	topicList[8].cstring = topic_buff+800;

	sprintf(topic_buff+900,"mandevices/GSHT_%s/environment/temperature",IMEI);
	topicList[9].cstring = topic_buff+900;

	sprintf(topic_buff+1000,"mandevices/GSHT_%s/environment/humidity",IMEI);
	topicList[10].cstring = topic_buff+1000;

	sprintf(topic_buff+1100,"mandevices/GSHT_%s/device/location",IMEI);
	topicList[11].cstring = topic_buff+1100;

	sprintf(topic_buff+1200,"mandevices/GSHT_%s/device/temperature",IMEI);
	topicList[12].cstring = topic_buff+1200;

	sprintf(topic_buff+1300,"mandevices/GSHT_%s/battery/rate_voltage",IMEI);
	topicList[13].cstring = topic_buff+1300;

	sprintf(topic_buff+1400,"mandevices/GSHT_%s/vehicle/rpm",IMEI);
	topicList[14].cstring = topic_buff+1400;

	sprintf(topic_buff+1500,"mandevices/GSHT_%s/device/temperature/$freq",IMEI);
	topicList[15].cstring = topic_buff+1500;

	sprintf(topic_buff+1600,"mandevices/GSHT_%s/environment/temperature/$freq",IMEI);
	topicList[16].cstring = topic_buff+1600;

	sprintf(topic_buff+1700,"mandevices/GSHT_%s/environment/humidity/$freq",IMEI);
	topicList[17].cstring = topic_buff+1700;

	sprintf(topic_buff+1800,"mandevices/GSHT_%s/battery/rate_voltage/$freq",IMEI);
	topicList[18].cstring = topic_buff+1800;

	sprintf(topic_buff+1900,"mandevices/GSHT_%s/device/location/$freq",IMEI);
	topicList[19].cstring = topic_buff+1900;

	sprintf(topic_buff+2000,"mandevices/GSHT_%s/vehicle/rpm/$freq",IMEI);
	topicList[20].cstring = topic_buff+2000;

	sprintf(topic_buff+2100,"mandevices/GSHT_%s/card/code",IMEI);
	topicList[21].cstring = topic_buff+2100;
}
void init_sub_topic(MQTTString *topicList,char *topic_buff, char *IMEI){
	sprintf(topic_buff,"mandevices/GSHT_%s/device/temperature/$freq/set",IMEI);
	topicList[0].cstring = topic_buff;
	sprintf(topic_buff+100,"mandevices/GSHT_%s/environment/temperature/$freq/set",IMEI);
	topicList[1].cstring = topic_buff+100;
	sprintf(topic_buff+200,"mandevices/GSHT_%s/environment/humidity/$freq/set",IMEI);
	topicList[2].cstring = topic_buff+200;
	sprintf(topic_buff+300,"mandevices/GSHT_%s/battery/rate_voltage/$freq/set",IMEI);
	topicList[3].cstring = topic_buff+300;
	sprintf(topic_buff+400,"mandevices/GSHT_%s/device/location/$freq/set",IMEI);
	topicList[4].cstring = topic_buff+400;
	sprintf(topic_buff+500,"mandevices/GSHT_%s/vehicle/rpm/$freq/set",IMEI);
	topicList[5].cstring = topic_buff+500;
	sprintf(topic_buff+600,"mandevices/GSHT_%s/card/code/set",IMEI);
	topicList[6].cstring = topic_buff+600;
}
void first_pub_topic(MQTTString *topicList)
{
	char buf[3]={0};
	int i = 0;
	char payload_buffer[7][60]={{"Giam sat hanh trinh"},
			{"ready"},{"4.0.0"},{"enviroment,device,battery,vehicle,driver,gps,sim"},
			{"ready"},{"SIMCOM_Ltd"},{"SIMCOM_SIM800C"}};
	for (i=0; i<7;i++)
	{
		MQTT_Pub(topicList[i].cstring, payload_buffer[i]);
		delay_ms(10);
	}
	MQTT_Pub(topicList[8].cstring, sim800.sim_id.imei);
	delay_ms(10);
	for (i=0;i<6;i++)
	{
		itoa(freq_array[i],buf,10);
		MQTT_Pub(topicList[i+15].cstring,buf);
		delay_ms(10);
	}
}
uint8_t first_sub_topic (MQTTString *topicList)
{
	uint8_t	count=0;
	char buf[64]={0};
	if (MQTT_Sub(topicList, requestedQoSs, 2)) count = count +2;
	delay_ms(10);
	if (MQTT_Sub(&topicList[2], requestedQoSs, 2)) count = count +2;
	delay_ms(10);
	if (MQTT_Sub(&topicList[4], requestedQoSs, 2)) count = count +2;
	if (MQTT_Sub(&topicList[6], 0, 1)) count = count +1;
	sprintf(buf,"log: Number of Topic Subcribed = %d\n",count);
#if _DEBUG
	trace_puts(buf);
#endif
#if _USE_DEBUG_UART
	debug_send_string(buf);
#endif
	return count;
}
static void sim_start(){
#if _USE_SIM
	if(sim_init(&sim800)){	//Khoi tao sim thanh cong
#if _USE_LCD
		sim_error_handler();
#endif
		init_pub_topic(pub_topicList, (char*)pub_topic, (char*)sim800.sim_id.imei);
		init_sub_topic(sub_topicList, (char*)sub_topic, (char*)sim800.sim_id.imei);
		sim800.mqttClient.clientID = sim800.sim_id.imei;
#if _USE_LCD
		sh1106_WriteString(2, 10, "-SETUP TCP:", Font_6x8, White, ALIGN_LEFT);
		sh1106_UpdateScreen();
#endif
		if(sim_set_TCP_connection()){
#if _USE_LCD
			sh1106_WriteString(2, 10, "TCP_OK", Font_6x8, White, ALIGN_RIGHT);
			sh1106_WriteString(2, 20, "-SERVER CONNECT:", Font_6x8, White, ALIGN_LEFT);
			sh1106_UpdateScreen();
#endif
			if (sim_connect_server(&sim800,10,5000)){	//Ket noi server thanh cong
#if _USE_LCD
				sh1106_WriteString(2, 20, "OK", Font_6x8, White, ALIGN_RIGHT);
				sh1106_WriteString(2, 30, "-BROKER CONNECT:", Font_6x8, White, ALIGN_LEFT);
				sh1106_UpdateScreen();
#endif
				delay_ms(500);
				if (MQTT_Connect(&sim800)){	//Ket noi MQTT Broker thanh cong
#if _USE_LCD
					sh1106_WriteString(2, 30, "OK", Font_6x8, White, ALIGN_RIGHT);
					sh1106_UpdateScreen();
#endif
					delay_ms(500);
#if _DEBUG
					trace_puts("Publish First Topic to Broker");
#endif
#if _USE_DEBUG_UART
					debug_send_string("log: Publish First Topics to Broker\n");
#endif
					first_pub_topic(pub_topicList);
#if _DEBUG
					trace_puts("Subcribe First Topic to Broker");
#endif
#if _USE_DEBUG_UART
					debug_send_string("log: Subcribe First Topics to Broker\n");
#endif
					first_sub_topic(sub_topicList);
				}
				else{	//ket noi mqtt broker fail
#if _USE_LCD
					sh1106_WriteString(2, 30, "FAIL", Font_6x8, White, ALIGN_RIGHT);
					sh1106_UpdateScreen();
#endif
				}
			}
			else {	//Ket noi server khong thanh cong
#if _USE_LCD
				sh1106_WriteString(2, 20, "FAIL", Font_6x8, White, ALIGN_RIGHT);
				sh1106_UpdateScreen();
#endif
			}
		}
		else {
#if _USE_LCD
			sh1106_WriteString(2, 10, "TCP_FAIL:", Font_6x8, White, ALIGN_RIGHT);
			sh1106_UpdateScreen();
#endif
		}
	}
	//khoi tao sim khong thanh cong
#if _USE_LCD
	sim_error_handler();
	t_check_connection = millis();
#endif
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
	sim_power_status(&sim800);
	sh1106_DrawLine(2, 15, 130, 15, White);
	char buf[20]={0};
	if (sim800.signal_condition<=NOSIGNAL) sh1106_DrawBitmap(2, 0, Gsm_signal[4]);
	if (sim800.signal_condition==MARGINAL) sh1106_DrawBitmap(2, 0, Gsm_signal[3]);
	if (sim800.signal_condition==OK) sh1106_DrawBitmap(2, 0, Gsm_signal[2]);
	if (sim800.signal_condition==GOOD) sh1106_DrawBitmap(2, 0, Gsm_signal[1]);
	if (sim800.signal_condition==EXCELLENT) sh1106_DrawBitmap(2, 0, Gsm_signal[0]);
	if (sim800.tcp_connect == true) sh1106_DrawBitmap(25, 0, Gsm_signal[5]);
	else sh1106_DrawBitmap(25, 0, Gsm_signal[6]);
	if (sim800.mqttServer.connect==true) sh1106_DrawBitmap(55, 0, Server_connect[0]);
	else sh1106_DrawBitmap(55, 0, Server_connect[1]);
	if (gps_l70.RMC.Data_Valid[0]!='V') sh1106_DrawBitmap(40, 0, Gps_signal[0]);
	else sh1106_DrawBitmap(40, 0, Gps_signal[1]);
//	sprintf(buf,"Temperature: %d *C",(int)ds18b20.temp);
	sh1106_WriteString(2, 17, buf, Font_7x10, White, ALIGN_LEFT);
	sh1106_UpdateScreen();
}
static void gps_start()
{

#if _USE_LCD
	sh1106_WriteString(2, 40, "CHECK_GPS", Font_6x8, White, ALIGN_LEFT);
#endif
	if (gps_l70.gps_err == GPS_NO_PWR){
#if _USE_LCD
		sh1106_WriteString(2, 40, "NO_POWER", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
#endif
		return;
	}
	if (gps_l70.gps_err == GPS_NO_RES){
#if _USE_LCD
		sh1106_WriteString(2, 40, "NO_RES", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
#endif
		return;
	}
	if (gps_l70.gps_err == GPS_NO_ERR){
#if _USE_LCD
		sh1106_WriteString(2, 40, "NO_ERROR", Font_6x8, White, ALIGN_RIGHT);
		sh1106_UpdateScreen();
#endif
		return;
	}
}
static void sdcard_check()
{
	TCHAR label[10];
	DWORD	vsn;
	FRESULT	fr;
	char log[128]={0};
	sdcard.mount =false;
	fr = f_mount(&FatFs, "", 1);
	if(fr == FR_OK){
		f_getlabel("", label, &vsn);
		sdcard.mount = true;
		sdcard.serNum = vsn;
		strcpy(sdcard.label,label);
		sprintf(log,"log: Mount SDCard OK. Label=%s. Serial Number=%ld\n",label,vsn);
#if	_DEBUG
		trace_write(log,strlen(log));
#endif
#if _USE_DEBUG_UART
		debug_send_string(log);
#endif
	}
	else {
#if	_DEBUG
		trace_puts("log: Mount SDCard Fail.");
#endif
#if _USE_DEBUG_UART
		debug_send_string("log: Mount SDCard Fail.\n");
#endif
	}

}
static void board_init()
{
	clk_init();
	usart_init();
	RTC_Init();
	user_led_init();
	btn_init();
	tim4_init();
	power_reset_sim();
#if _USE_SIM
	sim_gpio_init();
	sim_power_off(&sim800);
#endif
	rs232_init();
	adc_init();
	MFRC522_Init();
	ds18b20_init(&OneWire1, ds18b20);
#if _USE_SIM
	sim_power_on(&sim800);
#endif
	sim800.power_state = ON;
	gps_init(&gps_l70);
#if _USE_SDCARD
	sdcard_check();
	create_time_str(&Time, time_str);
	delay_ms(1000);
	sprintf(sd_buffer,"%s: RESTART\n",time_str);
	write2file(directory, strlen(directory), "DEVICE.LOG", sd_buffer, strlen(sd_buffer));
	memset(sd_buffer,0,128);
#endif
}
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
