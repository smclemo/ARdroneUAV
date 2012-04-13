//
// ConstantsAndMacros.h
//  Constants and macros for pengl view.
//
//  Created by Frédéric D'HAEYER on 09/10/30.
//  Copyright 2009 Parrot SA. All rights reserved.
//
// Macros
#ifndef _CONSTANTS_AND_MACROS_H_
#define _CONSTANTS_AND_MACROS_H_
#include <ardrone_api.h>
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/ardrone_time.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/Control/ardrone_control_ack.h>
#include <ardrone_Tool/Control/ardrone_control_configuration.h>
#include <ardrone_tool/Control/ardrone_control_soft_update.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_tool/Com/config_com.h>
#include <ardrone_tool/Video/video_com_stage.h>

#include <VP_Os/vp_os.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_delay.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>
#include <VP_Api/vp_api_thread_helper.h>

#include <VLIB/Stages/vlib_stage_decode.h>

#include <iniparser3.0b/src/iniparser.h>

#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <TargetConditionals.h>
#include "ControlData.h" 

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "mobile_main.h"
#include "wifi.h"
#include "navdata.h"
#include "ControlData.h"
#include "video_stage.h"
#include "opengl_stage.h"
#include "control_ack.h"
#include "control_cfg.h"

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <MediaPlayer/MediaPlayer.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#endif

#if TARGET_CPU_X86 == 1 // We are on iPhone simulator
#define WIFI_ITFNAME "en1"
#endif // TARGET_CPU_X86

#if TARGET_CPU_ARM == 1 // We are on real iPhone
#define WIFI_ITFNAME "en0"
#endif // TARGET_CPU_ARM

#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) / 180.0 * M_PI)
#define M_PI_8				(M_PI_4 / 2)

// How many times a second to refresh the screen
#define kFPS 20		// Frame per second
#define kAPS 40		// Number of accelerometer() function calls by second 

// Software Version
#define SOFTWARE_VERSION	@"1.0.3"

// For setting up perspective, define near, far, and angle of view
#define kZNear				0.01
#define kZFar				1000.0
#define kFieldOfView		45.0
#define kSize				(kZNear * tanf(DEGREES_TO_RADIANS(kFieldOfView) / 2.0))
#define kGroupIndexVertex	0

#define USE_3D_MOTION		1

// Control view definitions
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define JOYSTICK_BORDER_X	35
#define JOYSTICK_BORDER_Y	20

/**
 * @def SETTINGS_FILE
 * The name of the file about flight settings. The file is in app's documents directory.
 */
#define SETTINGS_FILE "/ARDroneSettings"

//drones input
#define ARDRONE_UI_NO_INPUT 0x11540000

// Drone Misc value
//internal key codes
#define KEY_NONE 0
#define KEY_UP 1
#define KEY_LEFT 1<<1
#define KEY_DOWN 1<<2
#define KEY_RIGHT 1<<3
//#define KEY_FIRE 1<<4
#define KEY_RELOAD 1<<5
#define KEY_ACCELERO_UP 1<<6
#define KEY_ACCELERO_LEFT 1<<7
#define KEY_ACCELERO_DOWN 1<<8
#define KEY_ACCELERO_RIGHT 1<<9

// ARDrone interface value
#define KEY_MAIN_MENU		1<<1
#define KEY_SETTINGS_MENU	1<<2
#define KEY_FIRE			1<<3

#define MAKE_BUTTON_RECT(name)	CGRectMake(name##_POS_X - (name##_WIDTH / 2.0), name##_POS_Y - (name##_HEIGHT / 2.0), name##_WIDTH, name##_HEIGHT)
#define CHECK_OPENGL_ERROR() ({ GLenum __error = glGetError(); if(__error) NSLog(@"OpenGLES error 0x%04X in %s\n", __error, __FUNCTION__); (__error ? NO : YES); })

//#define SPEED_MAX		10 // m/s

#define ARDRONE_TAG_SIZE_DEFAULT 14.0

//#define TEST_IPAD
//#define USE_MULTIDETECTION
//#define DISABLE_VIDEO
//#define WRITE_DEBUG_ACCELERO
#define INTERFACE_WITH_DEBUG

typedef struct {
	float x;
	float y;
	float z;
} Vertex3D, Coord3D, Rotation3D, Scale3D;

typedef struct {
	float v1;
	float v2;
	float v3;
} Face3D;

typedef struct {
	float r;
	float g;
	float b;
	float a;	
} ColorRGBA;

#endif // _CONSTANTS_AND_MACROS_H_