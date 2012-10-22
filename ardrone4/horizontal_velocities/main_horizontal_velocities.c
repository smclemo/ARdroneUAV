#include <stdio.h>   /* Standard input/output definitions */
#include <math.h>

#include "../util/type.h"
#include "../util/util.h"
#include "horizontal_velocities.h"


int main()
{
	int rc;
	struct horizontal_velocities_struct hv;
	struct att_struct att;

	
	printf("Horizontal velocities test program\r\n");

	//init att board
	printf("Init Attitute Estimate ...\r\n");
	rc = att_Init(&att);
	if(rc) return rc;
	printf("Init Attitute Estimate OK\r\n");

	printf("Init HV ...\r\n");
	rc = horizontal_velocities_init(&hv);
	if(rc) return rc;
	printf("Init HV OK\r\n");

	
	//int c=0; 
	//main loop	
	double xpos = 0.0, ypos = 0.0;
	while(1) { 
		rc = att_GetSample(&att);
		if(rc) {
			printf("ERROR: att_getSample return code=%d\n",rc); 
		}

		horizontal_velocities_getSample(&hv,&att);		
		xpos += hv.xv * att.dt;
		ypos += hv.yv * att.dt;
		//c++;
		//if(c%100 ==0) horizontal_velocities_print(&hv, xpos, ypos, att.h);
	}
	horizontal_velocities_close();
	att_Close();
	printf("\nDone...\n");
	return 0;
}
