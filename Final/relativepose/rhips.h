// -*- c++ -*-

#ifndef RHIPS_H
#define RHIPS_H

#define CVD_DEBUG 1

#include <TooN/TooN.h>
#include <cvd/image.h>
#include <vector>

#include "descriptor.h"
#include "pointsetindex.h"



// adds the points extracted from im to the vector of descriptors and the vector of points
void add_image(const CVD::BasicImage<unsigned char>& im, int fast_barrier, bool sample, std::vector<Descriptor>& descriptors, std::vector<CVD::ImageRef>& points);

// adds images at scales 1, 1.5, 2, 3 and 4
void add_image_pyramid(const CVD::BasicImage<unsigned char>& im, int fast_barrier, bool sample,
		       std::vector<Descriptor>& descriptors, std::vector<CVD::ImageRef>& points, std::vector<double>& scales);

// used by add_image_pyramid after the add_image function to record the scale information corresponding to each pyramid layer
void pad_scales(std::vector<CVD::ImageRef>& points, std::vector<double>& scales, double scale);

// combines image position and scale information to obtain image coordinates with respect to the base image of the pyramid
void compute_normalized_image_coords (const std::vector<CVD::ImageRef>& points, const std::vector<double>& scales, std::vector<TooN::Vector<2> >& normalized_image_coords);

// uses a camera model to obtain normalized camera coordinates from the image coordinates
template<class Camera>
void compute_normalized_camera_coords(const std::vector<TooN::Vector<2> >& normalized_image_coords, const Camera& cam, std::vector<TooN::Vector<2> >& normalized_camera_coords);

// returns the amount that each descriptor is rotated from its canonical orientation
// to obtain canonical orientation it needs to be rotated by the negative of this amount
void compute_orientation(std::vector<Descriptor>& descriptors, std::vector<int>& orientations);

// rotates each descriptor into canonical orientation
void rotation_normalize(std::vector<Descriptor>& descriptors, std::vector<int>& orientations, std::vector<Descriptor>& normalized);

// returns the amount that the unnormalized test descriptors need to be rotated to match the unnormalized reference descriptors
int global_orientation(std::vector<Descriptor>& test_norm, std::vector<int>& test_orient, PointSetIndex& psi, std::vector<int>& ref_orient, int match_threshold);

// rotate all descriptors by rotation
void rotate(std::vector<Descriptor>& descriptors, int rotation, std::vector<Descriptor>& rotated);



///// templated implementations follow //////


// templated function has to appear in header
template<class Camera>
void compute_normalized_camera_coords(const std::vector<TooN::Vector<2> >& normalized_image_coords, const Camera& cam, std::vector<TooN::Vector<2> >& normalized_camera_coords){
  const int sz=normalized_image_coords.size();
  normalized_camera_coords.resize(sz);
  for(int i=0; i<sz; i++){
    normalized_camera_coords[i] = cam.unproject(normalized_image_coords[i]);
  }
}




#endif
