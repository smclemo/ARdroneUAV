/*
 * uvq.cc
 *
 *  Created on: 06/10/2011
 *      Author: winston
 */

#include "uvq.h"

TooN::Vector<3> operator*(const TooN::SE3<>& pose, const TooN::Vector<3>& rhs){
  TooN::Vector<3> result;

  const TooN::Matrix<3>& R = pose.get_rotation().get_matrix();
  const TooN::Vector<3>& t = pose.get_translation();

  double qfac = 1.0/(R(2,0)*rhs[0] + R(2,1)*rhs[1] + R(2,2) + t[2]*rhs[2]);

  result[0] = (R(0,0)*rhs[0] + R(0,1)*rhs[1] + R(0,2) + t[0]*rhs[2]) * qfac;
  result[1] = (R(1,0)*rhs[0] + R(1,1)*rhs[1] + R(1,2) + t[1]*rhs[2]) * qfac;
  result[2] = rhs[2] * qfac;

  return result;
}
