#include "../video/video.h"
#include "../util/util.h"
#include "object_detect.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct img_struct* current_frame;

/** these are protected by mutex */

unsigned long seqNumF;
double dt2;
int locX, locY;

pthread_t object_detect_thread;
pthread_mutex_t location_access_mutex=PTHREAD_MUTEX_INITIALIZER;


struct point findBlob(struct img_struct* frame, unsigned char Ymin, unsigned char Ymax, unsigned char Umin, unsigned char Umax, unsigned char Vmin, unsigned char Vmax)
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
	
	// UYVY
	for(i = 2; i < frame->w*frame->h*2; i+=4)
	{
		V = *((unsigned char*)frame->buf + i);

		if(V>=Vmin && V<=Vmax)
		{
			*((unsigned char*)frame->buf + i) = 0;
			centre.x += (i%(frame->w*2))/2;
			centre.y += i/(frame->w*2);
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
		centre.x = frame->w/2;
		centre.y = frame->h/2;
	}
	
	return centre;
}


struct img_struct* compress_frame(struct img_struct* input_frame)
{
	int i;
	for(i = 0; i < input_frame->w*input_frame->h/2; i++)
	{
		// (x,y) = AVERAGE[(2x,4y)+(2x+1,4y)+(2x,4y+1)+(2x+1,4y+1)]
		// i = x + 640y
		// x = i%640, y = i/640
		unsigned int fact = 2*(i%640) + 2560*((int)(i/640));
		*((unsigned char*)input_frame->buf + i) = (	*((unsigned char*)input_frame->buf + (fact)) + 
												*((unsigned char*)input_frame->buf + (fact + 1)) +
												*((unsigned char*)input_frame->buf + (fact + 640)) + 
												*((unsigned char*)input_frame->buf + (fact + 641)))/4;							
	}
	
	return input_frame;
}


void *object_detect_thread_main(void *data)
{
	printf("Tracking...\n");
	double prevTime = 0.0;
	
    for (;;) 
	{
		//videoF_GrabImage(&vidF, current_frame);
	//  double start=util_timestamp();

		if (current_frame != NULL) 
		{		
			struct point blob_loc = findBlob(current_frame, 91, 100, 130, 136, 160, 240);
			
			double currentTime = util_timestamp();
			pthread_mutex_lock(&location_access_mutex);     

			dt2 = currentTime-prevTime;
			int temp = (int)(blob_loc.x)/10;
			locX = 10*temp - current_frame->w/2;
			temp = (int)(blob_loc.y)/10;
			locY = -10*temp + current_frame->h/2;
			//yaw -= (float)locX/(4*1700);
			//height += (float)locY/(4*2000);

			prevTime=currentTime;

			seqNumF++;
			
			pthread_mutex_unlock(&location_access_mutex);    
		}
    
	//	double endTime=util_timestamp();
	//	printf("   Loop took %f ms\n", (endTime-start)*1000);
    }

    //videoF_close();
	
    return 0;
}


int object_detect_init(struct object_detect_struct *od)
{
	//if(videoF_init() !=0) 
	//{
	//	printf("Open front camera failed\n");
	//	exit(-1);
	//}
	//current_frame = videoF_CreateImage(2);
	
	seqNumF=0;
	
	locX=0;
	locY=0;
	dt2=0;
	
	int rc = pthread_create(&object_detect_thread, NULL, object_detect_thread_main, NULL);
	if(rc) {
		printf("ctl_Init: Return code from pthread_create(object_detect_thread) is %d\n", rc);
		return 202;
	}
	return 0;
}


void object_detect_getSample(struct object_detect_struct *od)
{
     pthread_mutex_lock(&location_access_mutex);
          
     /**     @todo kalman or luenberger for filtering */  
     if(dt2>0) {
        od->locX=locX;
        od->locY=locY;
        od->dt=dt2;
     }
     od->seqNum=seqNumF;
     pthread_mutex_unlock(&location_access_mutex);     
}  


void object_detect_print(struct object_detect_struct *od, double xpos, double ypos)
{
	printf("seq=%ld   locX=%5.1f,locY=%5.1f, dt=%4.1f xpos=%4.1f ypos=%4.1f\n"
	        ,od->seqNum
	        ,od->locX
	        ,od->locY
			,od->dt*1000
			,xpos
			,ypos
	);
}

void object_detect_close()
{
	//videoF_close();
}
