/*
    video.h - video driver

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
#ifndef _VIDEO_H
#define _VIDEO_H

#include <sys/types.h>


struct img_struct {
	int seq;
	double timestamp;
	unsigned char *buf;
	int w;
	int h;
	int bytesPerPixel;
};


int video_init();
//create a new blank image
struct img_struct *video_CreateImage(int bytesPerPixel);
//grabs next B&W image from stream (blocking)
void video_GrabImageGrey(struct img_struct* imageOut);
void video_close();
void uyvyToGrey(unsigned char *dst, unsigned char *src, unsigned int numberPixels);
void write_pgm(struct img_struct *img, char *fn);
#endif
