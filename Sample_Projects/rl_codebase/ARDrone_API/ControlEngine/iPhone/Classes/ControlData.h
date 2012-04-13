/*
 *  ControlData.h
 *  ARDroneEngine
 *
 *  Created by Frederic D'HAEYER on 14/01/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _CONTROLDATA_H_
#define _CONTROLDATA_H_
#include "ConstantsAndMacros.h"

typedef enum  {
	MESSAGEBOX_WIFI_NOT_REACHABLED,
	MESSAGEBOX_START_NOT_RECEIVED,
	MESSAGEBOX_CANT_CONNECT_TO_TOY,
	MESSAGEBOX_EMERGENCY_ULTRASOUND,
	MESSAGEBOX_EMERGENCY_CUT_OUT,
	MESSAGEBOX_EMERGENCY_MOTORS,
	MESSAGEBOX_EMERGENCY_CAMERA,
	MESSAGEBOX_EMERGENCY_PIC_WATCHDOG,
	MESSAGEBOX_EMERGENCY_PIC_VERSION,
	MESSAGEBOX_EMERGENCY_TOO_MUCH_WIND,
	MESSAGEBOX_EMERGENCY_TOO_MUCH_ANGLE,
	MESSAGEBOX_EMERGENCY_NOT_ENOUGH_POWER,
	MESSAGEBOX_EMERGENCY_BATTERY_LOW,
	MESSAGEBOX_EMERGENCY_BATTERY_HIGH,
	MESSAGEBOX_EMERGENCY_USER,
	MESSAGEBOX_EMERGENCY_UNKNOWN,
	MESSAGEBOX_EMERGENCY_PRESS_RESET,	
	MESSAGEBOX_ALERT_BATTERY_LOW,
	MESSAGEBOX_ALERT_ULTRASOUND,
	MESSAGEBOX_ALERT_VISION,
	MESSAGEBOX_ALERT_NO_VIDEO_CONNECTION,
	MESSAGEBOX_ALERT_CAMERA,
	MESSAGEBOX_MAX
} eMESSAGEBOX;

typedef enum _EMERGENCY_STATE_
{
	EMERGENCY_STATE_EMERGENCY,
	EMERGENCY_STATE_RESET
} EMERGENCY_STATE;

typedef struct 
{
	/**
	 * Current bootstrap
	 */
	bool_t bootstrap;
	
	/**
	 * Current key pressed
	 */
	int	keyPressed;
	
	/**
	 * Progressive commands
	 * And accelerometers values transmitted to drone, FALSE otherwise
	 */
	float yaw, gaz, accelero_phi, accelero_theta;
	
	/**
	 * variable to know if setting is needed
	 */
	bool_t needSetFrequency;
	EMERGENCY_STATE	isInEmergency;
	
	bool_t is_client;
	
	bool_t needSetNavdataDemo;
	bool_t needGetConfiguration;
	bool_t needSetManualTrim;
	bool_t needSetPairing;
	
	bool_t manual_trim, manual_trim_enabled;
	float trim_pitch, trim_roll, trim_yaw;
	
	bool_t wifiReachabled;
	
	int framecounter;
	bool_t navdata_demo;
	bool_t needSetEmergency;
	int needAnimation;
	int needVideoSwitch;
	int needChangeCameraDetection;
	float tag_size;
	int needLedAnimation;
	float ledAnimFreq;
	unsigned int ledAnimDuration;
	
	bool_t showInterface;
	
	/**
	 * Strings to display in interface
	 */	
	char error_msg[64];
	char takeoff_msg[16];
	char emergency_msg[16];
} ControlData;

void initControlData(void);
void resetControlData(void);
void initNavdataControlData(void);
void checkErrors(void);
void controlfps(void);
void sendControls(bool_t accelero, bool_t left, bool_t right, bool_t up, bool_t down);
void switchTakeOff(void);
void emergency(void);
void switchVideoChannel(int video_channel);
void changeCameraDetection(int camera_type, float tag_size);
void changeLedAnimation(int led_anim, float freq, unsigned int duration);
void handleAccelerometers(void);
void disableAccelerometers(void);
void inputYaw(float percent);
void inputGaz(float percent);
void signal_input(int new_input);
void send_inputs(void);

void setNavdataDemo(bool_t navdata_demo);
#endif // _CONTROLDATA_H_
