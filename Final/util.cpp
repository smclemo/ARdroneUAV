/*
    util.c - utilities

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
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <time.h>  
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"

double led_time_on;

//non blocking getchar
int util_getch(void)
{
	struct termios oldt,
	newt;
	int ch=-1;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	fcntl(STDIN_FILENO, F_SETFL, O_NDELAY);
	ch = getchar();
	fcntl(STDIN_FILENO, F_SETFL, 0);
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

//return timestamp in seconds with microsecond resolution
double offset;
unsigned int first = 1;
struct timeval tv;

double util_timestamp()
{
	if(first)
	{
		first = 0;
		//offset = timeNow;
		struct timeval now;
		now.tv_sec=0;
		now.tv_usec=0;
		int rc = settimeofday(&now, NULL);
		if(rc!=0)
			printf("time not set %d\n", errno);
	}
	
	gettimeofday(&tv, NULL); 
	double timeNow = (double)tv.tv_sec+((double)tv.tv_usec)/1000000;
	
	return timeNow;// - offset;
}

//return timestamp in microseconds since first call to this function
int util_timestamp_int()
{
  static struct timeval tv1;
  struct timeval tv2;
  if(tv1.tv_usec==0 && tv1.tv_sec==0) gettimeofday(&tv1, NULL); 
  gettimeofday(&tv2, NULL); 
  return (int)(tv2.tv_sec-tv1.tv_sec)*1000000+(int)(tv2.tv_usec-tv1.tv_usec);
}

int floatcomp(const void* elem1, const void* elem2)
{
    if(*(const float*)elem1 < *(const float*)elem2)
        return -1;
    return *(const float*)elem1 > *(const float*)elem2;
}

float util_median(float* array, int length)
{
	qsort(array, length, sizeof(float), floatcomp);
	return array[length/2];
}

void util_notify(double time_send)
{
	led_time_on = time_send;
}

double util_get_led_time()
{
	return led_time_on;
}