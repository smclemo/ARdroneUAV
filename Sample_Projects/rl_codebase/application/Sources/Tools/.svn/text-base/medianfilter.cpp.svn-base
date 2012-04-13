
#include "medianfilter.hpp"
#include <stdio.h>
#include <stdlib.h>

MedianFilter::MedianFilter(int initsize, double initVal)
{
  if(initsize <= 0) initsize = 1;
  if(!(initsize % 2)) printf("Warning: even size given to Median Filter - this filter works best with odd sizes.\n");
  currentMedian = -1;
  nextInsert = 0;
  size = initsize;
  data = (double *)malloc(sizeof(double)*size);
  workspace = (int *)malloc(sizeof(int)*size);
  for(int i = 0; i < size; i++)
    data[i] = initVal;
  mutex = PTHREAD_MUTEX_INITIALIZER;
}

int MedianFilter::findMaxIndexRefrence(double *referenced, int *array, int worksize)
{
  int i = 0;
  int maxi = 0;
  for(i = 1; i < worksize; i++)
    if(array[i] > 0 && referenced[array[i]] > referenced[array[maxi]])
      maxi = i;
  return maxi;
}

double MedianFilter::getMedian()
{
  pthread_mutex_lock(&mutex);
  if(currentMedian < 0)
  {
    int medianNum = size / 2 + (size % 2);
    int i;
    int medianIndex = 0;
    
    for(int i = 0; i < size; i++) 
    {
      if(i < medianNum)
      {
        workspace[i] = i;
        if(data[i] > data[workspace[medianIndex]])
          medianIndex = i;
      }
      else
      {
        if(data[i] < data[workspace[medianIndex]])
        {
          workspace[medianIndex] = i;
          medianIndex = findMaxIndexRefrence(data, workspace, medianNum);
        }
      }
    }
    currentMedian = workspace[medianIndex];
  }

  double ret = data[currentMedian];
  pthread_mutex_unlock(&mutex);
  return ret;
}
  
void MedianFilter::pushValue(double val)
{
  pthread_mutex_lock(&mutex);
  currentMedian = -1;
  data[nextInsert] = val;
  nextInsert = (nextInsert+1)%size;
  pthread_mutex_unlock(&mutex);
}

