#include "video.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

void rgbToHsv( unsigned char r, unsigned char  g, unsigned char b, unsigned int *hue, unsigned char *saturation, unsigned char *value )
{
	float min, max, delta;
	float h,s,v;   

	h=s=v=0; 
	*saturation = (unsigned char) s;
	*value = (unsigned char) v;
	*hue = (unsigned int) h;

	min = min( r, min(g, b) );
	max = max( r, max(g, b) );
	v = max;				// v

	delta = max - min;

	if( max != 0 )
		s = min(delta*255 / max,255);		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		s = 0;
		h = -1;
		return;
	}

	if( r == max )
		h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		h = 4 + ( r - g ) / delta;	// between magenta & cyan
	h = h*60;
	if (h<0) h+=360;
	*saturation = (unsigned char) s;
	*value = (unsigned char) v;
	*hue = (unsigned int) h;
}

