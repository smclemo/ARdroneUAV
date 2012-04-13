//
//  control_ack.m
//  iPhoneDemo
//
//  Created by Mykonos on 20/10/08.
//  Copyright 2008 Parrot SA. All rights reserved.
//
#include "control_cfg.h"
#include <config.h>
#include <ardrone_common_config.h>

#undef ARDRONE_CONFIG_KEY_IMM
#undef ARDRONE_CONFIG_KEY_REF
#undef ARDRONE_CONFIG_KEY_STR
#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)
#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)
#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)
#include <config_keys.h> // must be included before to have types

#undef ARDRONE_CONFIG_KEY_IMM
#undef ARDRONE_CONFIG_KEY_REF
#undef ARDRONE_CONFIG_KEY_STR
#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) DEFAULT,
#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) DEFAULT,
#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) DEFAULT,
const ardrone_config_t ardrone_control_config_default =
{
#include <config_keys.h>
};

ardrone_config_t ardrone_control_config = 
{
#include <config_keys.h>
};

static ardrone_control_configuration_event_t control_cfg_event = { 0 };
static dictionary *control_cfg_dict = NULL;
static bool_t control_cfg_is_init = FALSE;

void control_cfg_init(void)
{
	if(!control_cfg_is_init)
	{
		control_cfg_dict = dictionary_new(0);

#undef ARDRONE_CONFIG_KEY_IMM
#undef ARDRONE_CONFIG_KEY_REF
#undef ARDRONE_CONFIG_KEY_STR
#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) \
iniparser_alias(control_cfg_dict, KEY ":" #NAME, INI_TYPE, &ardrone_control_config.NAME, NULL,RW);
#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) \
iniparser_alias(control_cfg_dict, KEY ":" #NAME, INI_TYPE, &ardrone_control_config.NAME, NULL,RW);
#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) \
iniparser_alias(control_cfg_dict, KEY ":" #NAME, INI_TYPE, &ardrone_control_config.NAME[0], NULL,RW);
#include <config_keys.h>

		control_cfg_event.status = ARDRONE_CONTROL_EVENT_IDLE;
		control_cfg_is_init = TRUE;
	}
}

static void control_cfg_event_configure_start(struct _ardrone_control_event_t* event)
{
	//printf("%s\n", __FUNCTION__);
}

static void control_cfg_event_configure_end(struct _ardrone_control_event_t* event)
{
	//printf("%s\n", __FUNCTION__);
	switch(event->status)
	{
		case ARDRONE_CONTROL_EVENT_FINISH_SUCCESS:
		case ARDRONE_CONTROL_EVENT_FINISH_FAILURE:
			break;
			
		default:
			// Nothing to do
			break;
	}
}

static void control_cfg_event_configure(void)
{
	control_cfg_event.event			= CFG_GET_CONTROL_MODE;
	control_cfg_event.num_retries	= 20;
	control_cfg_event.status		= ARDRONE_CONTROL_EVENT_WAITING;
	
	control_cfg_event.ardrone_control_event_start = control_cfg_event_configure_start;
	control_cfg_event.ardrone_control_event_end	  = control_cfg_event_configure_end;
	
	control_cfg_event.config_state = CONFIG_REQUEST_INI;
	control_cfg_event.ini_dict = control_cfg_dict;
	
	ardrone_control_send_event( (ardrone_control_event_t*)&control_cfg_event );
}

bool_t control_get_configuration(void)
{
	bool_t b = FALSE;
	if(control_cfg_is_init)
	{
		switch(control_cfg_event.status)
		{
			case ARDRONE_CONTROL_EVENT_IDLE:
				control_cfg_event_configure();
				break;
				
			case ARDRONE_CONTROL_EVENT_FINISH_FAILURE:
				control_cfg_event.status = ARDRONE_CONTROL_EVENT_IDLE;
				break;
				
			case ARDRONE_CONTROL_EVENT_FINISH_SUCCESS:
				printf("[Get configuration] done\n");
				control_cfg_event.status = ARDRONE_CONTROL_EVENT_IDLE;
				b = TRUE;
				break;
		}
	}
	
	return b;
}
