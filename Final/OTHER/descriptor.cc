#include "descriptor.h"

#include <iomanip>
#include <cmath>

using namespace std;
using namespace CVD;
using namespace TooN;

void Descriptor::clear(){
  for(int i=0; i<Num_Q_Intensities; i++){
    descriptor[i]=0;
  }
}


void Descriptor::build_from_image(const BasicImage<unsigned char>& im, ImageRef pos){
  char patch[64];
  for(int i=0; i<64; i++){
    ImageRef offset = rhips[i];
    patch[i] = im[pos+offset];
  }

  double total=0;
  double total_sq=0;

  for(int i=0; i<64; i++){
    int val=patch[i];
    total += val;
    total_sq += val*val;
  }

  double mean = total/64;
  double sigma = sqrt(total_sq/64 - total*total/4096);


  // 4 levels have 25% in each bin if normally distributed
  double offset = 0.6755*sigma;

  for(int i=0; i<Num_Q_Intensities; i++){
    descriptor[i]=0;
  }

  unsigned long long int bit=1;
  for(int i=0; i<64; i++){
    int val= patch[i];
    if(val < mean - offset){
      descriptor[0] |= bit;
    } else if (val < mean ) {
      descriptor[1] |= bit;
    } else if (val < mean + offset) {
      descriptor[2] |= bit;
    } else {
      descriptor[3] |= bit;
    }
    
    bit <<=1;
  }
}



unsigned int Descriptor::error_of(const Descriptor& d) const {
  unsigned int result=0;
  for(int i=0; i<Num_Q_Intensities; i++){
    result += ::bitcount(d.descriptor[i] & ~ descriptor[i]);
  }
  return result;
}

unsigned int Descriptor::bitcount () const{
  unsigned int result=0;
  for(int i=0; i<Num_Q_Intensities; i++){
    result += ::bitcount(descriptor[i]);
  }
  return result;
}

int Descriptor::orientation(){
  Vector<2> total=Zeros;

  unsigned long long int bit=1;
  for(int i=0; i<64; i++){
    // obtain mean intensity from descriptor
    double sum=0;
    int count=0;

    for(int j=0; j<Num_Q_Intensities; j++){
      if(descriptor[j]&bit){
	sum+=j;
	count++;
      }
    }
    if(count==0){
      count=1;
    }
    sum/=count;

    // cout << i << endl;

    total+=ideal[i] * sum;
    bit*=2;
  }

  // now have an orientation vector in total
  const bool t1 = (total[0] <0);
  Vector<2> total1 =makeVector(t1?-total[0]:total[0],t1?-total[1]:total[1]);

  const bool t2 = (total1[1]<0);
  Vector<2> total2 = makeVector(t2?-total1[1]:total1[0],t2?total1[0]:total1[1]);
  
  const bool t3 = (total2[0] > total2[1]);
  Vector<2> total3 = makeVector(t3?total2[1]:total2[0],t3?total2[0]:total2[1]);
  
  const bool t4 = (2.4142*total3[0] > total3[1]);

  int rot = t1*8 + t2*4 + t3*2 + (t3^t4);
  
  return rot;
}



std::ostream& operator << (std::ostream& os, const Descriptor& d){
  for(int i=0; i<Descriptor::Num_Q_Intensities; i++){
    os << std::hex << std::setw(16) << setfill('0') << d.descriptor[i] << std::endl;
  }
  return os;
}


CVD::ImageRef Descriptor::rhips[64]={
  CVD::ImageRef(3,0),  CVD::ImageRef(6,-1), CVD::ImageRef(8,0),  CVD::ImageRef(9,-2),
  CVD::ImageRef(3,-1), CVD::ImageRef(5,-3), CVD::ImageRef(7,-3), CVD::ImageRef(8,-5),
  CVD::ImageRef(2,-2), CVD::ImageRef(3,-5), CVD::ImageRef(5,-5), CVD::ImageRef(5,-8),
  CVD::ImageRef(1,-3), CVD::ImageRef(1,-6), CVD::ImageRef(3,-7), CVD::ImageRef(2,-9),

  CVD::ImageRef(0,-3), CVD::ImageRef(-1,-6), CVD::ImageRef(0,-8), CVD::ImageRef(-2,-9),
  CVD::ImageRef(-1,-3), CVD::ImageRef(-3,-5), CVD::ImageRef(-3,-7), CVD::ImageRef(-5,-8),
  CVD::ImageRef(-2,-2), CVD::ImageRef(-5,-3), CVD::ImageRef(-5,-5), CVD::ImageRef(-8,-5),
  CVD::ImageRef(-3,-1), CVD::ImageRef(-6,-1), CVD::ImageRef(-7,-3), CVD::ImageRef(-9,-2),

  CVD::ImageRef(-3,0),  CVD::ImageRef(-6,1), CVD::ImageRef(-8,0),  CVD::ImageRef(-9,2),
  CVD::ImageRef(-3,1), CVD::ImageRef(-5,3), CVD::ImageRef(-7,3), CVD::ImageRef(-8,5),
  CVD::ImageRef(-2,2), CVD::ImageRef(-3,5), CVD::ImageRef(-5,5), CVD::ImageRef(-5,8),
  CVD::ImageRef(-1,3), CVD::ImageRef(-1,6), CVD::ImageRef(-3,7), CVD::ImageRef(-2,9),

  CVD::ImageRef(0,3), CVD::ImageRef(1,6), CVD::ImageRef(0,8), CVD::ImageRef(2,9),
  CVD::ImageRef(1,3), CVD::ImageRef(3,5), CVD::ImageRef(3,7), CVD::ImageRef(5,8),
  CVD::ImageRef(2,2), CVD::ImageRef(5,3), CVD::ImageRef(5,5), CVD::ImageRef(8,5),
  CVD::ImageRef(3,1), CVD::ImageRef(6,1), CVD::ImageRef(7,3), CVD::ImageRef(9,2)
};

Vector<2> Descriptor::ideal[64] = {
  makeVector(3, 0.003),
  makeVector(5.88588, -1.16466),
  makeVector(8, 0.008),
  makeVector(8.82882, -1.74698),
  makeVector(2.77049, 1.15082),
  makeVector(5.88354, 1.17643),
  makeVector(7.38797, 3.06886),
  makeVector(8.82531, 1.76464),
  makeVector(2.1192, 2.12344),
  makeVector(4.98548, 3.33841),
  makeVector(5.65119, 5.66251),
  makeVector(7.47822, 5.00761),
  makeVector(1.14528, 2.77279),
  makeVector(3.32843, 4.99215),
  makeVector(3.05407, 7.39409),
  makeVector(4.99265, 7.48822),
  makeVector(-0.003, 3),
  makeVector(1.16466, 5.88588),
  makeVector(-0.008, 8),
  makeVector(1.74698, 8.82882),
  makeVector(-1.15082, 2.77049),
  makeVector(-1.17643, 5.88354),
  makeVector(-3.06886, 7.38797),
  makeVector(-1.76464, 8.82531),
  makeVector(-2.12344, 2.1192),
  makeVector(-3.33841, 4.98548),
  makeVector(-5.66251, 5.65119),
  makeVector(-5.00761, 7.47822),
  makeVector(-2.77279, 1.14528),
  makeVector(-4.99215, 3.32843),
  makeVector(-7.39409, 3.05407),
  makeVector(-7.48822, 4.99265),
  makeVector(-3, -0.003),
  makeVector(-5.88588, 1.16466),
  makeVector(-8, -0.008),
  makeVector(-8.82882, 1.74698),
  makeVector(-2.77049, -1.15082),
  makeVector(-5.88354, -1.17643),
  makeVector(-7.38797, -3.06886),
  makeVector(-8.82531, -1.76464),
  makeVector(-2.1192, -2.12344),
  makeVector(-4.98548, -3.33841),
  makeVector(-5.65119, -5.66251),
  makeVector(-7.47822, -5.00761),
  makeVector(-1.14528, -2.77279),
  makeVector(-3.32843, -4.99215),
  makeVector(-3.05407, -7.39409),
  makeVector(-4.99265, -7.48822),
  makeVector(0.003, -3),
  makeVector(-1.16466, -5.88588),
  makeVector(0.008, -8),
  makeVector(-1.74698, -8.82882),
  makeVector(1.15082, -2.77049),
  makeVector(1.17643, -5.88354),
  makeVector(3.06886, -7.38797),
  makeVector(1.76464, -8.82531),
  makeVector(2.12344, -2.1192),
  makeVector(3.33841, -4.98548),
  makeVector(5.66251, -5.65119),
  makeVector(5.00761, -7.47822),
  makeVector(2.77279, -1.14528),
  makeVector(4.99215, -3.32843),
  makeVector(7.39409, -3.05407),
  makeVector(7.48822, -4.99265)
};
