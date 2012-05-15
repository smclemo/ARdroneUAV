// arm-none-linux-gnueabi-gcc -o v4l2_test v4l2_test.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#ifndef _V4L2_H
#define _V4L2_H


#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
    void * start;
    size_t length;
};

struct point {
	unsigned int x;
	unsigned int y;
};

static char* dev_name = (char*) "/dev/video0";		//AR.Drone front camera
static int fd = -1;
struct buffer* buffers = NULL;
static unsigned int n_buffers = 0;
FILE* file_fd;
static unsigned long image_size;
static unsigned char* file_name;

// Gets a frame. Executes in 6.7ms for front camera
static int read_frame(void);

//return timestamp in microseconds since first call to this function
int util_timestamp_int();

void drawDot(void* data_pointer, unsigned int x, unsigned int y, unsigned int radius);

// takes about 1.1ms. 
// Known issue: outliers on the right or bottom of the image have a greater effect when the blob
// is in the top of left of the image. Ideally should use median rather than mean but this would be too slow.
struct point findBlob(void* data_pointer, unsigned char Ymin, unsigned char Ymax, unsigned char Umin, unsigned char Umax, unsigned char Vmin, unsigned char Vmax);

static void* compressFrame(void);

int videoInit();

struct point processFrame();

int closeVideo();

#endif
