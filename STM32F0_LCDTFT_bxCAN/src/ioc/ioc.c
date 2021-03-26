#include "ioc.h"
#include "switch.h"
#include "actuator_io_hardware.h"
#include "../src/board.h"

extern volatile uint8_t ioc_tick ;
static uint32_t on_vib_counter = 0;
uint8_t horn_count=0;

bool key_state = false;
bool _is_turn_on_motor = false;
uint8_t turn_on_counter = 0;
static void ioc_switch_in_init(ioc_t* p_ioc);
static void ioc_switch_out_init(ioc_t* p_ioc);
static void ioc_scan_input(ioc_t* p_ioc);
static void ioc_update_output(ioc_t* p_ioc);
static void ioc_init_switch(ioc_t* p_ioc);
static void io_switch_set_state(ioc_t* p_ioc, uint8_t index);
static void ioc_process_output(ioc_t* p_ioc);
static bool is_key_on(ioc_t* p_ioc);
static void ioc_turn_on_vib_motor(ioc_t* p_ioc);
static void ioc_turn_off_vib_motor(ioc_t* p_ioc);

static void ioc_switch_in_init(ioc_t* p_ioc) {
        uint8_t input_index = 0;
        Switch* p_switch = &p_ioc->actuator_buffer[0];
        for (input_index = 0; input_index < MAX_APP_INPUT_NUM; input_index++) {
                p_switch = &p_ioc->actuator_buffer[input_index];
                switch_init(p_switch, input_index, STATE_OFF, true);
                p_switch->is_remote_control = false;
                p_switch->is_changed = false;
        }
}

static void ioc_switch_out_init(ioc_t* p_ioc) {
        uint8_t output_index = 0;
        Switch* p_switch = &p_ioc->actuator_control[0];
        for (output_index = 0; output_index < MAX_APP_OUTPUT_NUM; output_index++) {
                p_switch = &p_ioc->actuator_control[output_index];
                switch_init(p_switch, output_index, STATE_OFF, false);
                switch_off(p_switch);
        }
}

static void ioc_scan_input(ioc_t* p_ioc) {
        int8_t input_index;
        for (input_index = (MAX_APP_INPUT_NUM - 2); input_index >= 0;input_index--)
        {
        	Switch* p_switch = &p_ioc->actuator_buffer[input_index];
            if (input_index == FAR_LIGHT_BUF_INDEX || input_index == COS_LIGHT_BUF_INDEX)
            {
            	if (p_ioc->actuator_buffer[H_LIGHT_BUF_INDEX].state == STATE_OFF)
            	{
            		p_switch->state = STATE_OFF;
                }
            	else
            	{
                    switch_read_state(p_switch);
            	}
             }
            else
            {
            	switch_read_state(p_switch);
            }
        }
}

static void io_switch_set_state(ioc_t* p_ioc, uint8_t index) {
        Switch* p1_switch = &p_ioc->actuator_buffer[index];
        Switch* p2_switch = &p_ioc->actuator_control[index];
        if (p1_switch->state == STATE_ON)
                switch_on(p2_switch);
        else
                switch_off(p2_switch);

}


static void ioc_turn_off_all(ioc_t* p_ioc) {
        uint8_t output_index = 0;
        Switch* p_switch = &p_ioc->actuator_control[0];
        for (output_index = 0; output_index < MAX_APP_OUTPUT_NUM; output_index++) {
                p_switch = &p_ioc->actuator_control[output_index];
                switch_off(p_switch);
        }
}

static void ioc_update_output(ioc_t* p_ioc) {
        //if (is_key_on(p_ioc) == true) {
        if (1) {
                ioc_process_output(p_ioc);
                if (_is_turn_on_motor == false) {
                        ioc_turn_on_vib_motor(p_ioc);
                        _is_turn_on_motor = true;
                }
                if (on_vib_counter++ == 10) {
                        ioc_turn_off_vib_motor(p_ioc);
                }

        } else {
                ioc_turn_off_all(p_ioc);
        }
}

static void ioc_process_output(ioc_t *p_ioc) {
        uint8_t output_index = 0;
        for (output_index = 0; output_index <= SPORT_MODE_BUF_INDEX; output_index++)
        {
                Switch *p_switch = &p_ioc->actuator_buffer[output_index];
                Switch *p1_switch = &p_ioc->actuator_control[output_index];
                if (p_switch->is_remote_control == false)
                {
                	if (output_index == TURN_L_LIGHT_CTRL_INDEX || output_index == TURN_R_LIGHT_CTRL_INDEX)
                	{
                		Switch *p2_switch = &p_ioc->actuator_control[output_index + 7];
                        if (p_switch->state == STATE_ON)
                        {
                        	if (ioc_tick == 0)
                        	{
                        		ioc_tim_enable();
                            }
                        	else
                            {
                        		switch_toggle(p1_switch); /* toggle output     */
                                switch_toggle(p2_switch); /* toggle output     */
                                ioc_tick = 0; /* reset count value */
                            }
                         }
                         else
                         {
                        	 switch_off(p1_switch);
                             switch_off(p2_switch); /* toggle output           */
                         }
                	}
                	else if (output_index == BRAKE_LIGHT_CTRL_INDEX)
                	{
                		if (p_switch->state == STATE_ON)
                		{
                			switch_on(p1_switch);
                			Switch *p3_switch = &p_ioc->actuator_control[BACK_LIGHT_CTRL_INDEX];
                			switch_off(p3_switch);
                		}
                		else
                		{
                			switch_off(p1_switch);
                			io_switch_set_state(p_ioc,H_LIGHT_BUF_INDEX);
                		}
                	}
                	else
                	{
                		if (output_index == SPORT_MODE_CTRL_INDEX)
                		{
                			if (p_switch->state == STATE_ON)
                			{
                				BSP_LCD_DrawBitmap_Grayscale4(380, 75, (uint8_t *)icon_sport);
                			}
                			else BSP_LCD_DrawBitmap_Grayscale4(380, 75, (uint8_t *)icon_eco);
                		}
                		if (output_index == FAR_LIGHT_CTRL_INDEX)
                		{
                			if (p_switch->state == STATE_ON)
                			{
                				BSP_LCD_DrawBitmap_Grayscale4(55, 75, (uint8_t *)icon_far_led);
                			}
                		}
                		if (output_index == COS_LIGHT_CTRL_INDEX)
                		{
                			if (p_switch->state == STATE_ON)
                			{
                				BSP_LCD_DrawBitmap_Grayscale4(55, 75, (uint8_t *)icon_cos_led);
                			}
                		}
                		if (output_index == BACK_LIGHT_CTRL_INDEX )
                		{
                			if (p_switch->state == STATE_OFF) BSP_LCD_DrawBitmap_Monochrome(55, 75, (uint8_t *)icon_blank);
                		}
                		io_switch_set_state(p_ioc, output_index);
                	}
                }
        }
}

static void ioc_init_switch(ioc_t* p_ioc) {
        ioc_switch_in_init(p_ioc);
        ioc_switch_out_init(p_ioc);
}

static void ioc_turn_on_vib_motor(ioc_t* p_ioc) {
        Switch* p_switch = &p_ioc->actuator_control[VIB_MOTOR_CTRL_INDEX];
        switch_on(p_switch);
}

static void ioc_turn_off_vib_motor(ioc_t* p_ioc) {
        Switch* p_switch = &p_ioc->actuator_control[VIB_MOTOR_CTRL_INDEX];
        switch_off(p_switch);
}

void ioc_init(ioc_t* p_ioc) {
        ioc_init_switch(p_ioc);
}

static bool is_key_on(ioc_t* p_ioc)
{
	if (p_ioc->actuator_buffer[START_BUF_INDEX].state == STATE_ON)
	{
		ioc_tim_start_ver2();
		if (turn_on_counter++ == 10)
		{
			key_state = true;
			turn_on_counter = 0;
		}
	}
	else key_state = false;
	return key_state;
}

void ioc_update_device(ioc_t* p_ioc) {
        ioc_scan_input(p_ioc);
        ioc_update_output(p_ioc);
}




