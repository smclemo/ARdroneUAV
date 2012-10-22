/*
    main_video.h - video driver demo program

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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "video.h"
#include "blocksum.h"

int main(int argc,char ** argv)
{
	if(video_init() !=0) exit(-1);

	struct img_struct* img_old = video_CreateImage(1);
	struct img_struct* img_new = video_CreateImage(1);
	
	int dx,dy;
	int x=0,y=0;

	video_GrabImageGrey(img_old);
    for (;;) {
		video_GrabImageGrey(img_new);

/*
		char fn[100];
		snprintf(fn,100,"pgm_%d.pgm",img_new->seq);
		write_pgm(img_new,fn);
*/
		//process
		video_blocksum(img_old, img_new, &dx, &dy);
		x+=dx;
		y+=dy;
		printf("diff between img %5d and %5d -> dx=%2d dy=%2d x=%4d y=%4d\n",img_old->seq,img_new->seq,dx,dy,x,y);
		
		if(dx!=0 || dy!=0) {
			//swap buffers
			struct img_struct* tmp = img_new;
			img_new=img_old;
			img_old=tmp;
		}
    }

	video_close();
	
    return 0;
}
