/*
    video.h - video driver
*/ 
#ifndef _VIDEO_H
#define _VIDEO_H

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>	      // low-level i/o 
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

enum io_method {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

struct buffer {
	void *start;
	size_t length;
};

int video_init(char* videoName, int cam_width, int cam_height, int cam_fps);

void get_frame(unsigned char* image_out);

void get_frame_grey(unsigned char* image_out);

void drawDot(unsigned char* data_pointer, unsigned int x, unsigned int y, unsigned int radius);

void drawBox(unsigned char* data_pointer, unsigned int x, unsigned int y, unsigned int size);

void uyvyToGrey(unsigned char *dst, unsigned char *src, unsigned int numberPixels);

void video_close();

int video_frame_ready();

void video_start_recording();

void halfsize(unsigned char* halfim, unsigned char* im, int width, int height);

#endif
