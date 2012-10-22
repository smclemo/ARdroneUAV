/*
 * orbdetector.h
 * OpenCV orb adapted for libcvd
 *  Created on: 01/04/2012
 *      Author: winston
 */

/** OpenCV ORB Authors: Ethan Rublee, Vincent Rabaud, Gary Bradski */

/*********************************************************************
* Software License Agreement (BSD License)
*
* Copyright (c) 2009, Willow Garage, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided
* with the distribution.
* * Neither the name of the Willow Garage nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/


#ifndef ORBDETECTOR_H_
#define ORBDETECTOR_H_

#include "cvd_lite/image.h"
#include "util/container.h"
#include "Tom_fast9.h"
#include "nonmax.h"
#include "TooN/TooN.h"
#include <cmath>
#include <utility>
#include <algorithm>
#include <vector>


int ImgRefCompare(const void* a, const void* b);

class OrbDetector{
public:
  OrbDetector(int patchDiameter = 7)
  :spatialbinssz(800/binsz, 480/binsz),
    spatialbins(spatialbinssz)
  {
    calculate_circle_patch(patchDiameter);
    fast_corners.set_max_size(2048);
    harris_scores.set_max_size(2048);
    max_corners.reserve(2048);

    for(int y=0; y<spatialbinssz.y; y++)
    {
      for(int x=0; x<spatialbinssz.x; x++)
      {
        spatialbins[y][x].reserve(2000/binsz);
      }
    }
  }
  /*
   * @ param circle_patch contains the horizontal offsets for a circle
   * blockSize must be odd!!
   */
  template <class T>
  void harrisResponses(CVD::Image<T> const &image, Container<CVD::ImageRef> const &corners,
      Container<int> &scores);

  template<class T>
  int findShiTomasiScoreAtPoint(CVD::Image<T> const &image,
                                   CVD::ImageRef const &irCenter);

  void calculate_circle_patch(int const blockSize);
  void cull(std::vector<std::pair<CVD::ImageRef, int> > &keypoints, size_t const n_points);

  template <class T>
  void detector(CVD::Image<T> const &image, int &fast_barrier, Container<CVD::ImageRef> &corners, size_t const n_points);

  template <class T>
  void detectDistributed(CVD::Image<T> const &image, int &fast_barrier, Container<CVD::ImageRef> &corners, size_t const n_pointspbin);

  std::vector<int> harris_circular_patch; //circular patch. +/- Horizontal offsets for each vertical offset from center
  int hcsz;

  Container<CVD::ImageRef> fast_corners;
  Container<int> harris_scores;
  std::vector<std::pair<CVD::ImageRef, int> > max_corners;
  static const int binsz = 20;
  CVD::ImageRef const spatialbinssz;
  CVD::Image<std::vector<std::pair<CVD::ImageRef, int> > > spatialbins;
};


struct KeypointResponseGreaterThanThreshold
{
  KeypointResponseGreaterThanThreshold(int _value) :
  value(_value)
  {
  }
  inline bool operator()(std::pair<CVD::ImageRef, int> const &kpt) const
  {
    return kpt.second >= value;
  }
  int value;
};

struct KeypointResponseGreater
{
  inline bool operator()(std::pair<CVD::ImageRef, int> const &kp1, std::pair<CVD::ImageRef, int> const &kp2) const
  {
    return kp1.second > kp2.second;
  }
};


/**
* Function that computes the Harris responses in a
* blockSize x blockSize patch at given points in an image
*
* Uses a hybrid of ptam's implementation of shi-tomasi and opencv's circular patch. No sigmaI and sigmaD gaussian weighting
*/
template <class T>
void OrbDetector::harrisResponses(CVD::Image<T> const &image, Container<CVD::ImageRef> const &corners,
    Container<int> &scores)
{
  int const corners_sz = corners.size();
  scores.set_size(corners_sz);
  for(size_t i =0; i < corners_sz; i++)
  {
    scores[i] = findShiTomasiScoreAtPoint(image, corners[i]);
  }
}

/**
* circle_patch size must be odd!!!
*/
template<class T>
int OrbDetector::findShiTomasiScoreAtPoint(CVD::Image<T> const &image,
                                 CVD::ImageRef const &irCenter)
{
  int dXX = 0, dYY = 0, dXY = 0;

  int nPixels = 0;
  CVD::ImageRef ir;
  ir.y = irCenter.y - hcsz/2;

  for(int i = 0; i < hcsz; i++)
  {
    for(ir.x = irCenter.x - harris_circular_patch[i]; ir.x<=irCenter.x + harris_circular_patch[i]; ir.x++)
    {
      int const dx = image[ir + CVD::ImageRef(1,0)] - image[ir - CVD::ImageRef(1,0)];
      int const dy = image[ir + CVD::ImageRef(0,1)] - image[ir - CVD::ImageRef(0,1)];
      dXX += dx*dx;
      dYY += dy*dy;
      dXY += dx*dy;
      nPixels ++;
    }
    ir.y++;
  }
  dXX = dXX / (2 * nPixels);
  dYY = dYY / (2 * nPixels);
  dXY = dXY / (2 * nPixels);

  // Find and return smaller eigenvalue:
  return (dXX + dYY - sqrt( (float) (dXX + dYY) * (dXX + dYY) - 4 * (dXX * dYY - dXY * dXY) ))/2;
}


template <class T>
void OrbDetector::detector(CVD::Image<T> const &image, int &fast_barrier, Container<CVD::ImageRef> &corners, size_t const n_points)
{
  fast_corners.clear();
  harris_scores.clear();
  max_corners.clear();
//  corners.clear();

  int const border = std::max(int(this->harris_circular_patch.size()/2), 18);
  Tom_fast9_detect(image, fast_barrier, border, fast_corners);
  if(fast_corners.size()>0)
  {
    qsort(&(fast_corners[0]), fast_corners.size(), sizeof(CVD::ImageRef), ImgRefCompare);
  }
   // __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Fast corners %d", fast_corners.size());
  if(fast_corners.size() > n_points + 200)
  {
    fast_barrier++;
  }
  else if(fast_barrier>1)
  {
    fast_barrier--;
  }

  harrisResponses(image, fast_corners, harris_scores);
  nonmax_suppression_with_scores(fast_corners, harris_scores, max_corners);
  cull(max_corners, n_points);
  size_t const max_sz = max_corners.size();
//  corners.resize(max_sz);
//  for(size_t i=0; i<max_sz; i++)
//  {
//    corners[i] = max_corners[i].first;
//  }
  //Concatenates to previous corners
  corners.set_size(max_sz);
  for(size_t i=0; i<max_sz; i++)
  {
    corners[i] = max_corners[i].first;
  }
 // __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Orb corners %d", corners.size());
}

template <class T>
void OrbDetector::detectDistributed(CVD::Image<T> const &image, int &fast_barrier, Container<CVD::ImageRef> &corners, size_t const n_pointspbin)
{
  fast_corners.clear();
  harris_scores.clear();
  max_corners.clear();

  for(int y=0; y<spatialbinssz.y; y++)
  {
    for(int x=0; x<spatialbinssz.x; x++)
    {
      spatialbins[y][x].clear();
    }
  }

  int const border = std::max(int(this->harris_circular_patch.size()/2), 18);
  Tom_fast9_detect(image, fast_barrier, border, fast_corners);
  if(fast_corners.size()>0)
  {
    qsort(&(fast_corners[0]), fast_corners.size(), sizeof(CVD::ImageRef), ImgRefCompare);
  }
 // __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Fast corners %d", fast_corners.size());
  if(fast_corners.size() > 600)
  {
    fast_barrier++;
  }
  else if(fast_barrier>1)
  {
    fast_barrier--;
  }

  harrisResponses(image, fast_corners, harris_scores);
  nonmax_suppression_with_scores(fast_corners, harris_scores, max_corners);

  //Place corners into bins
  int const mcsz = max_corners.size();
  corners.clear();
  corners.set_max_size(mcsz);
  for(int i =0; i<mcsz; i++)
  {
//    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Binning %d %d", (max_corners[i].first/binsz).x, (max_corners[i].first/binsz).y);
    spatialbins[max_corners[i].first/binsz].push_back(max_corners[i]);
//    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Max %d %d", spatialbins[max_corners[i].first/binsz].back().first.x, spatialbins[max_corners[i].first/binsz].back().first.y);
  }

  for(int y=0; y<spatialbinssz.y; y++)
  {
    for(int x=0; x<spatialbinssz.x; x++)
    {
      cull(spatialbins[y][x], n_pointspbin);
      int const spatialbin_sz = spatialbins[y][x].size();
      for(int i=0; i<spatialbin_sz; i++)
      {
        corners.add(spatialbins[y][x][i].first);
      }
    }
  }
  //__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Orb corners %d", corners.size());
}


#endif /* ORBDETECTOR_H_ */
