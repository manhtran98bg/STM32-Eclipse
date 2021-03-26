
#ifndef IOC_H_
#define IOC_H_
#include "switch.h"

typedef enum DRIVING_MODE{
	ECO_MODE,
	NORMAL_MODE,
	SUPPER_MODE,
}DRIVING_MODE;

typedef enum TURNING_SIDE_CONST {
	TURN_LEFT	=0x00,
	TURN_RIGHT	=0x01
}TURNING_SIDE;

typedef enum HEAD_LIGHT_MODE_CONST{
	LIGHT_PHA_MODE	=0x00,
	LIGHT_COS_MODE	=0x01
}HEAD_LIGHT_MODE;

typedef enum LIGHT_STATE{
    LIGHT_ON    =0x00,
    LIGHT_OFF   =0x01
}LIGHT_STATE;


typedef struct ioc_t{
	Switch actuator_control[MAX_APP_OUTPUT_NUM];
	Switch actuator_buffer[MAX_APP_INPUT_NUM];
    uint16_t status;
}ioc_t;

void ioc_init(ioc_t* p_ioc);
void ioc_update_device(ioc_t* p_ioc);
void ioc_update_dashboard(ioc_t* p_ioc);
void ioc_process_config(ioc_t* p_ioc, const uint16_t node_id,const uint16_t index, const uint8_t sub_index, const uint32_t data);

#endif /* IOC_H_ */
