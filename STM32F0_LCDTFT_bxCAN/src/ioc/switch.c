#include "switch.h"
#include "actuator_io_hardware.h"

void delay_ms(uint32_t ms)
{
	msTicks=ms;
	while(msTicks);
}

void delay_us(uint32_t us)
{
	us*=7;
	while(us--);
}
static void switch_set_state(Switch* p_switch,const SWITCH_STATE state);
static uint8_t switch_get_level(Switch* p_switch);


void switch_init(Switch* p_switch,const uint8_t id,
		const SWITCH_STATE state,const bool reversed){
	p_switch->id=id;
	p_switch->state=state;
	p_switch->is_reversed=reversed;
	p_switch->blink_interval=0;
	p_switch->counter = 0;
}

void switch_on(Switch* p_switch){
	switch_set_state(p_switch,STATE_ON);
}

void switch_off(Switch* p_switch){
	switch_set_state(p_switch,STATE_OFF);
}

SWITCH_STATE switch_get_state(Switch* p_switch) //lay trang thai nut nhan
{
	return p_switch->state;
}

static void switch_set_state(Switch* p_switch,const SWITCH_STATE state){
	if(p_switch->is_reversed==false){
		if(state==STATE_ON)
			set_gpio_out(p_switch->id);
		else
			clear_gpio_out(p_switch->id);
	}
	else{
		if(state==STATE_ON)
			clear_gpio_out(p_switch->id);
		else
			set_gpio_out(p_switch->id);

	}
	p_switch->state=state;
}

static uint8_t switch_get_level(Switch* p_switch)
{
	uint8_t debounce=0; //Doi phim
	uint8_t switch_level=0;
	while(debounce<3)
	{
		switch_level+= read_gpio_in(p_switch->id);
		delay_us(10);
		debounce++;
	}
	if(switch_level >= (debounce-1)) {
		switch_level=1;
	}
	else
		switch_level=0;
	return switch_level;
}

void switch_read_state(Switch* p_switch) //ham doc trang thai nut nhan
{
	SWITCH_STATE old_state =  switch_get_state(p_switch);
	uint8_t switch_level = switch_get_level(p_switch);
	if(p_switch->is_reversed==false){
		if(switch_level==1)
			p_switch->state=STATE_ON;
		else
			p_switch->state=STATE_OFF;
	}
	else{
		if(switch_level==1)
			p_switch->state=STATE_OFF;
		else
			p_switch->state=STATE_ON;
	}
	if(p_switch->state != old_state){
	   	    p_switch->is_changed = true;
	   	    p_switch->is_remote_control = false;
	}
	else
	   	    p_switch->is_changed = false;
}

void switch_toggle(Switch* p_switch){
	if(p_switch->state==STATE_ON){
		switch_off(p_switch);
	}
	else
	{
		switch_on(p_switch);
	}
}
