#ifndef POINTSET_H
#define POINTSET_H

#include "cvd_lite/image.h"
#include "util/container.h"
#include "orbdetector.h"
#include "imagepoint.h"

using namespace CVD;
using namespace std;



class PointSet {
public:
  PointSet():od(7)
  {
    fast_barrier = 40;
    database.set_max_size(1024);
  }

  void build_from_image(CVD::Image<unsigned char>& im, Image<TooN::Vector<2, float> >& xy_lookup, bool const use_rhips);
  void add_layer(CVD::Image<unsigned char>& im, Image<TooN::Vector<2, float> >& xy_lookup, double const scale, bool const use_rhips);

  ImagePoint& operator[](int i);
  ImagePoint* get_corner(int i){return &(database[i]);}

  int size(){return database.size();}

  void erase(){database.clear();}

  void add(ImagePoint& ip){
    database.add(ip);
  }

private:
  Container<ImagePoint> database;
  OrbDetector od;
  int fast_barrier;
  static const int nCornersPerLayer = 100;
};



#endif
