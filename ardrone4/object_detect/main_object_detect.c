#include <stdio.h>   /* Standard input/output definitions */
#include <math.h>

#include "../util/type.h"
#include "../util/util.h"
#include "object_detect.h"


int main()
{
	int rc;
	struct object_detect_struct od;

	
	printf("Horizontal velocities test program\r\n");


	printf("Init od ...\r\n");
	rc = object_detect_init(&od);
	if(rc) return rc;
	printf("Init od OK\r\n");

	
	int c=0; 
	//main loop	
	double xpos = 0.0, ypos = 0.0;
	while(1) { 

		object_detect_getSample(&od);		
		xpos += od.locX;
		ypos += od.locY;
		c++;
		if(c%100 ==0) object_detect_print(&od, xpos, ypos);
	}
	object_detect_close();
	printf("\nDone...\n");
	return 0;
}
