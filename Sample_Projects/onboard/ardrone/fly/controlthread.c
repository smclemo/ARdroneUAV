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
#include "../attitude/attitude.h"
#include "../udp/udp.h"
#include "pid.h"
#include "controlthread.h"
#include "../video/video.h"


      float adj_roll;
      float adj_pitch;
      float adj_yaw;
      float adj_h;
      float adj_x;
      float adj_y;

pthread_t ctl_thread;

pid_struct pid_roll;
pid_struct pid_pitch;
pid_struct pid_yaw;
pid_struct pid_h;
pid_struct pid_x;
pid_struct pid_y;

float throttle;

att_struct att;

struct setpoint_struct {
  float pitch;     //radians  
  float roll;      //radians     
  float yaw;     //yaw in radians   
  float h;         //cm
  float pitch_roll_max; //radians     
  float h_max; //cm
  float h_min; //cm
  float throttle_hover; //hover throttle setting
  float throttle_min; //min throttle (while flying)
  float throttle_max; //max throttle (while flying)
} setpoint;

udp_struct udpNavLog;
int logcnt=0;
void navLog_Send();
void *ctl_thread_main(void* data);

	vid_struct vid;

	img_struct* img_old;
	img_struct* img_new;
	
	int dx,dy;
	int x=0,y=0;

void video_cg(img_struct* img) 
{
	int h=img->h;
	int w=img->w;
	int n=w*h;
	
	int p=0;
	int px=0;
	int py=0;
	int x=0;
	int y=0;
	unsigned char* buf = img->buf;
	for(int i=0;i<n;i++) {
		p+=buf[i];
		px+=x*buf[i];
		py+=y*buf[i];
		x++;
		if(x==w) {
			x=0;
			y++;
		}
	}
	float cg_x = (float)px/p;
	float cg_y = (float)py/p;
	printf("x=%10.6f y=%10.6f\n",cg_x,cg_y);
}

void video_blocksum(img_struct* img1, img_struct* img2, int* dx_out, int* dy_out) 
{
	int h=img1->h;
	int w=img1->w;
	int n=w*h;
	unsigned char* buf1 = img1->buf;
	unsigned char* buf2 = img2->buf;
	
	int dmax = 3;
	int min_sum = 2000000000;
	int min_dx = -99;
	int min_dy = -99;
	for(int dy=-dmax;dy<=dmax;dy++) {
		for(int dx=-dmax;dx<=dmax;dx++) {
			int sum=0;
			for(int y=dmax;y<h-dmax;y++) {
				int i1 = y*w + dmax;
				int i2 = (y+dy)*w + dmax+dx;
				for(int x=dmax;x<w-dmax;x++) {
					//printf("x=%d y=%d i1=%d i2=%d\n",x,y,i1,i2);
					sum += abs(buf1[i1] - buf2[i2]);
					i1++;
					i2++;
				}
			}
			if(min_sum>sum) {
				min_sum = sum;
				min_dx = dx;
				min_dy = dy;
			}
		}
	}
			
	*dx_out=min_dx;
	*dy_out=min_dy;
}

int ctl_Init(char *client_addr) 
{
	int rc;
  
	//defaults from AR.Drone app:  pitch,roll max=12deg; yawspeed max=100deg/sec; height limit=on; vertical speed max=700mm/sec; 
	setpoint.pitch_roll_max=DEG2RAD(12); //degrees     
  //setpoint.yawsp_max=DEG2RAD(100); //degrees/sec
  setpoint.h_max=600; //cm
  setpoint.h_min=40; //cm
  setpoint.throttle_hover=0.46;
  setpoint.throttle_min=0.30;
  setpoint.throttle_max=0.85;
  			
	//init pid pitch/roll 
	pid_Init(&pid_roll,  0.4,0,-0.05,0);//0.5
	pid_Init(&pid_pitch, 0.4,0,-0.05,0);//0.5
	pid_Init(&pid_yaw,   0.8,0,-0.06,0);//1.00
	//pid_Init(&pid_yaw,   0,0,0,0);//1.00
	pid_Init(&pid_h,     0.003,0,-0.001,1);//0.0005
//	pid_Init(&pid_x,     0.01,0,0,0);//0.0005
//	pid_Init(&pid_y,     0.01,0,0,0);//0.0005

  throttle=0.00;

  //Attitude Estimate
	rc = att_Init(&att);
	if(rc) return rc;

  
  //udp logger
  udpClient_Init(&udpNavLog, client_addr, 7778);
  //navLog_Send();
  printf("udpClient_Init\n", rc);
  
	//start motor thread
	rc = mot_Init();
	if(rc) return rc;
  
	vid.device = (char*)"/dev/video1";
	vid.w=176;
	vid.h=144;
	vid.n_buffers = 4;
	video_Init(&vid);

	img_old = video_CreateImage(&vid);
	img_new = video_CreateImage(&vid);
	
	video_GrabImage(&vid, img_old);

	//start ctl thread 
	rc = pthread_create(&ctl_thread, NULL, ctl_thread_main, NULL); 
	if(rc) {
		printf("ctl_Init: Return code from pthread_create(mot_thread) is %d\n", rc);
		return 202;
	}
}

void *ctl_thread_main(void* data)
{
	int cnt;
	int rc;


 	while(1) {
		rc = att_GetSample(&att);
		if(!rc) break;
		if(rc!=1) printf("ctl_thread_main: att_GetSample return code=%d",rc); 
	}	
    
	while(1) {
		//get sample
		while(1) {
			rc = att_GetSample(&att); //non blocking call
			if(!rc) break; //got a sample
			if(rc!=1) printf("ctl_thread_main: att_GetSample return code=%d",rc); 
		}

//	video_GrabImage(&vid, img_new);

		//process
//	video_blocksum(img_old, img_new, &dx, &dy);
//	x+=dx;
//	y+=dy;

    float motor[4];    
    if(setpoint.h==0.00) {
      //motors off
      adj_roll = 0;
      adj_pitch = 0;
      adj_h = 0;

	  adj_x = 0;
      adj_y = 0;

	  adj_yaw = 0;
      throttle = 0;
    }else{     
      //flying, calc pid controller corrections
      adj_roll  = pid_CalcD(&pid_roll,  setpoint.roll   - att.roll,  att.dt, att.gx); //err positive = need to roll right
      adj_pitch = pid_CalcD(&pid_pitch, setpoint.pitch  - att.pitch, att.dt, att.gy); //err positive = need to pitch down
      adj_yaw   = pid_CalcD(&pid_yaw,   setpoint.yaw    - att.yaw,   att.dt, att.gz); //err positive = need to increase yaw to the left
      adj_h     = pid_CalcD(&pid_h,     setpoint.h      - att.h,     att.dt, att.hv); //err positive = need to increase height
      
//      adj_x     = pid_Calc(&pid_x,     0 - x,     att.dt);
//      adj_y     = pid_Calc(&pid_y,     0 - y,     att.dt);

	  throttle = setpoint.throttle_hover + adj_h;
      if(throttle < setpoint.throttle_min) throttle = setpoint.throttle_min;
      if(throttle > setpoint.throttle_max) throttle = setpoint.throttle_max;      
    }

//	printf("%f %f -- %f %f\n",x,y, adj_x, adj_y);

//	adj_pitch += adj_y;
//	adj_roll += adj_x;

    //convert pid adjustments to motor values
    motor[0] = throttle +adj_roll -adj_pitch +adj_yaw;
    motor[1] = throttle -adj_roll -adj_pitch -adj_yaw;
    motor[2] = throttle -adj_roll +adj_pitch +adj_yaw;
    motor[3] = throttle +adj_roll +adj_pitch -adj_yaw;

    //send to motors
    mot_Run(motor[0],motor[1],motor[2],motor[3]);
        
    //blink leds    
    cnt++;
    if((cnt%200)==0) 
      mot_SetLeds(MOT_LEDGREEN,MOT_LEDGREEN,MOT_LEDGREEN,MOT_LEDGREEN);
    else if((cnt%200)==100) 
      mot_SetLeds(0,0,0,0);
        
    //send UDP nav log packet    
    //navLog_Send();
  
		//yield to other threads
		pthread_yield();
	}
}


//logging
void navLog_Send()
{
  char logbuf[1024];
  int logbuflen;

  float motval[4];
	mot_GetMot(motval);
    
  logcnt++;
  logbuflen=sprintf(logbuf,"%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"
    //sequence+timestamp
    ,logcnt
    ,att.ts   // navdata timestamp in sec
    //sensor data
    ,att.ax   // acceleration x-axis in [G] front facing up is positive         
    ,att.ay   // acceleration y-axis in [G] left facing up is positive                
    ,att.az   // acceleration z-axis in [G] top facing up is positive             
    ,RAD2DEG(att.gx)   // gyro value x-axis in [deg/sec] right turn, i.e. roll right is positive           
    ,RAD2DEG(att.gy)   // gyro value y-axis in [deg/sec] right turn, i.e. pirch down is positive                     
    ,RAD2DEG(att.gz)   // gyro value z-axis in [deg/sec] right turn, i.e. yaw left is positive 
    ,att.hv   // vertical speed [cm/sec]
    //height
    ,setpoint.h  // setpoint height
    ,att.h       // actual height above ground in [cm] 
    ,throttle    // throttle setting 0.00 - 1.00
    //pitch
    ,RAD2DEG(setpoint.pitch)  //setpoint pitch [deg]
    ,RAD2DEG(att.pitch)       //actual pitch   
    ,adj_pitch                //pitch motor adjustment 
    //roll
    ,RAD2DEG(setpoint.roll)   //setpoint roll [deg]
    ,RAD2DEG(att.roll)        //actual roll  
    ,adj_roll                 //roll motor adjustment 
    //yaw
    ,RAD2DEG(setpoint.yaw)    //yaw pitch [deg]
    ,RAD2DEG(att.yaw)         //actual yaw  
    ,adj_yaw                  //yaw motor adjustment
  );    

  printf(logbuf);
//  udpClient_Send(&udpNavLog,logbuf,logbuflen); 
}

int ctl_FlatTrim()
{
  return att_FlatTrim(&att);
}

void ctl_SetSetpoint(float pitch, float roll, float yaw, float h)
{
  if(pitch > setpoint.pitch_roll_max) pitch = setpoint.pitch_roll_max;
  if(pitch < -setpoint.pitch_roll_max) pitch = -setpoint.pitch_roll_max;
  setpoint.pitch=pitch;
  if(roll > setpoint.pitch_roll_max) roll = setpoint.pitch_roll_max;
  if(roll < -setpoint.pitch_roll_max) roll = -setpoint.pitch_roll_max;
  setpoint.roll=roll;
  //if(yaw > setpoint.yawsp_max) yaw = setpoint.yawsp_max;
  //if(yaw < -setpoint.yawsp_max) yaw = -setpoint.yawsp_max;
  setpoint.yaw=yaw;
  if(h > setpoint.h_max) h = setpoint.h_max;
  if(h <= 0) h = 0;
  if(h>0 && h < setpoint.h_min) h = setpoint.h_min;
  if(setpoint.h==0 && h>0) throttle=0.4; //takeoff
  setpoint.h=h;
}

void ctl_SetSetpointDiff(float pitch, float roll, float yaw, float h)
{
  ctl_SetSetpoint(pitch+setpoint.pitch, setpoint.pitch+pitch, yaw+setpoint.yaw, h+setpoint.h);
}

void ctl_Close()
{
  mot_Close();
  att_Close();
}

void ctl_SetGas(float gas1)
{
//	throttle+=gas1;
	setpoint.throttle_hover += gas1;
}

void ctl_SetThrottleOff()
{
	setpoint.h = 0;
}

void setPidPitchRoll(float Kp, float Ki, float Kd){
	//init pid pitch/roll 
	pid_Init(&pid_roll,  Kp,Ki,Kd,10);//0.5
	pid_Init(&pid_pitch, Kp,Ki,Kd,10);//0.5
}

void setPidYaw(float Kp, float Ki, float Kd){
	pid_Init(&pid_yaw,  Kp,Ki,Kd,0);
}

void setPidHight(float Kp, float Ki, float Kd){
	pid_Init(&pid_h, Kp, Ki, Kd, 10);
}

