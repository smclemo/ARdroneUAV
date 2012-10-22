#ifndef IMAGEPOINT_H
#define IMAGEPOINT_H

#include "cvd_lite/image.h"
#include "util/container.h"
#include "TooN/TooN.h"

#include "descriptor.h"

class ImagePoint {
 public:
  ImagePoint(){
    // children[0]=0;
    // children[1]=0;
    match_error=1000; // a number bigger than the maximum error in matching (currently 64)
  }

  void build_from_image(CVD::Image<unsigned char>& im, CVD::ImageRef& pos, CVD::Image<TooN::Vector<2, float> > const &xy_lookup, double scale);
  //void sample_from_image(const CVD::Image<unsigned char>& im, CVD::ImageRef pos, double scale);

  void rbuild_from_image(CVD::Image<unsigned char>& im, CVD::ImageRef& pos, CVD::Image<TooN::Vector<2, float> > const &xy_lookup, double scale);

  // obtains image coords and normalized cam coords from the image ref, camera and image scale
  void convert_pos_scale(const CVD::ImageRef& pos, CVD::Image<TooN::Vector<2, float> > const &xy_lookup, double scale);

  void serialise(char * serialised);

  Descriptor descriptor;
  // Descriptor pure_descriptor;
  TooN::Vector<2, float> cam_coords;
  //TooN::Vector<2> normalized_cam_coords;
  //double q; // 1/z in camera coordinates
  int match_error;

  // ImagePoint* children[2];
};


#endif
