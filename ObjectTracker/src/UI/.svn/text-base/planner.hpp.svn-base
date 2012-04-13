#ifndef _PLANNER_H_
#define _PLANNER_H_

#include <opencv/highgui.h>
#include <pthread.h>

extern IplImage* frontImgStream; 
extern IplImage* bottomImgStream;

class Planner
{
public:
  Planner();
  ~Planner();
  
  int threadid;
  pthread_t plannerthread;
  bool running; //If the system is running    
  bool enabled; //If the algorithms are enabled (should run)
  int32_t dpitch, dyaw, droll, dgaz; //Current Algorithm Results
  int32_t dpitch_final, dyaw_final, droll_final, dgaz_final; //Final Algorithm Results
};
    
#endif
