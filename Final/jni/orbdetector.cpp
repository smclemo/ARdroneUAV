/*
 * orbdetector.cpp
 *
 *  Created on: 14/08/2012
 *      Author: winston
 */

#include "orbdetector.h"

#include <algorithm>

int ImgRefCompare(const void* a, const void* b){
        const CVD::ImageRef* a1 = (CVD::ImageRef*)a;
        const CVD::ImageRef* b1 = (CVD::ImageRef*)b;
        if(a1->y != b1->y){
                        return a1->y - b1->y;
        } else {
                        return a1->x - b1->x;
        }
}

void OrbDetector::calculate_circle_patch(int const blockSize)
{
  harris_circular_patch.resize(blockSize);
  int const r = blockSize /2;
  float const r_sq = blockSize*blockSize/4.0f;
  for(int i=0; i<blockSize; i++)
  {
    harris_circular_patch[i] = floor(sqrt(r_sq - (i-r)*(i-r) ));
  }
  hcsz = harris_circular_patch.size();
}

//takes keypoints and culls them by the response
void OrbDetector::cull(std::vector<std::pair<CVD::ImageRef, int> > &keypoints, size_t const n_points)
{
  //this is only necessary if the keypoints size is greater than the number of desired points.
  if (keypoints.size() > n_points)
  {
    if (n_points==0) {
        keypoints.clear();
        return;
    }
    //first use nth element to partition the keypoints into the best and worst.
    std::nth_element(keypoints.begin(), keypoints.begin() + n_points, keypoints.end(), KeypointResponseGreater());
//    //this is the boundary response, and in the case of FAST may be ambigous
//    int ambiguous_response = keypoints[n_points - 1].second;
//    //use std::partition to grab all of the keypoints with the boundary response.
//    std::vector<std::pair<CVD::ImageRef, int> >::const_iterator new_end =
//    std::partition(keypoints.begin() + n_points, keypoints.end(),
//                   KeypointResponseGreaterThanThreshold(ambiguous_response));
//    //resize the keypoints, given this new end point. nth_element and partition reordered the points inplace
//    keypoints.resize(new_end - keypoints.begin());
    keypoints.resize(n_points);
  }
}


