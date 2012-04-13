/******************************
 * Flight Planner Skeleton
 * Cooper Bills (csb88@cornell.edu)
 * Cornell University
 * 1/4/11
 ******************************/
#define cvAlgWindow "AlgorithmView"
#define cvBotAlgWindow "BottomAlgorithmView"

#include <ardrone_api.h>
#include <stdio.h>
#include <pthread.h>
#include "planner.hpp"
#include "Navdata/NavDataContainer.hpp"
#include "Tools/coopertools.hpp"
#include "xbee/xbee.hpp"
#include "gamepad.h"
#include <sys/time.h>
#include <fstream>

#include <iostream>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>

//Globablly Accessable Variables:
extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
extern int32_t ALTITUDE; //Altitude of drone in mm (defined in Navdata/navdata.c)
extern float32_t PSI; //Current Direction of Drone (-180deg to 180deg) (defined in Navdata/navdata.c)
extern NavDataContainer globalNavData; //Navdata (defined in Navdata/NavDatacontainer.cpp)
extern int newFrameAvailable; //if navdata indicates a new frame (defined in Navdata/navdata.c)
extern int saveColourNow;

Xbee *xbee;


//*****************************************************************

using namespace std;

#define GAZ_GAIN 25
#define YAW_GAIN 25

// Computes direct distance between 2 points using Pythagoras
int distanceDirect(CvPoint a, CvPoint b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

// Computes bearing heading between 2 points in degrees from North clockwise
int trajectory(CvPoint from, CvPoint to)
{
	int deltaX = to.x - from.x;
	int deltaY = from.y - to.y;

	if(deltaX > 0)
	{
		if(deltaY > 0)
		{
			cout << "Q1: ";
			return (180/PI)*atan(deltaX/deltaY);			// Quadrant 1
		}
		cout << "Q4: ";
		return 90 + (180/PI)*atan(-deltaY/deltaX);			// Quadrant 4
	}
	if(deltaY > 0)
	{
		cout << "Q2: ";
		return (int)(360 - (180/PI)*atan(-deltaX/deltaY)) % 360;	// Quadrant 2
	}
	cout << "Q3: ";
	if(deltaY == 0)
		return 270;
	return 180 +(180/PI)* atan(deltaX/deltaY);				// Quadrant 3
}

CvPoint contourCentre(CvSeq* contour)
{
	CvPoint centre = cvPoint(0, 0);
	for(int i=0; i<contour->total; i++)
	{
		CvPoint* temp = (CvPoint*) CV_GET_SEQ_ELEM(CvPoint, contour, i);
		centre.x += temp->x;
		centre.y += temp->y;
	}
	centre.x /= contour->total;
	centre.y /= contour->total;

	return centre;
}

CvSeq* findBlobContour(IplImage* image, int* matchColour, int thres)
{
	CvScalar lb = cvScalar(matchColour[0]-thres, matchColour[1]-thres,
			matchColour[2]-thres);
	CvScalar ub = cvScalar(matchColour[0]+thres, matchColour[1]+thres,
			matchColour[2]+thres);

	IplImage* lowerBound = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	cvSet(lowerBound, lb);
	IplImage* upperBound = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	cvSet(upperBound, ub);


	cvNamedWindow("ub", CV_WINDOW_AUTOSIZE);
	cvShowImage("ub", upperBound);

	cvNamedWindow("lb", CV_WINDOW_AUTOSIZE);
	cvShowImage("lb", lowerBound);



	cvNamedWindow("conv", CV_WINDOW_AUTOSIZE);
	cvShowImage("conv", image);

	// Create binary image
	IplImage* mask = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 1);;
	cvInRange(image, lowerBound, upperBound, mask);

	cvNamedWindow("mask", CV_WINDOW_AUTOSIZE);
	cvShowImage("mask", mask);

	// Erode and Dilate
	IplConvKernel* elem = cvCreateStructuringElementEx(5, 5, 0, 0, CV_SHAPE_RECT);
	CvArr* temp;
	cvMorphologyEx(mask, mask, temp, elem, CV_MOP_CLOSE);
	cvMorphologyEx(mask, mask, temp, elem, CV_MOP_OPEN);

	cvNamedWindow("cleanMask", CV_WINDOW_AUTOSIZE);
	cvShowImage("cleanMask", mask);

	CvMemStorage* contours = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	cvFindContours(mask, contours, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	if(contour == 0)
		return NULL;

	// Find largest contour
	CvSeq* largestContour = contour;
	for( ; contour != 0; contour = contour->h_next )
        {
		if (cvContourArea(contour) > cvContourArea(largestContour))
			largestContour = contour;
        }

	return largestContour;
}

//***************************************************************************************



//Thread to run separately:
void *Planner_Thread(void *params)
{
	Planner *self = (Planner *)params;
	xbee = new Xbee(7, 12.0);
	while(!newFrameAvailable) {usleep(100000);} //wait for initialization
  
 	//******** Add Your Initialization Below this Line *********
	IplImage*	frame = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	//int       	key = 0;
	bool		objectSaved = false;
	int		objectColour[3];
	IplImage*	converted = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	CvSeq*		blobContour;
	CvPoint		blobLocation;
	//int		bearing;

	cvNamedWindow("output", CV_WINDOW_AUTOSIZE);
	

  	//******** Add Your Initialization Above this Line *********

  	while(self->running) //Continue to loop until system is closed
  	{
    		if(self->enabled)
    		{

			if(!newFrameAvailable || frame == NULL)
			{
				usleep(100000);
				//printf("\n    Uh-Oh!  No New Frames \n\n");
				//If you would like to handle this situation, put it here.
				continue; //planner will only run when new data is available
			}

			cvCopy(frontImgStream, frame);
			newFrameAvailable = 0; //reset
			xbee->updateFrontDeriv(); //Update xbee (if applicable)
			

			//******** Edit Below this Line **********

			/* When algorithms are enabled, This loop will loop indefinitely
			 (until algorithms are disabled and manual control is
			 regained).  The goal of this loop is to take the input from
			 the drone (images, sensors, etc.), process it, then output
			 control values.  There are 4 axis of control on our drones:
			 pitch (forward/backward), roll (left/right), yaw (turning),
			 and gaz (up/down).  An external thread reads dXXX_final and
			 sends the command to the drone for us. */

			// Commands are values from -25000 to 25000:
			// Pitch - Forward is Negative.
			// Roll - Right is Positive.
			// Yaw - Right is Positive.
			// Gaz - Up is Negative.

			// For example, we want the drone to remain in place, but hover ~1.5m:
			// A simple proportional controller:
			
		  	CvPoint centre = cvPoint(frame->width/2, frame->height/2);

			if(objectSaved)
			{
				//counter = (counter+1)%10;
				cvCvtColor(frame, converted, CV_BGR2YCrCb);
				blobContour = findBlobContour(converted, objectColour, 30);

				// Find centre point of contour
				if(blobContour != NULL && cvContourArea(blobContour) > 200)
				{
					CvPoint newLocation = contourCentre(blobContour);

					if((distanceDirect(newLocation, blobLocation) < 100)
							|| (blobLocation.x == 0))
					{
						//bearing = trajectory(oldBlobLocation, newLocation);
						blobLocation = newLocation;
					}

					// Update desired colour
					//objectColour = &CV_IMAGE_ELEM(converted, unsigned char, blobLocation.y, 3*blobLocation.x);

					// Draw the contour
					cvDrawContours(frame, blobContour, CV_RGB(255, 0, 0), CV_RGB(255, 0, 0), 0);
					cvCircle(frame, blobLocation, 5, CV_RGB(0, 255, 0), 7);
				}
				else
					blobLocation = cvPoint(0, 0);
			}
			else
			{
				cvCircle(frame, centre, 10, CV_RGB(255, 0, 0));

				if(saveColourNow == 1)
				{
					cvCvtColor(frame, converted, CV_BGR2YCrCb);
					CvScalar pix = cvGet2D(converted, centre.y, centre.x);

					objectColour[0] = pix.val[0];
					objectColour[1] = pix.val[1];
					objectColour[2] = pix.val[2];

					/*uchar* data = (uchar*) converted->imageData;
					int step = converted->widthStep/sizeof(uchar);
					objectColour[0] = data[centre.y*step+centre.x*3+1];
					objectColour[1] = data[centre.y*step+centre.x*3+2];
					objectColour[2] = data[centre.y*step+centre.x*3+3];*/
					objectSaved = true;
					saveColourNow = 0;
				}
				blobLocation = centre;
			}

			cvShowImage("output", frame);

			int deltaX = blobLocation.x - centre.x;
			int deltaY = blobLocation.y - centre.y;
			//******** Edit Above this Line **********


			self->dpitch = 0; // No forward motion
			self->droll = 0; // No side-to-side motion
			self->dyaw = deltaX*YAW_GAIN; // No turning motion
			self->dgaz = deltaY*GAZ_GAIN; // Adjust altitude by difference from goal.

			//Apply commands all at once
			self->dgaz_final = self->dgaz - deltaY*GAZ_GAIN;
			self->dyaw_final = self->dyaw + deltaX*YAW_GAIN;
			self->droll_final = self->droll;
			self->dpitch_final = self->dpitch;

    } //end if enabled
    else
    {
      pthread_yield();
    }
    usleep(50000);
  }

  //De-initialization 
  printf("thread returned\n");
  return 0;
}


Planner::Planner()
{
  dpitch_final = 0;
  dyaw_final = 0;
  droll_final = 0;
  dgaz_final = 0;
  dpitch = 0;
  dyaw = 0;
  droll = 0;
  dgaz = 0;
  enabled = false;
  running = true;
  
  //Create the Planner Thread
  threadid = pthread_create( &plannerthread, NULL, Planner_Thread, (void*) this);
}

Planner::~Planner()
{
  //destructor
  running = false;
}





/******************************
 * Flight Planner Skeleton
 * Cooper Bills (csb88@cornell.edu)
 * Cornell University
 * 1/4/11
 ******************************
#define cvAlgWindow "AlgorithmView"
#define cvBotAlgWindow "BottomAlgorithmView"

#include <ardrone_api.h>
#include <stdio.h>
#include <pthread.h>
#include "planner.hpp"
#include "Navdata/NavDataContainer.hpp"
#include "Tools/coopertools.hpp"
#include "xbee/xbee.hpp"
#include <sys/time.h>
#include <fstream>

using namespace std;

//Globablly Accessable Variables:
//extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
//extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
extern int32_t ALTITUDE; //Altitude of drone in mm (defined in Navdata/navdata.c)
extern float32_t PSI; //Current Direction of Drone (-180deg to 180deg) (defined in Navdata/navdata.c)
extern NavDataContainer globalNavData; //Navdata (defined in Navdata/NavDatacontainer.cpp)
extern int newFrameAvailable; //if navdata indicates a new frame (defined in Navdata/navdata.c)

Xbee *xbee;

//Thread to run separately:
void *Planner_Thread(void *params)
{
  Planner *self = (Planner *)params;
  xbee = new Xbee(7, 12.0);
  while(frontImgStream == NULL) {usleep(100000);} //wait for initialization
  cvStartWindowThread();
  cvNamedWindow(cvAlgWindow, CV_WINDOW_AUTOSIZE);
  //cvNamedWindow(cvBotAlgWindow, CV_WINDOW_AUTOSIZE);
  
  //******** Add Your Initialization Below this Line *********


  //******** Add Your Initialization Above this Line *********

  while(self->running) //Continue to loop until system is closed
  {
    if(self->enabled)
    {
      if(!newFrameAvailable)
      {
        usleep(100000);
        if(!newFrameAvailable) printf("\n    Uh-Oh!  No New Frames \n\n");
	//If you would like to handle this situation, put it here.
        continue; //planner will only run when new data is available
      }
      newFrameAvailable = 0; //reset
      xbee->updateFrontDeriv(); //Update xbee (if applicable)
      
      //******** Edit Below this Line **********

      /* When algorithms are enabled, This loop will loop indefinitely
	 (until algorithms are disabled and manual control is
	 regained).  The goal of this loop is to take the input from
	 the drone (images, sensors, etc.), process it, then output
	 control values.  There are 4 axis of control on our drones:
	 pitch (forward/backward), roll (left/right), yaw (turning),
	 and gaz (up/down).  An external thread reads dXXX_final and
	 sends the command to the drone for us. 

      // Commands are values from -25000 to 25000:
      // Pitch - Forward is Negative.
      // Roll - Right is Positive.
      // Yaw - Right is Positive.
      // Gaz - Up is Negative.

      // For example, we want the drone to remain in place, but hover ~1.5m:
      // A simple proportional controller:
      self->dpitch = 0; // No forward motion
      self->droll = 0; // No side-to-side motion
      self->dyaw = 0; // No turning motion
      self->dgaz = ALTITUDE - 1500; // Adjust altitude by difference from goal.


      //******** Edit Above this Line **********

      //Apply commands all at once
      self->dgaz_final = self->dgaz;
      self->dyaw_final = self->dyaw;
      self->droll_final = self->droll;
      self->dpitch_final = self->dpitch;

    } //end if enabled
    else
    {
      pthread_yield();
    }
    usleep(50000);
  }

  //De-initialization 
  cvDestroyWindow(cvAlgWindow);
  cvDestroyWindow(cvBotAlgWindow);
  printf("thread returned\n");
  return 0;
}


Planner::Planner()
{
  dpitch_final = 0;
  dyaw_final = 0;
  droll_final = 0;
  dgaz_final = 0;
  dpitch = 0;
  dyaw = 0;
  droll = 0;
  dgaz = 0;
  enabled = false;
  running = true;
  
  //Create the Planner Thread
  threadid = pthread_create( &plannerthread, NULL, Planner_Thread, (void*) this);
}

Planner::~Planner()
{
  //destructor
  running = false;
}*/

