// -*- c++ -*-

#ifndef UVQ_H
#define UVQ_H

#include <TooN/TooN.h>
#include <TooN/se3.h>

TooN::Vector<3> operator*(const TooN::SE3<>& pose, const TooN::Vector<3>& rhs);

#endif
