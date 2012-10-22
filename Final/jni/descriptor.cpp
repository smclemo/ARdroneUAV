#include "descriptor.h"

//CVD::ImageRef Descriptor::rhips[64]={
//  CVD::ImageRef(3,0),  CVD::ImageRef(6,-1), CVD::ImageRef(8,0),  CVD::ImageRef(9,-2),
//  CVD::ImageRef(3,-1), CVD::ImageRef(5,-3), CVD::ImageRef(7,-3), CVD::ImageRef(8,-5),
//  CVD::ImageRef(2,-2), CVD::ImageRef(3,-5), CVD::ImageRef(5,-5), CVD::ImageRef(5,-8),
//  CVD::ImageRef(1,-3), CVD::ImageRef(1,-6), CVD::ImageRef(3,-7), CVD::ImageRef(2,-9),
//
//  CVD::ImageRef(0,-3), CVD::ImageRef(-1,-6), CVD::ImageRef(0,-8), CVD::ImageRef(-2,-9),
//  CVD::ImageRef(-1,-3), CVD::ImageRef(-3,-5), CVD::ImageRef(-3,-7), CVD::ImageRef(-5,-8),
//  CVD::ImageRef(-2,-2), CVD::ImageRef(-5,-3), CVD::ImageRef(-5,-5), CVD::ImageRef(-8,-5),
//  CVD::ImageRef(-3,-1), CVD::ImageRef(-6,-1), CVD::ImageRef(-7,-3), CVD::ImageRef(-9,-2),
//
//  CVD::ImageRef(-3,0),  CVD::ImageRef(-6,1), CVD::ImageRef(-8,0),  CVD::ImageRef(-9,2),
//  CVD::ImageRef(-3,1), CVD::ImageRef(-5,3), CVD::ImageRef(-7,3), CVD::ImageRef(-8,5),
//  CVD::ImageRef(-2,2), CVD::ImageRef(-3,5), CVD::ImageRef(-5,5), CVD::ImageRef(-5,8),
//  CVD::ImageRef(-1,3), CVD::ImageRef(-1,6), CVD::ImageRef(-3,7), CVD::ImageRef(-2,9),
//
//  CVD::ImageRef(0,3), CVD::ImageRef(1,6), CVD::ImageRef(0,8), CVD::ImageRef(2,9),
//  CVD::ImageRef(1,3), CVD::ImageRef(3,5), CVD::ImageRef(3,7), CVD::ImageRef(5,8),
//  CVD::ImageRef(2,2), CVD::ImageRef(5,3), CVD::ImageRef(5,5), CVD::ImageRef(8,5),
//  CVD::ImageRef(3,1), CVD::ImageRef(6,1), CVD::ImageRef(7,3), CVD::ImageRef(9,2)
//};

CVD::ImageRef Descriptor::rhips[64]={
  CVD::ImageRef(3,0),
  CVD::ImageRef(2,-2),
  CVD::ImageRef(0,-3),
  CVD::ImageRef(-2,-2),
  CVD::ImageRef(-3,0),
  CVD::ImageRef(-2,2),
  CVD::ImageRef(0,3),
  CVD::ImageRef(2,2),

  CVD::ImageRef(6,-1),
  CVD::ImageRef(3,-5),
  CVD::ImageRef(-1,-6),
  CVD::ImageRef(-5,-3),
  CVD::ImageRef(-6,1),
  CVD::ImageRef(-3,5),
  CVD::ImageRef(1,6),
  CVD::ImageRef(5,3),

  CVD::ImageRef(8,0),
  CVD::ImageRef(5,-5),
  CVD::ImageRef(0,-8),
  CVD::ImageRef(-5,-5),
  CVD::ImageRef(-8,0),
  CVD::ImageRef(-5,5),
  CVD::ImageRef(0,8),
  CVD::ImageRef(5,5),

  CVD::ImageRef(9,-2),
  CVD::ImageRef(5,-8),
  CVD::ImageRef(-2,-9),
  CVD::ImageRef(-8,-5),
  CVD::ImageRef(-9,2),
  CVD::ImageRef(-5,8),
  CVD::ImageRef(2,9),
  CVD::ImageRef(8,5),

  CVD::ImageRef(3,-1),
  CVD::ImageRef(1,-3),
  CVD::ImageRef(-1,-3),
  CVD::ImageRef(-3,-1),
  CVD::ImageRef(-3,1),
  CVD::ImageRef(-1,3),
  CVD::ImageRef(1,3),
  CVD::ImageRef(3,1),

  CVD::ImageRef(5,-3),
  CVD::ImageRef(1,-6),
  CVD::ImageRef(-3,-5),
  CVD::ImageRef(-6,-1),
  CVD::ImageRef(-5,3),
  CVD::ImageRef(-1,6),
  CVD::ImageRef(3,5),
  CVD::ImageRef(6,1),

  CVD::ImageRef(7,-3),
  CVD::ImageRef(3,-7),
  CVD::ImageRef(-3,-7),
  CVD::ImageRef(-7,-3),
  CVD::ImageRef(-7,3),
  CVD::ImageRef(-3,7),
  CVD::ImageRef(3,7),
  CVD::ImageRef(7,3),

  CVD::ImageRef(8,-5),
  CVD::ImageRef(2,-9),
  CVD::ImageRef(-5,-8),
  CVD::ImageRef(-9,-2),
  CVD::ImageRef(-8,5),
  CVD::ImageRef(-2,9),
  CVD::ImageRef(5,8),
  CVD::ImageRef(9,2)

};
