/*
 *  
 *
 *  Steven Clementson 2012
 */

#include <termios.h> // POSIX terminal control definitions 
#include <stdio.h>
#include <stdlib.h>

#include "jni/cvd_lite/image.h"
#include "util.h"

#include "maths.h"
#include "app.h"

#include "jni/native.h"

//system("echo -en \"\\x60\\x00\" >/dev/ttyO0");	OFF
//system("echo -en \"\\x7f\\x00\" >/dev/ttyO0");	RED
//system("echo -en \"\\x60\\x1f\" >/dev/ttyO0");	GREEN
//system("echo -en \"\\x7f\\x36\" >/dev/ttyO0");	ORANGE

bool flying = false;
FILE *filt_log_file;

void init()
{	
	filt_log_file = fopen("main_logX.csv", "wb");
	if(filt_log_file==NULL)
	{
		printf("File 1 open failed\n");
	}
	fprintf(filt_log_file,"Time,Filt X (m),Filt Y (m),Filt Z (m),Filt Yaw (rad),Drone pitch,Drone roll,Drone yaw,Drone gaz,Waypoint Number\n");

	initnative();
	
	int rc = system("(../../bin/program.elf ${PELF_ARGS}; gpio 181 -d ho 1) &");
	sleep(7);
	printf("Return code from program.elf = %i\n", rc);

	//init control port
	at_run();
}

void uninit()
{
	//Land
	at_hover();
	sleep(1);
	at_stop();
	
	if(fclose(filt_log_file) != 0)
		printf("File 1 not closed!");
		
	uninitnative();
	
	system("/usr/bin/killall program.elf");
}

float32_t sgn(float32_t num)
{
	if(num<0) return -1;
	return 1;
}

float32_t abs(float32_t num)
{
	if(num<0) return -1*num;
	return num;
}

void validate_cmd(float32_t* pitch, float32_t* roll, float32_t* gaz, float32_t* yaw)
{
	if(*pitch != 0)
	{
		if(abs(*pitch) < PITCH_MIN)
			*pitch = sgn(*pitch)*PITCH_MIN;
		if(abs(*pitch) > PITCH_MAX)
			*pitch = sgn(*pitch)*PITCH_MAX;
	}
	
	if(*roll != 0)
	{
		if(abs(*roll) < ROLL_MIN)
			*roll = sgn(*roll)*ROLL_MIN;
		if(abs(*roll) > ROLL_MAX)
			*roll = sgn(*roll)*ROLL_MAX;
	}
	
	if(*gaz != 0)
	{
		if(abs(*gaz) < GAZ_MIN)
			*gaz = sgn(*gaz)*GAZ_MIN;
		if(abs(*gaz) > GAZ_MAX)
			*gaz = sgn(*gaz)*GAZ_MAX;
	}
	
	if(*yaw != 0)
	{
		if(abs(*yaw) < YAW_MIN)
			*yaw = sgn(*yaw)*YAW_MIN;
		if(abs(*yaw) > YAW_MAX)
			*yaw = sgn(*yaw)*YAW_MAX;
	}
}

int main()
{
	init();
	
	_float_or_int_t pitch, roll, gaz, yaw;
	pitch.f = 0.0f;
	roll.f = 0.0f;
	gaz.f = 0.0f;
	yaw.f = 0.0f;
	
	pose goal_location, waypoint_1, waypoint_2, waypoint_3, waypoint_4, current_location, correction;
	waypoint_1.x = 0.0;	//right of kinect positive x
	waypoint_1.y = -0.3;	//below kinect positive y
	waypoint_1.z = 0.0;		//in front of kinect positive z
	waypoint_1.yaw = 0.0; 	//turning left of kinect positive yaw
	
	waypoint_2.x = 0.0;	
	waypoint_2.y = -0.3;
	waypoint_2.z = -0.3;
	waypoint_2.yaw = 0.0;
	
	waypoint_3.x = -0.5;	
	waypoint_3.y = -0.3;
	waypoint_3.z = -0.3;	
	waypoint_3.yaw = 0.0;
	
	float32_t prev_x = 0, prev_z = 0, prev_time = 0;
	float32_t x_velocity = 0, z_velocity = 0;
	
	unsigned int waypoint_num = 1;
	unsigned int hover_count = 0;
		
	int keyPress = -1;
	printf("Starting...\n");
	
	//video_start_recording();
	
	while(keyPress==-1)
	{
		keyPress = util_getch();
		
		/*FAST CORNERS
		int numcorners_halfsize, numcorners_fullsize;
		
		xy* corners_halfsize = fast9_detect_nonmax((const byte*) frame_grey_halfsize, WIDTH/2, HEIGHT/2, WIDTH/2, 8, (int*)&numcorners_halfsize);
		printf("Corners found halfsize = %i\n", numcorners_halfsize);
		
		xy* corners_fullsize = fast9_detect_nonmax((const byte*) frame_grey, WIDTH, HEIGHT, WIDTH, 12, (int*)&numcorners_fullsize);
		printf("Corners found fullsize = %i\n\n", numcorners_fullsize);
	
		if(logging)
		{
			unsigned int j;
			for (j=0; j<numcorners_fullsize; j++)
				drawDot(frame, corners_fullsize[j].x, corners_fullsize[j].y, 6);
			
			fwrite((void*)frame, WIDTH*HEIGHT*2, 1, debug_file_fd);
		}*/	
		
		//CONTROL
		if(get_pose(&current_location))
		{			
			if(!flying)
			{
				//at_takeoff();
				flying = true;
				sleep(2);
			}
			else
			{
				switch(waypoint_num)
				{
					case 1:
						goal_location = waypoint_1;
						break;
					case 2:
						goal_location = waypoint_2;
						break;
					case 3:
						goal_location = waypoint_3;
						break;
					default:
						keyPress=1; 	//land
				}
				
				float32_t time_now = util_timestamp();
				float32_t dt = time_now - prev_time;
				x_velocity = (current_location.x - prev_x)/dt;
				z_velocity = (current_location.z - prev_z)/dt;
				
				prev_time = time_now;
				prev_x = current_location.x;
				prev_z = current_location.z;
				
				//printf("X velocity = %f, Z velocity = %f\n", x_velocity, z_velocity);
				
				if(abs(x_velocity) < 1)
					x_velocity = 0;
				if(abs(z_velocity) < 1)
					z_velocity = 0;
				
				correction.yaw = goal_location.yaw - current_location.yaw;
				correction.x = goal_location.x - current_location.x - x_velocity;
				correction.y = goal_location.y - current_location.y;
				correction.z = goal_location.z - current_location.z - z_velocity;
				
				// Reset command
				roll.f = 0.0f;
				pitch.f = 0.0f;
				gaz.f = 0.0f;
				yaw.f = 0.0f;
					
				if(abs(correction.yaw) > YAW_THRESHOLD)
				{
					yaw.f = -1*sgn(correction.yaw)*(YAW_MIN);
				}
				else 
				{					
					if(abs(correction.x) > X_THRESHOLD)
						roll.f = sgn(correction.x)*ROLL_MIN;
						
					if(abs(correction.z) > Z_THRESHOLD)
						pitch.f = -1*sgn(correction.z)*PITCH_MIN;
				
					if(abs(correction.y) > HEIGHT_THRESHOLD)
						gaz.f = -1*sgn(correction.y)*GAZ_MIN;
				}
				
				validate_cmd(&pitch.f, &roll.f, &gaz.f, &yaw.f);
				
				//printf("Waypoint: %d Command to drone: pitch %f, roll %f, gaz %f, yaw %f\n", waypoint_num, pitch.f, roll.f, gaz.f, yaw.f);
				printf("Filtered pose (x,y,z,yaw): %f, %f, %f, %f\n", current_location.x, current_location.y, current_location.z, current_location.yaw);
				
				at_set_radiogp_input(pitch.i, roll.i, gaz.i, yaw.i);

				fprintf(filt_log_file,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%d\n"
					,util_timestamp()		// timestamp in sec  
					,current_location.x   	// x loction            
					,current_location.y  	// y location    
					,current_location.z		// Z location
					,current_location.yaw		// relative yaw
					,pitch.f
					,roll.f
					,yaw.f
					,gaz.f
					,waypoint_num
					);
				
				if(abs(yaw.f)>0)		//YAWING
				{
					usleep(50000); 		//0.05 secs
				}
				else if(abs(pitch.f)>0 || abs(roll.f)>0 || abs(gaz.f)>0)	//OTHER MOVEMENT
				{
					usleep(50000);		//0.05 secs
				}
				else					//GOAL REACHED
				{
					at_hover();
					usleep(50000);			//1s - wait
					//waypoint_num++;
				}
				
				hover_count = 0;
			}
		}
		else
		{
			hover_count++;
			
			if(hover_count < 200)
				//at_set_radiogp_input(0, 0, 0, 0);
				at_hover();
			else if(hover_count < 300)		//Lost - spin left and right
			{
				yaw.f = -1*YAW_MIN;
				at_set_radiogp_input(0, 0, 0, yaw.i);
			}
			else
			{
				yaw.f = YAW_MIN;
				at_set_radiogp_input(0, 0, 0, yaw.i);
				if(hover_count == 400)
					hover_count = 200;
			}
				
			//printf("bad pose - hovering\n");
			//fprintf(filt_log_file,"%f,0,0,0,0,BAD,POSE,ERROR,HOVER,%d\n" ,util_timestamp(), waypoint_num);
			usleep(30000);		//30ms - wait for next update
		}
		
		/*if (frames_processed == 400)
		{			
			procframe(frame_grey, &current_location); 
			undistort(frame_grey); 
			saveKeypoints(frame_grey);
			
		}*/
	}
	
	uninit();
	printf("Ended successfully\n");
	return 0;
}
