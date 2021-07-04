
// ----------------------------------------------------------------------------

#include "main.h"
#include "board.h"
#include "service/delay.h"
#include "usart/usart.h"
#include "hc05/hc05.h"
#include "board.h"
#include "adc/adc.h"
bool spk_state = false;
uint8_t cnt_temp = 0;
uint8_t cnt_dust = 0;
uint8_t cnt_distance = 0;
bool temp_flag = false;
bool dust_flag = false;
bool distance_flag = false;
bool stop_flag = false;
bool current_flag = false;
uint32_t time_read_sensor = 0;
uint32_t time_beep = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
int main(int argc, char* argv[])
 {
	float current;
	clock_config();
	hc05_init();
	user_led_init();
	usart_init();
	btn_config();
	output_pin_init();
	adc_init();
	relay_on();
	time_read_sensor = millis();
	while(1){
		if (spk_state == false && stop_flag == false){
			get_current_load(&current);
			if (current>5) {//Qua dong
				debug_send_string("log: Qua dong => Ngat dien\n");
				relay_off();
				current_flag = true;
				spk_state = true;
			}
			if (millis()-time_read_sensor>950){
				if (hc05_data.temp>50) cnt_temp ++;
				else cnt_temp = 0;
				if (hc05_data.dust>550) cnt_dust ++;
				else cnt_dust = 0;
				if (hc05_data.range<120) cnt_distance ++;
				else cnt_distance = 0;
				time_read_sensor = millis();
			}
			if (cnt_temp>15) temp_flag =true;
			else temp_flag = false;
			if (cnt_dust>15) dust_flag =true;
			else dust_flag = false;
			if (cnt_distance>15) distance_flag =true; //co nguoi
			else distance_flag = false;	//khong co nguoi
			if (temp_flag || dust_flag ){
				if (temp_flag) debug_send_string("log: Qua nhiet\n");
				else if (dust_flag) debug_send_string("log: Phat hien khoi\n");
				if (distance_flag == true){ //Co nguoi
					debug_send_string("log: Co nguoi => bat coi canh bao\n");
					spk_state = true;
					time_beep = millis();
				}
				else {//Khong co nguoi
					debug_send_string("log: Khong co nguoi => bat coi canh bao + Tat relay\n");
					relay_off();
					spk_state = true;
					time_beep = millis();
				}
			}
		}
		if (spk_state==true){
			buzzer_beep(50);
			delay_ms(100);
			if (millis()-time_beep>15000){
				debug_send_string("log: Auto Stop\n");
				if (distance_flag==true) {
					debug_send_string("log: Tat relay\n");
					relay_off();
				}
				spk_state = false; //stop beep
				stop_flag = true; //stop_system
			}
		}
		if(stop_flag){
			 spk_state = false;
			 temp_flag = false;
			 dust_flag = false;
			 distance_flag = false;
			 cnt_temp = 0;
			 cnt_dust = 0;
			 cnt_distance = 0;
		}
	}
	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
