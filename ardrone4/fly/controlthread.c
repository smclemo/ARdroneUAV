	/*
 controlthread.c - AR.Drone control thread

 Copyright (C) 2011 Hugo Perquin - http://blog.perquin.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA 02110-1301 USA.
 */

//#define _GNU_SOURCE
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>  //exit()
#include <pthread.h>
#include <math.h>

#include "../util/type.h"
#include "../util/util.h"
#include "../motorboard/mot.h"
#include "../udp/udp.h"
#include "controlthread.h"
#include "controls.h"
#include "control_strategies/pid_strategy.h"


pthread_t ctl_thread;
pthread_mutex_t control_access_mutex = PTHREAD_MUTEX_INITIALIZER;

struct drone_state_struct ds;


struct control_strategy_struct control_strategy;

int i=0;
unsigned int threadsActive = 0;

float motor[4];

#define MAX_LOGBUFSIZE 1024

struct udp_struct udpNavLog;
int logcnt = 0;
void navLog_Send();
void *ctl_thread_main(void* data);

int ctl_Init(char *client_addr) 
{
	LOAD_STRATEGY(control_strategy, pid_strategy);
	
	printf("%p %p %p \n", control_strategy.init, control_strategy.calculateMotorSpeeds, control_strategy.getLogText);
	
	int rc;
 
	//defaults from AR.Drone app:  pitch,roll max=12deg; yawspeed max=100deg/sec; height limit=on; vertical speed max=700mm/sec; 
	ds.control_limits.pitch_roll_max = DEG2RAD(12); //degrees     
	//control_limits.yawsp_max=DEG2RAD(100); //degrees/sec
	ds.control_limits.h_max = 1.80;
	ds.control_limits.h_min = 0;
	ds.control_limits.throttle_hover = 0.66;
	ds.control_limits.throttle_min = 0.50;
	ds.control_limits.throttle_max = 0.85;
	
	ds.hor_velocities.xv = 0;
	ds.hor_velocities.yv = 0;

	//Attitude Estimate
	rc = att_Init(&ds.att);
	if (rc)
		return rc;
	printf("Attitude init done\n");
	
	//udp logger
	/*if (client_addr) {
		udpClient_Init(&udpNavLog, client_addr, 7778);
		navLog_Send();
		printf("udpClient_Init %d\n", rc);
	}*/

	//start motor thread
	rc = mot_Init();
	if (rc)
		return rc;
	printf("Motor init done\n");
		
	control_strategy.init();
	printf("PID Strategy init done\n");
	
	//start ctl thread 
	rc = pthread_create(&ctl_thread, NULL, ctl_thread_main, NULL);
	if (rc) {
		printf("ctl_Init: Return code from pthread_create(mot_thread) is %d\n", rc);
		return 202;
	}
	
	pthread_mutex_lock(&control_access_mutex);
	printf("Starting Horizontal Velocity thread\n");
	rc = horizontal_velocities_init(&ds.hor_velocities);
	if (rc)
		return rc;
	printf("Horizontal Velocity init done\n");
	threadsActive = 1;
	pthread_mutex_unlock(&control_access_mutex);
	
	return 0;
}

void *ctl_thread_main(void* data) {
	int cnt=0;
	int rc;
	switchState(&ds,Landed);

	while (1) {
		rc = att_GetSample(&ds.att);
		if (!rc) {
			//horizontal_velocities_getSample(&ds.hor_velocities,&ds.att);
			break;
		}
		if (rc != 1)
			printf("ctl_thread_main: att_GetSample return code=%d", rc);
			
	}

	while (1) {

		//get sample
		//printf("C");
		while (1) {
			rc = att_GetSample(&ds.att); //non blocking call
			if (!rc) 
			{
				if(threadsActive) horizontal_velocities_getSample(&ds.hor_velocities,&ds.att);
				break;
			}
			if (rc != 1)
				printf("ctl_thread_main: att_GetSample return code=%d", rc);
		}
		
		control_strategy.calculateMotorSpeeds(&ds, motor);

		//send to motors
		mot_Run(motor[0], motor[1], motor[2], motor[3]);

		if ((cnt % 200) == 0) {
			printf("\nSET ROLL %5.2f PITCH %5.2f YAW %5.2f   H %5.2f\n",
					ds.setpoint.roll, ds.setpoint.pitch, ds.setpoint.yaw, ds.setpoint.h);
			printf("ATT ROLL %5.2f PITCH %5.2f YAW %5.2f   H %5.2f\n", ds.att.roll,
					ds.att.pitch, ds.att.yaw, ds.att.h);
		}

		//blink leds    
		cnt++;
		if ((cnt % 200) == 0)
			mot_SetLeds(MOT_LEDGREEN, MOT_LEDGREEN, MOT_LEDGREEN, MOT_LEDGREEN);
		else if ((cnt % 200) == 100)
			mot_SetLeds(0, 0, 0, 0);

		//navLog_Send();
		
		//yield to other threads
		pthread_yield();
	}
}

//logging
/*void navLog_Send() {
	char logbuf[MAX_LOGBUFSIZE];
	int logbuflen;

	float motval[4];
	mot_GetMot(motval);

	logcnt++;
	logbuflen = snprintf(logbuf,MAX_LOGBUFSIZE,
			"%d,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%ld"
			//sequence+timestamp
			, logcnt, ds.att.ts // navdata timestamp in sec
			, ds.flyState
			//sensor data
			, ds.att.ax // acceleration x-axis in [m/s^2] front facing up is positive         
			, ds.att.ay // acceleration y-axis in [m/s^2] left facing up is positive                
			, ds.att.az // acceleration z-axis in [m/s^2] top facing up is positive             
			, RAD2DEG(ds.att.gx) // gyro value x-axis in [deg/sec] right turn, i.e. roll right is positive           
			, RAD2DEG(ds.att.gy) // gyro value y-axis in [deg/sec] right turn, i.e. pitch down is positive                     
			, RAD2DEG(ds.att.gz) // gyro value z-axis in [deg/sec] right turn, i.e. yaw left is positive 
			, ds.att.hv // vertical speed [m/sec]
			//height
			, ds.setpoint.h // setpoint height
			, ds.att.h // actual height above ground in [m] 
			, (motval[0]+motval[1]+motval[2]+motval[3])/4 // throttle setting 0.00 - 1.00
			//pitch
			, RAD2DEG(ds.setpoint.pitch) //setpoint pitch [deg]
			, RAD2DEG(ds.att.pitch) //actual pitch   
			//roll
			, RAD2DEG(ds.setpoint.roll) //setpoint roll [deg]
			, RAD2DEG(ds.att.roll) //actual roll  
			//yaw
			, RAD2DEG(ds.setpoint.yaw) //yaw pitch [deg]
			, RAD2DEG(ds.att.yaw) //actual yaw  
			, motval[0]
			, motval[1]
			, motval[2]
			, motval[3]
			, ds.hor_velocities.xv
			, ds.hor_velocities.yv
			, ds.hor_velocities.dt
			, ds.hor_velocities.seqNum
			);
			
	logbuflen+=control_strategy.getLogText(logbuf+logbuflen,MAX_LOGBUFSIZE-logbuflen);
	udpClient_Send(&udpNavLog, logbuf, logbuflen);
}*/

int ctl_FlatTrim() {
	return att_FlatTrim(&ds.att);
}

void ctl_SetSetpoint(float pitch, float roll, float yaw, float h) {
	if (pitch > ds.control_limits.pitch_roll_max)
		pitch = ds.control_limits.pitch_roll_max;
	if (pitch < -ds.control_limits.pitch_roll_max)
		pitch = -ds.control_limits.pitch_roll_max;
	ds.setpoint.pitch = pitch;
	if (roll > ds.control_limits.pitch_roll_max)
		roll = ds.control_limits.pitch_roll_max;
	if (roll < -ds.control_limits.pitch_roll_max)
		roll = -ds.control_limits.pitch_roll_max;
	ds.setpoint.roll = roll;
	//if(yaw > control_limits.yawsp_max) yaw = control_limits.yawsp_max;
	//if(yaw < -control_limits.yawsp_max) yaw = -control_limits.yawsp_max;
	ds.setpoint.yaw = yaw;
	if (h > ds.control_limits.h_max)
		h = ds.control_limits.h_max;
	if (h <= 0)
		h = 0;
	if (h > 0 && h < ds.control_limits.h_min)
		h = ds.control_limits.h_min;
	ds.setpoint.h = h;
}

void ctl_SetSetpointDiff(float pitch, float roll, float yaw, float h) {
	ctl_SetSetpoint(pitch + ds.setpoint.pitch, ds.setpoint.pitch + pitch,
			yaw + ds.setpoint.yaw, h + ds.setpoint.h);
}

void ctl_Close() {
	control_strategy.Close();
	mot_Close();
	horizontal_velocities_close();
	att_Close();
}


