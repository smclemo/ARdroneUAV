//////////////////////////////////////
// Xbee sensor system class
// Author: Cooper Bills <csb88@cornell.edu>
// 6/7/10
//////////////////////////////////////

#ifndef XBEE_H
#define XBEE_H

#define XBEESERIALPORT "/dev/ttyUSB0"
#define XBEESENDERADDRESS 0xD00D

#include "Tools/smoothingmedianfilter.hpp"
#include <pthread.h>

class Xbee
{
  private:
    int getFrontDeriv();
  public:
    Xbee(int medianFilterValue, double smoothingRange);
    ~Xbee();
    int getRightDist();
    int getLeftDist();
    int getFrontDist();
    int updateFrontDeriv();
    int lastFrontDeriv();
    int deriv0, deriv1, deriv2, deriv3;
    int deriv0s, deriv1s, deriv2s, deriv3s; //smoothing
    SmoothingMedianFilter *adc0, *adc1, *adc2, *adc3;
    int fd;//, avgRate;
		pthread_t xbeethread;
		int threadid;
		bool done;
};

#endif
