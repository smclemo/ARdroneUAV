/*
    main_fly.c - AR.Drone fly demo program

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
#include <stdlib.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
//#include <fcntl.h>   /* File control definitions */
//#include <errno.h>   /* Error number definitions */
//#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>  //exit()
#include <pthread.h>
#include <ctype.h>    /* For tolower() function */
#include <math.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../util/type.h"
#include "../util/util.h"
#include "../motorboard/mot.h"
#include "controlthread.h"
#include "../video/v4l2.h"

float Kp = 0.3;
float Ki = 0.0;
float Kd = -0.05;

float gain = 0.01;

void setPid() {
	setPidPitchRoll(Kp,Ki,Kd);
	//setPidYaw(Kp,Ki,Kd);
	//setPidheight(Kp,Ki,Kd);
}

int main()
{  
  //char c = 'q';
  setPid();
	
  float roll = 0;
  float pitch = 0;
  float yaw = 0;
  float height = 10;

  //init controller
  ctl_Init();
  printf("ctl_Init completed\n");

  videoInit();
  printf("videoInit completed\n");
  
  //main loop	
  while(1) { 
	
	struct point offset = processFrame();
	yaw += (float) offset.x/100.0;
	
	/*
	if(c=='s') {
	  roll = 0;
      pitch = 0;
      yaw = 0;
	  height = 10;
	}
	if(c=='q') {
		printf("QUITTING !!!\n");
	    ctl_SetThrottleOff();
		sleep(1);
		break;
	}
	if(c=='z') {
		navLog_Send();
	}
	if(c=='u') {
		Kp = Kp+gain;
		printf("P-up: %f\n",Kp);
		setPid();
	}
	if(c=='j') {
		Kp = Kp-gain;
		printf("P-down: %f\n",Kp);
		setPid();
	}
	if(c=='i') {
		Ki = Ki+gain;
		printf("I-up: %f\n",Ki);
		setPid();
	}
	if(c=='k') {
		Ki = Ki-gain;
		printf("I-down: %f\n",Ki);
		setPid();
	}
	if(c=='o') {
		Kd = Kd+gain;
		printf("D-up: %f\n",Kd);
		setPid();
	}
	if(c=='l') {
		Kd = Kd-gain;
		printf("D-down: %f\n",Kd);
		setPid();
	}

	if(c=='+') {
		ctl_SetGas(0.01);
		printf("gas ++\n");
	}
	if(c=='-') {
		ctl_SetGas(-0.01);
		printf("gas --\n");
	}

	if(c=='w') { // up
		pitch+=0.05;
		printf("pitch: %f\n", pitch);
	}
	if(c=='x') { // down
		pitch-=0.05;
		printf("pitch: %f\n", pitch);
	}
	if(c=='a') { // left
		roll+=0.05;
		printf("roll: %f\n", roll);
	}
	if(c=='d') { // right
		roll-=0.05;
		printf("roll: %f\n", roll);
	}

	if(c=='1') {
		yaw+=0.1;
		printf("yaw: %f\n", yaw);
	}
	if(c=='2') {
		yaw-=0.1;
		printf("yaw: %f\n", yaw);
	}

	if(c=='e') { // pgup
		height+=10.;
		printf("height: %f\n", height);
	}
	if(c=='c') { // pgdown
		height-=10.;
		printf("height: %f\n", height);
	}*/

	ctl_SetSetpoint(roll,pitch,yaw,height);
  }
  ctl_Close();
  printf("\nDone...\n");
  return 0;

}
