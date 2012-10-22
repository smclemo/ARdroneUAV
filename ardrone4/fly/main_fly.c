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
#include "../udp/udp.h"
#include "../object_detect/object_detect.h"
#include "controlthread.h"

////////////////////////////
#include <assert.h>
#include <getopt.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <asm/types.h> 


int main()
{
	//wait for udp packet on port 7777
	//udp_struct udpCmd;
//	udpServer_Init(&udpCmd,7777,1/*blocking*/);
//	char buf[1024];

	float roll = 0;
	float pitch = 0;
	float yaw = 0;
	float height = 0;
	
	struct object_detect_struct od;
	
	int rc = system("(../../bin/program.elf ${PELF_ARGS}; gpio 181 -d ho 1) &");
	sleep(7);
	printf("Return code from program.elf = %i\n", rc);

	//kill program.elf
	rc = system("/usr/bin/killall program.elf > /dev/null 2>&1");
	printf("killall program.elf -> returncode=%d  (0=killed,256=not found)\n",rc);	

	//init controller
	while(ctl_Init(NULL))
	{
		printf("Flat trim failed. Retry in 2 seconds...\n");
		sleep(2);
	}
	printf("ctl_Init completed\n");

	//rc = object_detect_init(&od);
	//if (rc)
	//	return rc;
	
	//height = 0.5;
	ctl_SetSetpoint(roll,pitch,yaw,height);
	sleep(6);
	int counter = 0;

	int keyPress = -1;
	printf("Starting...\n");
	
	//main loop	
	while(keyPress==-1) 
	{ 
		keyPress = util_getch();
		//object_detect_getSample(&od);

		//yaw -= (float)od.locX/(4*1700);
		height += (float)od.locY/(4*2000);

		//printf("roll: %f pitch: %f yaw: %f height: %f\n",roll,pitch,yaw,height);
		//ctl_SetSetpoint(roll,pitch,yaw,height);
	}

	//printf("Framerate: %f frames per second\n", (double)numFrames/(util_timestamp()-timeStart));
	//printf("Landing...\n");
	ctl_SetSetpoint(roll,pitch,yaw,0);
	sleep(6);
	//object_detect_close();
	ctl_Close();
	

	printf("\nDone...\n");
	return 0;
}
