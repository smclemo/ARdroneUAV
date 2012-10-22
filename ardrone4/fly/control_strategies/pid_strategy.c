#include "pid.h"
#include "../controls.h"
#include "pid_strategy.h"
#include "../../util/util.h"
#include <stdio.h>
#include <math.h>

float findBearing(float mag_x, float mag_y);


struct pid_struct pid_roll;
struct pid_struct pid_pitch;
struct pid_struct pid_yaw;


struct pid_struct pid_hor_vel_x;
struct pid_struct pid_hor_vel_y;
struct pid_struct pid_h;


double targetRoll;
double targetPitch;

float adj_roll;
float adj_pitch;
float adj_yaw;
float adj_h;

float throttle;

int logcnt2=0;
FILE *fp;

// minimum throttle for launching 
#define MOTOR_INIT_THROTTLE 0.2
// ramp end throttle
#define MOTOR_TAKEOF_THROTTLE 0.56
/** ramp progress while launching*/
#define LAUNCHRAMP_LENGTH 1000 // 200 ^= 1 second
int launchRamp;

struct setpoint_struct setpoint_landing={0,0,0,0.2};

void pid_strategy_init()
{
	//File Output
	fp = fopen("flightLog.csv", "wb");
	if(fp==NULL)
	{
		printf("File open failed\n");
		return;
	}
	fprintf(fp,"Log Count,Time Stamp,Acceleration x,Acceleration y,"
				"Acceleration z,Gyro x,Gyro y,Gyro z,Vertical Speed,"
				"Desired Height,Actual Height,Throttle,Desired Pitch,"
				"Actual Pitch,Pitch Adjustment,Desired Roll,Actual Roll,"
				"Roll Adjustment,Desired Yaw,Actual Yaw,Yaw Adjustment,"
				"Magnet X,Magnet Y,Magnet Z,Bearing,Horizontal xv,Horizontal yv,"
				/*"Blob x-coord,Blob y-coord*/"\n");
  
  //init pid pitch/roll 
	pid_Init(&pid_roll,  0.4,  0.000, -0.05, 0);
	pid_Init(&pid_pitch, 0.4,  0.000, -0.05, 0);

	pid_Init(&pid_yaw, 0.8, 0.008, -0.065, 0);
	
	/** @todo these need an i-part */
	//pid_Init(&pid_hor_vel_x, 0.01, 0.0, 0, 0.1);
	//pid_Init(&pid_hor_vel_y, 0.01, 0.0, 0, 0.1);
	
	pid_Init(&pid_h, 0.26, 0.023, -0.014, 1);
	
	throttle = 0.00;
}

void pidStrategy_calculateMotorSpeedsFlying(struct horizontal_velocities_struct *hv, struct att_struct *att, struct setpoint_struct *setpoint, struct control_limits_struct *control_limits, float motorOut[4])
{
	/* overwrite setpoints for now */
	targetRoll=setpoint->roll;
	targetPitch=setpoint->pitch;
	
	if(0) {
	  targetRoll =pid_Calc(&pid_hor_vel_y, -hv->yv, -hv->dt);
	  targetPitch=pid_Calc(&pid_hor_vel_x, -hv->xv, -hv->dt);
	}   

	//flying, calc pid controller corrections
	adj_roll  = pid_CalcD(&pid_roll,  targetRoll  - att->roll , att->dt, att->gx); //err positive = need to roll right
	adj_pitch = pid_CalcD(&pid_pitch, targetPitch - att->pitch, att->dt, att->gy); //err positive = need to pitch down
	adj_yaw   = pid_CalcD(&pid_yaw,   setpoint->yaw   - att->yaw,   att->dt, att->gz); //err positive = need to increase yaw to the left
	adj_h     = pid_CalcD(&pid_h,     setpoint->h     - att->h,     att->dt, att->hv); //err positive = need to increase height

	throttle = control_limits->throttle_hover + adj_h;
	if (throttle < control_limits->throttle_min)
		throttle = control_limits->throttle_min;
	if (throttle > control_limits->throttle_max)
		throttle = control_limits->throttle_max;

	//convert pid adjustments to motor values
	motorOut[0] = throttle + adj_roll - adj_pitch + adj_yaw;
	motorOut[1] = throttle - adj_roll - adj_pitch - adj_yaw;
	motorOut[2] = throttle - adj_roll + adj_pitch + adj_yaw;
	motorOut[3] = throttle + adj_roll + adj_pitch - adj_yaw;
	
	if (motorOut[0]<control_limits->throttle_hover/3)
		motorOut[0] = control_limits->throttle_hover/3;
	if (motorOut[1]<control_limits->throttle_hover/3)
		motorOut[1] = control_limits->throttle_hover/3;
	if (motorOut[2]<control_limits->throttle_hover/3)
		motorOut[2] = control_limits->throttle_hover/3;
	if (motorOut[3]<control_limits->throttle_hover/3)
		motorOut[3] = control_limits->throttle_hover/3;
}


void pid_strategy_calculateMotorSpeeds(struct drone_state_struct* cs, float motorOut[4])
{
	struct setpoint_struct* setpoint = &cs->setpoint;
	switch(cs->flyState) {
	  case Landed:
		for(int i=0;i<4;i++) motorOut[i]=0;
		if(setpoint->h>0) switchState(cs,Launching);
	  break;
	  
	  case Launching:
		launchRamp++;
		for(int i=0;i<4;i++) motorOut[i]=launchRamp*(MOTOR_TAKEOF_THROTTLE-MOTOR_INIT_THROTTLE)/LAUNCHRAMP_LENGTH+MOTOR_INIT_THROTTLE;

		if (cs->att.h > 0.4 || launchRamp > LAUNCHRAMP_LENGTH) 
		{
			//launchRamp = 1000;
			switchState(cs,Flying);
		}
		if (setpoint->h < 0.1) switchState(cs,Landing);
	  break;

	  case Flying:
		if (setpoint->h < 0.1) 
			switchState(cs,Landing);
		else
			pidStrategy_calculateMotorSpeedsFlying(&cs->hor_velocities, &cs->att,setpoint,&cs->control_limits,motorOut);
	  break;

	  case Landing:
		if(cs->att.h > 0.4)
		{
			cs->setpoint.h = cs->att.h - 0.05;
			pidStrategy_calculateMotorSpeedsFlying(&cs->hor_velocities, &cs->att,setpoint,&cs->control_limits,motorOut);
		}
		else 
		{
			launchRamp--;
			for(int i=0;i<4;i++) motorOut[i]=launchRamp*(MOTOR_TAKEOF_THROTTLE-MOTOR_INIT_THROTTLE)/LAUNCHRAMP_LENGTH+MOTOR_INIT_THROTTLE;

			if (launchRamp <= 400) 
			{
				cs->setpoint.h = 0;
				switchState(cs,Landed);
			}
		}
	  break;

	  case Error:
		for(int i=0;i<4;i++) motorOut[i]=0;
		if(setpoint->h==0) switchState(cs,Landed);
		launchRamp=0;
	  break;
	}
	navLog_Save(cs);
}

/*void msleep(unsigned long milisec)
{
    struct timespec req={0},rem={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
	nanosleep(&req,&rem);
}*/

unsigned int pid_strategy_getLogText(char *buf,unsigned int maxLen)
{
  int len;
  len= snprintf(buf,maxLen,
        "%f,%f,%f,%f,%f,%f,"
        ,targetPitch
        ,targetRoll
        ,adj_pitch
        ,adj_roll
        ,adj_yaw
        ,adj_h
      );
  return len;      
}

//logging
void navLog_Save(struct drone_state_struct *cs)
{
  logcnt2++;
  fprintf(fp,"%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"
    //sequence+timestamp
    ,logcnt2
    ,cs->att.ts   // navdata timestamp in sec
    //sensor data
    ,cs->att.ax   // acceleration x-axis in [G] front facing up is positive         
    ,cs->att.ay   // acceleration y-axis in [G] left facing up is positive                
    ,cs->att.az   // acceleration z-axis in [G] top facing up is positive             
    ,RAD2DEG(cs->att.gx)   // gyro value x-axis in [deg/sec] right turn, i.e. roll right is positive           
    ,RAD2DEG(cs->att.gy)   // gyro value y-axis in [deg/sec] right turn, i.e. pirch down is positive                     
    ,RAD2DEG(cs->att.gz)   // gyro value z-axis in [deg/sec] right turn, i.e. yaw left is positive 
    ,cs->att.hv   // vertical speed [cm/sec]
    //height
    ,cs->setpoint.h  // setpoint height
    ,cs->att.h       // actual height above ground in [cm] 
    ,throttle    // throttle setting 0.00 - 1.00
    //pitch
    ,RAD2DEG(cs->setpoint.pitch)  //setpoint pitch [deg]
    ,RAD2DEG(cs->att.pitch)       //actual pitch   
    ,adj_pitch                //pitch motor adjustment 
    //roll
    ,RAD2DEG(cs->setpoint.roll)   //setpoint roll [deg]
    ,RAD2DEG(cs->att.roll)        //actual roll  
    ,adj_roll                 //roll motor adjustment 
    //yaw
    ,RAD2DEG(cs->setpoint.yaw)    //yaw pitch [deg]
    ,RAD2DEG(cs->att.yaw)         //actual yaw  
    ,adj_yaw                  //yaw motor adjustment
	,cs->att.mag_x 				//magX = cos(HEADING/2)
	,cs->att.mag_y				//magY = -sin(HEADING)
	,cs->att.mag_z
	,RAD2DEG(findBearing(cs->att.mag_x, cs->att.mag_y))
	,cs->hor_velocities.xv
	,cs->hor_velocities.yv
	//,setpoint.locX				//blob x location
	//,setpoint.locY				//blob y location
  );    
}

int min(float* array, int size)
{
	int minIndex = 0;
	int i;
	for(i=1; i<size; i++)
	{
		if (array[i]<array[minIndex])
			minIndex = i;
	}
	return minIndex;
}

float average(float a, float b)
{
	return (a+b)/2;
}

float findBearing(float mag_x, float mag_y)
{
	float x_scaled = (mag_x-20)/60;
	if(x_scaled<-1) x_scaled = -1;
	if(x_scaled>1) x_scaled = 1;

	float y_scaled = (mag_y)/60;
	if(y_scaled<-1) y_scaled = -1;
	if(y_scaled>1) y_scaled = 1;

	float angle1, angle2;
	if(x_scaled<=0 && y_scaled<=0)
	{
		angle1 = asin(x_scaled) + 2*M_PI;
		angle2 = acos(y_scaled) + M_PI;
	}
	if(x_scaled<=0 && y_scaled>0)
	{
		angle1 = -asin(x_scaled) + M_PI;
		angle2 = acos(y_scaled) + M_PI;
	}
	if(x_scaled>0 && y_scaled<=0)
	{
		angle1 = asin(x_scaled);
		angle2 = -acos(y_scaled) + M_PI;
	}
	if(x_scaled>0 && y_scaled>0)
	{
		angle1 = -asin(x_scaled) + M_PI;
		angle2 = -acos(y_scaled) + M_PI;
	}
	
	return average(angle1, angle2);
	/*float angle1 = asin(x_scaled);
	if(x_scaled < 0)
		angle1 += 2*M_PI;
	float angle2 = -asin(x_scaled) + M_PI;
	float angle3 = acos(-y_scaled);
	float angle4 = -acos(-y_scaled) + 2*M_PI;

	printf("1: %f  2: %f  3: %f  4: %f\n", angle1, angle2, angle3, angle4);
	float diff[4];
	diff[0] = fabs(angle1-angle3);
	diff[1] = fabs(angle1-angle4);
	diff[2] = fabs(angle2-angle3);
	diff[3] = fabs(angle2-angle4);

	switch (min(diff, 4))
	{
		case 0:
			return average(angle1, angle3);
			break;
		case 1:
			return average(angle1, angle4);
			break;
		case 2:
			return average(angle2, angle3);
			break;
		case 3:
			return average(angle2, angle3);
			break;
	}*/
}

void pid_strategy_Close() 
{
	if (fclose(fp) != 0)
	printf("File not closed!");
}