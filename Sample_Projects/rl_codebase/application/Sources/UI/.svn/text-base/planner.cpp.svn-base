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
#include <sys/time.h>
#include <fstream>

using namespace std;

//Globablly Accessable Variables:
extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
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
  cvNamedWindow(cvBotAlgWindow, CV_WINDOW_AUTOSIZE);
  
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
	 sends the command to the drone for us. */

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
}

