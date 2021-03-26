#ifndef SWITCH_H_
#define SWITCH_H_

#include "../src/board.h"

typedef struct Switch_t Switch;
typedef enum SWITCH_STATE_CONST {
	STATE_ON	=0x00,
	STATE_OFF	=0x01
}SWITCH_STATE;


void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void switch_init(Switch* p_switch,const uint8_t id,
		const SWITCH_STATE state,const bool reversed);
void switch_on(Switch* p_switch);
void switch_off(Switch* p_switch);
void switch_toggle(Switch* p_switch);
SWITCH_STATE switch_get_state(Switch* p_switch);
void switch_read_state(Switch* p_switch);

struct Switch_t{
	uint8_t id;
	bool is_reversed;
	SWITCH_STATE state;
	uint8_t blink_interval;
	bool is_changed;
	bool is_remote_control;
	uint8_t counter;
};

#endif
