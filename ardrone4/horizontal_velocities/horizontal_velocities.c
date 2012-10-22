#include "../video/video.h"
#include "../video/blocksum.h"
#include "../util/util.h"
#include "horizontal_velocities.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** these are protected by mutex */

unsigned long seqNum;
double dt;
FILE *file_fd;

/** width of a pixel in meters at 1 m height */
#define CAM_HEIGHT_SCALE_FACTOR 0.005
#define BUF_SIZE 5

struct img_struct* img_old;
struct img_struct* img_new;
float xv_buffer[BUF_SIZE];
float yv_buffer[BUF_SIZE];
int buf_ind = 0;

pthread_t horizontal_velocities_thread;
pthread_mutex_t velocity_access_mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned char writeImagesToDisk=0;

void *horizontal_velocities_thread_main(void *data) 
{
	//file_fd = fopen("test.yuv", "wb");
	double prevTime = 0;
	
	video_GrabImageGrey(img_old);
	for (;;) 
	{
		//double t1=util_timestamp();
		video_GrabImageGrey(img_new);
		
		//double t2=util_timestamp();
		//printf("video grab took %f ms\n", (t2-t1)*1000);
		
		double currentTime = img_new->timestamp;
		
		int dxi, dyi;
		video_blocksum(img_old, img_new, &dxi, &dyi);
		
		//double t3=util_timestamp();
		//printf("blocksum took %f ms\n", (t3-t2)*1000);
		
		if (dxi != 0 || dyi != 0) 
		{
			//swap buffers
			struct img_struct* tmp = img_new;
			img_new = img_old;
			img_old = tmp;
		}
		
		pthread_mutex_lock(&velocity_access_mutex);
		
		dt = currentTime - prevTime;
		xv_buffer[buf_ind] = dxi / dt;
		yv_buffer[buf_ind] = dyi / dt;
		buf_ind = (buf_ind+1)%BUF_SIZE;
		
		seqNum++;
		prevTime = currentTime;
	
		pthread_mutex_unlock(&velocity_access_mutex);
		
		printf("%f\n", util_timestamp());
		
		//printf("\ndxi=%i dyi=%i cur=%f pre=%f dt=%f\n", dxi, dyi, currentTime, prevTime, dt);
		
		//if(writeImagesToDisk)
		//	fwrite((const void *)img_new->buf, 320*240, 1, file_fd);
		
		// bottom camera = 60Hz
		//usleep(10000);
	}

	video_close();
	return 0;
}

int horizontal_velocities_init(struct horizontal_velocities_struct *hv) {

	if (video_init() != 0)
	{
		printf("video init failed\n");
		exit(-1);
	}
	printf("video init done\n");
	
	img_old = video_CreateImage(1);
	img_new = video_CreateImage(1);

	seqNum = 0;

	int rc = pthread_create(&horizontal_velocities_thread, NULL,
			horizontal_velocities_thread_main, NULL);
	if (rc) {
		printf(
				"ctl_Init: Return code from pthread_create(hor_vel_thread) is %d\n",
				rc);
		return 202;
	}
	return 0;
}

void horizontal_velocities_getSample(struct horizontal_velocities_struct *hv,
		struct att_struct *att) {
	pthread_mutex_lock(&velocity_access_mutex);

	/**     @todo kalman or luenberger for filtering */
	if (dt > 0) {
		hv->xv = util_median(xv_buffer, BUF_SIZE) * att->h * CAM_HEIGHT_SCALE_FACTOR;
		hv->yv = util_median(yv_buffer, BUF_SIZE) * att->h * CAM_HEIGHT_SCALE_FACTOR;
		hv->dt = dt;
	}
	hv->seqNum = seqNum;
	pthread_mutex_unlock(&velocity_access_mutex);
}

void horizontal_velocities_print(struct horizontal_velocities_struct *hv,
		double xpos, double ypos, double h) {
	printf(
			"seq=%ld   xv=%5.1f,yv=%5.1f, dt=%4.1f xpos=%4.1f ypos=%4.1f h=%4.1f\n",
			hv->seqNum, hv->xv, hv->yv, hv->dt * 1000, xpos, ypos, h);
}

void horizontal_velocities_close()
{
	fclose(file_fd);
	video_close();
}

