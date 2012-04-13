/***********************************************
 * Smoothing Median Filter - caps inputs to a specific range from mean
 * Author: Cooper Bills (csb88@cornell.edu)
 * (Thread safe)
 ***********************************************/

#ifndef SMOOTHINGMEDIANFILTER_HPP
#define SMOOTHINGMEDIANFILTER_HPP

#include <pthread.h>

class SmoothingMedianFilter
{
	private:
    double *data;
    double maxRange;
    double avg;
    double getAvg();
    int *workspace;
    int size;
    int currentMedian;
    int nextInsert;
    pthread_mutex_t mutex;
    int findMaxIndexRefrence(double *referenced, int *array, int size);
    
	public:
	  SmoothingMedianFilter(int size, double range, double initVal);
	  ~SmoothingMedianFilter() {}
	  double getMedian();
	  void pushValue(double val);
};

#endif
