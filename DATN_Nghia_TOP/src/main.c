
#include "ds18b20/ds18b20.h"
#include "main.h"
#include "service/delay.h"
#include "board.h"
#include "hc05/hc05.h"
#include "usart/usart.h"
#include "sr04/sr04.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
int dust = 0;
int main(int argc, char* argv[])
{
	char buf[32]={0};
	rcc_config();
	user_led_config();
	usart_init();
	tim4_init();
	sr04_init();
	hc05_init();
	delay_ms(100);
	ds18_config(RES_9BIT);
	delay_ms(1000);
	while(1){
		user_led_toggle();
		ds18_read_temp(&ds18b20);
		range = sr04_get_distance();
		sprintf(buf,"$DATA,%d,%d,%d\n",(int)ds18b20.temp,range,dust);
		hc05_send_cmd(buf);
		delay_ms(500);
	}
	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
