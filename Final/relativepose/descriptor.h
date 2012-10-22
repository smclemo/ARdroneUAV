// -*- c++ -*-

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <cvd/image.h>
#include <TooN/TooN.h>

#include <iostream>


#include "bitcount.h"


class Descriptor{
public:
  Descriptor(){}
  Descriptor(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos){
    build_from_image(im,pos);
  }
  void clear();
  void build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos);

  unsigned int error_of(const Descriptor& d) const; // how many bits are set in d that are not set in me
  unsigned int bitcount() const; // how many bits are set in me

  // rotates the descriptor clockwise by angle in a conventional left handed image coordinate frame
  // (x to the right and y down)
  Descriptor rotate(const int angle){
    const int a=4*(angle&15);
    Descriptor result;
    for(int i=0; i<Num_Q_Intensities; i++){
      result.descriptor[i] = (descriptor[i] >> a) + (descriptor[i] << (64-a));
    }
    return result;
  }

  // compute canonical rotation clockwise from canonical position
  // descriptor.rotation(1).orientation() == (descriptor.orientation() + 1)%16
  int orientation();


  Descriptor operator|(const Descriptor& d) const{
    Descriptor result;
    for(int i=0; i<Num_Q_Intensities; i++){
      result.descriptor[i] = descriptor[i] | d.descriptor[i];
    }
    return result;
  }

  Descriptor& operator|=(const Descriptor& d) {
    for(int i=0; i<Num_Q_Intensities; i++){
      descriptor[i] |= d.descriptor[i];
    }
    return *this;
  }

  Descriptor operator&(const Descriptor& d) const{
    Descriptor result;
    for(int i=0; i<Num_Q_Intensities; i++){
      result.descriptor[i] = descriptor[i] & d.descriptor[i];
    }
    return result;
  }

  Descriptor& operator&=(const Descriptor& d) {
    for(int i=0; i<Num_Q_Intensities; i++){
      descriptor[i] &= d.descriptor[i];
    }
    return *this;
  }


  static const int Num_Q_Intensities=4; // 5 is optimal - but 4 is nearly as good and much easier on many platforms
  unsigned long long int descriptor[Num_Q_Intensities];

  static CVD::ImageRef rhips[64];
  static TooN::Vector<2> ideal[64];
};

std::ostream& operator << (std::ostream& os, const Descriptor& d);



#endif
