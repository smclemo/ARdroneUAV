/*
 * Created on 2010 by Tom Krajnik
 * This class provides a basic interface to the AR-drone 
 * it envelopes the at commands, navdata and stream from ARDrone original SDK
 * the helidata structure (check app.h) contains the drone status 
 */

#include "app.h"
#include "stdio.h"
#include "CRawImage.h" 

typedef struct{
	int axis[6];
	bool buttons[11];
}TJoyState;

class CHeli
{
private:

public:
	CHeli();
	~CHeli();

	//resets the drone state, recalibrates sensors and takes off 
	void takeoff();				

	//lands the drone
	void land();

	// chooses the camera - 0-front, 1-bottom, 2,3-picture in picture mode
	void switchCamera(int camera); 		

	// fills the "image" structure with the latest captured image, returns 0
	int renewImage(CRawImage* image);	

	//sets the pitch, roll, yaw and vertical speed
	void setAngles(float pitch, float roll,float yaw,float vertical);
	
private:
	//closes communication with the drone 
	void close();			
	bool landed;
	int imageWidth,imageHeight;
};

