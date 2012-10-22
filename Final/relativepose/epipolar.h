// -*- c++ -*-

#ifndef EPIPOLAR_H
#define EPIPOLAR_H

#include <TooN/TooN.h>
#include <TooN/se3.h>
#include <vector>
#include "ransac.h"
#include <iostream>

class Epipolar {
 public:
  static const int sample_size=5;
  TooN::Vector<2> *im1; //Normalised camera coordinates
  TooN::Vector<2> *im2;
};

template<>
class Hypothesis<Epipolar> {
 public:
  void generate(const std::vector<Ransac<Epipolar>* >& gen_set);
  double is_inlier(const Ransac<Epipolar>& test, double threshold);

  void save(std::ostream& os);
  void load(std::istream& is);

  TooN::SO3<> Rt; // rotation from cam1 to cam2
  TooN::SO3<> Rn; // rotation from X -> translation direction from 1 to 2
};





#endif
