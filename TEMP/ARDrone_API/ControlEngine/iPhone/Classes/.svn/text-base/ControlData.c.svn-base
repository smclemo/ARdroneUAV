/*
 *  ControlData.m
 *  ARDroneEngine
 *
 *  Created by Frederic D'HAEYER on 14/01/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include "ConstantsAndMacros.h"
#include "ControlData.h"

//#define DEBUG_CONTROL

ControlData ctrldata = { 0 };
instance_navdata_t ctrlnavdata;
extern instance_navdata_t inst_nav; 
extern char iphone_mac_address[];

static const char* messages[MESSAGEBOX_MAX] =
{
	[MESSAGEBOX_WIFI_NOT_REACHABLED] = "WIFI NOT REACHABLE",
	[MESSAGEBOX_START_NOT_RECEIVED] = "START NOT RECEIVED",
	[MESSAGEBOX_CANT_CONNECT_TO_TOY] = "CAN'T CONNECT TO TOY",
	[MESSAGEBOX_EMERGENCY_ULTRASOUND] = "EMERGENCY ULTRASOUND",
	[MESSAGEBOX_EMERGENCY_CUT_OUT] = "EMERGENCY CUT OUT",
	[MESSAGEBOX_EMERGENCY_MOTORS] = "EMERGENCY MOTORS",
	[MESSAGEBOX_EMERGENCY_CAMERA] = "EMERGENCY CAMERA",
	[MESSAGEBOX_EMERGENCY_PIC_WATCHDOG] = "EMERGENCY PIC WATCHDOG",
	[MESSAGEBOX_EMERGENCY_PIC_VERSION] = "EMERGENCY PIC VERSION",
	[MESSAGEBOX_EMERGENCY_TOO_MUCH_ANGLE] = "EMERGENCY TOO MUCH ANGLE",	
	[MESSAGEBOX_EMERGENCY_BATTERY_LOW] = "EMERGENCY BATTERY LOW",
	[MESSAGEBOX_EMERGENCY_USER] = "EMERGENCY BY USER",
	[MESSAGEBOX_EMERGENCY_UNKNOWN] = "EMERGENCY UNKNOWN",
	[MESSAGEBOX_EMERGENCY_PRESS_RESET] = "EMERGENCY PRESS RESET",		
	[MESSAGEBOX_ALERT_BATTERY_LOW] = "ALERT BATTERY LOW",
	[MESSAGEBOX_ALERT_ULTRASOUND] = "ALERT ULTRASOUND",
	[MESSAGEBOX_ALERT_VISION] = "ALERT VISION",
	[MESSAGEBOX_ALERT_NO_VIDEO_CONNECTION] = "ALERT NO VIDEO CONNECTION",
	[MESSAGEBOX_ALERT_CAMERA] = "ALERT CAMERA",
};

void initControlData(void)
{
	ctrldata.framecounter = 0;
	ctrldata.needSetFrequency = TRUE;
	setNavdataDemo(TRUE);
	ctrldata.needGetConfiguration = TRUE;
	ctrldata.needSetPairing = TRUE;
	ctrldata.needSetEmergency = FALSE;
	ctrldata.isInEmergency = FALSE;
	ctrldata.is_client = FALSE;

	ctrldata.needSetManualTrim = FALSE;
	ctrldata.manual_trim = FALSE;
	ctrldata.manual_trim_enabled = FALSE;
	
	ctrldata.trim_pitch = 0.0;
	ctrldata.trim_roll = 0.0;
	ctrldata.trim_yaw = 0.0;
	
	ctrldata.needAnimation = -1;
	ctrldata.needVideoSwitch = -1;
	ctrldata.needChangeCameraDetection = -1;
	ctrldata.tag_size = ARDRONE_TAG_SIZE_DEFAULT;
	
	ctrldata.needLedAnimation = -1;
	ctrldata.ledAnimFreq = 0.0f;
	ctrldata.ledAnimDuration = 0;
	
	ctrldata.wifiReachabled = FALSE;
	ctrldata.showInterface = TRUE;
	
	strcpy(ctrldata.error_msg, "");
	strcpy(ctrldata.takeoff_msg, "take_off");
	strcpy(ctrldata.emergency_msg, "emergency");
	
	initNavdataControlData();
	resetControlData();
	ardrone_tool_start_reset();
}

void initNavdataControlData(void)
{
	//drone data
	ardrone_navdata_reset_data(&ctrlnavdata);
}

void resetControlData(void)
{
	//printf("reset control data\n");
	ctrldata.keyPressed = KEY_NONE;
	ctrldata.accelero_phi = 0.0;
	ctrldata.accelero_theta = 0.0;
	ctrldata.yaw = 0;
	ctrldata.gaz = 0;
	ctrldata.bootstrap = TRUE;
	initNavdataControlData();
}

void setNavdataDemo(bool_t navdata_demo)
{
	ctrldata.navdata_demo = navdata_demo;
	ctrldata.needSetNavdataDemo = TRUE;
}

void switchTakeOff(void)
{
	if(!ctrlnavdata.startPressed)
	{
#ifdef DEBUG_CONTROL
		PRINT("START BUTTON\n");
#endif
		ardrone_tool_set_ui_pad_start( 1 );
	}
	else
	{
#ifdef DEBUG_CONTROL
		PRINT("STOP BUTTON\n");
#endif
		ardrone_tool_set_ui_pad_start( 0 );
	}
}

void emergency(void)
{
#ifdef DEBUG_CONTROL
	PRINT("%s\n", __FUNCTION__);
#endif
	ctrldata.needSetEmergency = TRUE;
}

void switchVideoChannel(int videoChannel)
{
#ifdef DEBUG_CONTROL
	PRINT("%s\n", __FUNCTION__);
#endif
	ctrldata.needVideoSwitch = videoChannel;
}

void changeCameraDetection(int camera_type, float tag_size)
{
#ifdef DEBUG_CONTROL
	PRINT("%s\n", __FUNCTION__);
#endif
	ctrldata.needChangeCameraDetection = camera_type;
	ctrldata.tag_size = tag_size;
}

void changeLedAnimation(int led_anim, float freq, unsigned int duration)
{
	ctrldata.needLedAnimation = led_anim;
	ctrldata.ledAnimFreq = freq;
	ctrldata.ledAnimDuration = duration;
}

void inputYaw(float percent)
{
#ifdef DEBUG_CONTROL
	PRINT("%s : %d\n", __FUNCTION__, percent);
#endif
	if(-1.0 <= percent && percent <= 1.0)
		ctrldata.yaw = percent;
	else
		ctrldata.yaw = 0.0;
}

void inputGaz(float percent)
{
#ifdef DEBUG_CONTROL
	PRINT("%s : %d\n", __FUNCTION__, percent);
#endif
	if(-1.0 <= percent && percent <= 1.0)
		ctrldata.gaz = percent;
	else
		ctrldata.gaz = 0.0;
}

void sendControls(bool_t accelero, bool_t left, bool_t right, bool_t up, bool_t down)
{
	ardrone_at_set_progress_cmd(accelero, ctrldata.accelero_phi, ctrldata.accelero_theta, ctrldata.gaz, ctrldata.yaw);
}

void checkErrors(void)
{
	input_state_t* input_state = ardrone_tool_get_input_state();
	
	strcpy(ctrldata.error_msg, "");
	
	if(!ctrldata.wifiReachabled)
	{
		strcat(ctrldata.error_msg, messages[MESSAGEBOX_WIFI_NOT_REACHABLED]);
		strcat(ctrldata.error_msg, "\n");
		resetControlData();
	}
	else
	{
		if(ctrldata.needAnimation >= 0)
		{
			ardrone_at_set_anim(ctrldata.needAnimation, MAYDAY_TIMEOUT[ctrldata.needAnimation]);
			ctrldata.needAnimation = -1;
		}
		
		if(ctrldata.needVideoSwitch >= 0)
		{
			ardrone_at_zap(ctrldata.needVideoSwitch);
			ctrldata.needVideoSwitch = -1;
		}
		
		if(ctrldata.needChangeCameraDetection >= 0)
		{
			ardrone_at_cad(ctrldata.needChangeCameraDetection, ctrldata.tag_size);
			ctrldata.needChangeCameraDetection = -1;
		}	
		
		if(ctrldata.needLedAnimation >= 0)
		{
			ardrone_at_set_led_animation(ctrldata.needLedAnimation, ctrldata.ledAnimFreq, ctrldata.ledAnimDuration);
			ctrldata.needLedAnimation = -1;
		}	
		
		if(ctrldata.needSetEmergency)
		{
			ctrldata.isInEmergency = ctrlnavdata.emergencyLanding;
			ardrone_tool_set_ui_pad_select(1);
			ctrldata.needSetEmergency = FALSE;
		}
		
		if( ctrldata.needSetNavdataDemo)
		{
			if( control_ack_configure_navdata_demo(ctrldata.navdata_demo))
				ctrldata.needSetNavdataDemo = FALSE;
		}
		else if( ctrldata.needSetPairing )
		{
			if(control_ack_configure_mac_address((const char*)&iphone_mac_address[0]))
				ctrldata.needSetPairing = FALSE;
		}
		else if(ctrldata.needSetFrequency)
		{
			if(control_ack_configure_ultrasound_frequency(!ctrldata.is_client))
				ctrldata.needSetFrequency = FALSE;
		}
		else if(ctrldata.needSetManualTrim)
		{
			if(control_ack_configure_manual_trim(ctrldata.manual_trim))
			{
				ctrldata.needSetManualTrim = FALSE;
				ctrldata.manual_trim_enabled = ctrldata.manual_trim;
			}
		}
		else if(ctrldata.needGetConfiguration)
		{
			//PRINT("Request configuration file\n");
			if(control_get_configuration())
				ctrldata.needGetConfiguration = FALSE;
		}
		
		if((ctrldata.framecounter % (kFPS / 2)) == 0)
		{
			if(ctrlnavdata.bootstrap)
			{
				setNavdataDemo(TRUE);
				ctrldata.needSetFrequency = TRUE;
				ctrldata.needGetConfiguration = TRUE;
				ctrldata.needSetPairing = TRUE;
			}
		}
		
		if(ardrone_navdata_client_get_num_retries() >= NAVDATA_MAX_RETRIES)
		{
			strcat(ctrldata.error_msg, messages[MESSAGEBOX_CANT_CONNECT_TO_TOY]);
			strcat(ctrldata.error_msg, "\n");
			resetControlData();
		}
		else if(ctrlnavdata.emergencyLanding)
		{
			if(!ctrldata.isInEmergency && input_state->select)
				ardrone_tool_set_ui_pad_select(0);
			
			strcpy(ctrldata.emergency_msg, "reset");
			strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_PRESS_RESET]);
			strcat(ctrldata.error_msg, "\n");
			
			if(ctrlnavdata.ultrasoundProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_ULTRASOUND]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.cutoutProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_CUT_OUT]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.motorsProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_MOTORS]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.cameraProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_CAMERA]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.adcProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_PIC_WATCHDOG]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.adcVersionProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_PIC_VERSION]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.anglesProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_TOO_MUCH_ANGLE]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.vbatLowProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_BATTERY_LOW]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.userEmergency)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_USER]);
				strcat(ctrldata.error_msg, "\n");
			}
			else
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_EMERGENCY_UNKNOWN]);
				strcat(ctrldata.error_msg, "\n");
			}
						
			resetControlData();
			ardrone_tool_start_reset();

			if(ctrlnavdata.startPressed)
				switchTakeOff();
		}
		else if(!ctrlnavdata.emergencyLanding)
		{	
			if(ctrldata.isInEmergency && input_state->select)
				ardrone_tool_set_ui_pad_select(0);
			
			if(video_stage_get_num_retries() > VIDEO_MAX_RETRIES)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_ALERT_NO_VIDEO_CONNECTION]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.vbatLowProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_ALERT_BATTERY_LOW]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.ultrasoundProblem)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_ALERT_ULTRASOUND]);
				strcat(ctrldata.error_msg, "\n");
			}
			else if(ctrlnavdata.visionProblem && ctrlnavdata.flyingState)
			{
				strcat(ctrldata.error_msg, messages[MESSAGEBOX_ALERT_VISION]);
				strcat(ctrldata.error_msg, "\n");
			}

			if(!ctrlnavdata.timerElapsed)
				strcpy(ctrldata.emergency_msg, "emergency");
			
			if(input_state->start)
			{
				if(ctrlnavdata.startPressed)
				{
					strcpy(ctrldata.takeoff_msg, "take_land");
				}
				else
				{	
					strcpy(ctrldata.takeoff_msg, "take_off");
					strcat(ctrldata.error_msg, messages[MESSAGEBOX_START_NOT_RECEIVED]);
					strcat(ctrldata.error_msg, "\n");
				}
			}
			else
			{
				strcpy(ctrldata.takeoff_msg, "take_off");
			}			
		}
		
	}
}

