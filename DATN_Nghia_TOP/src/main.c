
#include "main.h"
#include "service/delay.h"
#include "board.h"
#include "sensor/ds18b20.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
//Struct Sensor
int main(int argc, char* argv[])
{
	rcc_config();
	user_led_config();
	tim4_init();
	ds18_config(RES_9BIT);
	while(1){
		user_led_toggle();
		ds18_read_temp(&ds18b20);
		delay_ms(1000);
	}
	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
