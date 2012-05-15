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
#include "controlthread.h"

////////////////////////////
#include <assert.h>
#include <getopt.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h> 
#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
    void * start;
    size_t length;
};

struct point {
	int x;
	int y;
};

static char * dev_name = (char*) "/dev/video0";		//AR.Drone front camera
static int fd = -1;
struct buffer * buffers = NULL;
static unsigned int n_buffers = 0;
static unsigned long image_size;

// takes about 1.1ms. 
// Known issue: outliers on the right or bottom of the image have a greater effect when the blob
// is in the top of left of the image. Ideally should use median rather than mean but this would be too slow.
struct point findBlob(void* data_pointer, unsigned char Ymin, unsigned char Ymax, unsigned char Umin, unsigned char Umax, unsigned char Vmin, unsigned char Vmax)
{
	// pixel at location (x,y) has:
	// Y value i=x+320*y
	// U value i=320*240 + x/2+160*(int)(y/2)
	// V value i=320*240+160*120+x/2+160*(int)(y/2)
	
	struct point centre;
	centre.x = 0;
	centre.y = 0;
	unsigned int i, x, y, count = 0;
	unsigned char Y, U, V;
	
	for(i = 96000; i < 115200; i++)
	{
		V = *((unsigned char*)data_pointer + i);

		if(V>=Vmin && V<=Vmax)
		{
			*((unsigned char*)data_pointer + i) = 0;
			centre.x += 2*((i-96000)%160);
			centre.y += (i-96000)/80;
			count++;
		}
	}
	
	if(count > 5)
	{
		centre.x /= count;
		centre.y /= count;
	}
	else
	{
		centre.x = 160;
		centre.y = 120;
	}
	
	return centre;
}

static void* compress_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_DQBUF failed.\n");
		return NULL;    
    }

    assert(buf.index < n_buffers);
	void* data_pointer = buffers[buf.index].start;
	
	for(i = 0; i < image_size/4; i++)
	{
		// (x,y) = AVERAGE[(2x,4y)+(2x+1,4y)+(2x,4y+1)+(2x+1,4y+1)]
		// i = x + 640y
		// x = i%640, y = i/640
		unsigned int fact = 2*(i%640) + 2560*((int)(i/640));
		*((unsigned char*)data_pointer + i) = (	*((unsigned char*)data_pointer + (fact)) + 
												*((unsigned char*)data_pointer + (fact + 1)) +
												*((unsigned char*)data_pointer + (fact + 640)) + 
												*((unsigned char*)data_pointer + (fact + 641)))/4;							
	}
	
	if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_QBUF failed.\n");
		return NULL;
    }
	
	return data_pointer;
}

/////////////////////////////////////////
float Kp = 0.3;
float Ki = 0.0;
float Kd = -0.05;

float gain = 0.01;

void setPid() {
	setPidPitchRoll(Kp,Ki,Kd);
	//setPidYaw(Kp,Ki,Kd);
	//setPidHight(Kp,Ki,Kd);
}

int main()
{
	printf("Reservoir Lab QuadCopter control\n");
	//wait for udp packet on port 7777
	udp_struct udpCmd;
	udpServer_Init(&udpCmd,7777,1/*blocking*/);
	char buf[1024];

	setPid();

	float roll = 0;
	float pitch = 0;
	float yaw = 0;
	float hight = 0;

	int bufcnt;

	//kill program.elf
	int rc = system("/usr/bin/killall program.elf > /dev/null 2>&1");
	printf("killall program.elf -> returncode=%d  (0=killed,256=not found)\n",rc);	

	//init controller
	ctl_Init(inet_ntoa(udpCmd.si_other.sin_addr));
	printf("ctl_Init completed\n");

	struct v4l2_capability cap;
	unsigned int i;
	enum v4l2_buf_type type;

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);

	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) 
	{
		printf("ioctl() VIDIOC_QUERYCAP failed.\n");
		return -1;    
	}		

	struct v4l2_format fmt;
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;	//for AR.Drone front camera
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;

	if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) 
	{
		printf("ioctl() VIDIOC_S_FMT failed.\n");
		return -1;    
	}

	image_size = fmt.fmt.pix.width * fmt.fmt.pix.height *3/2;

	struct v4l2_requestbuffers req;

	CLEAR(req);
	req.count = 5;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) 
	{
		printf("ioctl() VIDIOC_REQBUFS failed.\n");
		return -1;    
	}

	//printf("Buffer count = %d\n", req.count);

	buffers = (buffer*) calloc(req.count, sizeof(*buffers));

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) 
		{
			printf("ioctl() VIDIOC_QUERYBUF failed.\n");
			return -1;    
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL, buf.length, 
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start) 
		{
			printf ("mmap() failed.\n");
			return -1;
		}
	}

	for (i = 0; i < n_buffers; ++i) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
		{
			printf("ioctl() VIDIOC_QBUF failed.\n");
			return -1;    
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd, VIDIOC_STREAMON, &type)< 0) 
	{
		printf("ioctl() VIDIOC_STREAMON failed.\n");
		return -1;    
	}

	fd_set fds;
	struct timeval tv;
	int r;

	hight = 50;
	int counter = 0;
	int numFrames = -1;
	double timeStart = 0.0;
	//main loop	
	while(counter < 850) 
	{ 
		counter++;

		int locX = 0;
		int locY = 0;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		tv.tv_sec = 2;
		tv.tv_usec = 0;
		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) 
		{
			if (EINTR == errno) continue; 
			printf("select err\n");
		}

		if (0 == r) 
		{
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}

		while(1) 
		{
			void* small_frame = compress_frame();
			if (small_frame != NULL) 
			{
				struct point blob_loc = findBlob(small_frame, 91, 100, 130, 136, 200, 240);
				int temp = (int)(blob_loc.x)/10;
				locX = 10*temp - 160;
				temp = (int)(blob_loc.y)/10;
				locY = -10*temp + 120;
				if (counter>80)
				{
					yaw -= (float)locX/1700;
					hight += (float)locY/20;
				}
				//printf("yaw: %f\n", yaw);
				if(timeStart <= 1.0)
				{
					timeStart = util_timestamp();
				}
				numFrames++;
				break;
			}
		}
		ctl_SetSetpoint(roll,pitch,yaw,hight,locX,locY);
	}

	printf("Framerate: %f frames per second\n", (double)numFrames/(util_timestamp()-timeStart));
	
	hight = 10;
	ctl_SetSetpoint(roll,pitch,yaw,hight,0,0);
	sleep(5);
	ctl_SetThrottleOff();
	sleep(1);
	ctl_Close();
  
	for (i = 0; i < n_buffers; ++i) 
	{
		if (-1 == munmap(buffers[i].start, buffers[i].length)) printf("munmap() failed.\n");
	}

	close(fd);
	printf("\nDone...\n");
	return 0;
}
