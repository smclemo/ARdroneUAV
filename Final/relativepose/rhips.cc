#include "rhips.h"

#include <cvd/fast_corner.h>
#include <vector>
#include <cvd/vision.h>
#include <cvd/vector_image_ref.h>
#include <list>
#include <map>

#include "pointsetindex.h"

using namespace std;
using namespace CVD;

ImageRef OffsetList[8] = {
  ImageRef(-1,-1),
  ImageRef(0,-1),
  ImageRef(1,-1),
  ImageRef(-1,0),
  ImageRef(1,0),
  ImageRef(-1,1),
  ImageRef(0,1),
  ImageRef(1,1)
};

void add_image(const CVD::BasicImage<unsigned char>& im, int fast_barrier, bool sample,  // input arguments
	       std::vector<Descriptor>& descriptors, std::vector<ImageRef>& points){    // output arguments (arrays are added to)
  vector<ImageRef> corners;
  fast_corner_detect_9(im,corners, fast_barrier);
  vector<ImageRef> max_corners;
  fast_nonmax(im,corners, fast_barrier, max_corners);
  for(int i=0; i<max_corners.size(); i++){
    if(im.in_image_with_border(max_corners[i],10)){ // 10 pixels from edge needed (9 for patch circle and 1 for offset for sampling)
      const ImageRef pos(max_corners[i]);
      points.push_back(pos);
      Descriptor d(im,pos);
      if(sample){
	for(int o=0; o<8; o++){
	  d|=Descriptor(im,pos+OffsetList[o]);
	}
      }
      descriptors.push_back(d);
    }
  }
}

void add_image_pyramid(const CVD::BasicImage<unsigned char>& im, int fast_barrier, bool sample,
		       std::vector<Descriptor>& descriptors, std::vector<CVD::ImageRef>& points, std::vector<double>& scales){
  // initially ensure that points and scales match descriptors
  const int sz = descriptors.size();
  points.resize(sz);
  scales.resize(sz);

  // powers of two scale (1, 2, 4)
  add_image(im, fast_barrier, sample, descriptors, points);
  pad_scales(points,scales,1.0);

  Image<unsigned char> im2 = halfSample(im); // must use a temporary because of lazy eval ?? really?
  add_image(im2, fast_barrier, sample, descriptors, points);
  pad_scales(points,scales,2.0);

  Image<unsigned char> im4 = halfSample(im2); // must use a temporary because of lazy eval ?? really?
  add_image(im4, fast_barrier, sample, descriptors, points);
  pad_scales(points,scales,4.0);

//  // intermediate 1.5 * powers of two scale (1.5, 3, 6, ...)
//  Image<unsigned char>im1_5 = twoThirdsSample(im);
//  add_image(im1_5, fast_barrier, sample, descriptors, points);
//  pad_scales(points,scales,1.5);
//
//  Image<unsigned char>im3 = halfSample(im1_5);
//  add_image(im3, fast_barrier, sample, descriptors, points);
//  pad_scales(points,scales,3.0);
}


void pad_scales(std::vector<CVD::ImageRef>& points, std::vector<double>& scales, double scale) {
  int from = scales.size();
  int to = points.size();
  scales.resize(to);
  for(int i=from; i<to; i++){
    scales[i]=scale;
  }
}

void compute_normalized_image_coords (const std::vector<CVD::ImageRef>& points, const std::vector<double>& scales, std::vector<TooN::Vector<2> >& normalized_image_coords){
  const int sz = points.size();
  normalized_image_coords.resize(sz);
  for(int i=0; i<sz; i++){
    double scale = scales[i];
    CVD::ImageRef pos = points[i];
    TooN::Vector<2> scale_offset = TooN::makeVector(scale/2,scale/2);
    normalized_image_coords[i] = vec(pos)*scale + scale_offset;
  }
}

void compute_orientation(std::vector<Descriptor>& descriptors, std::vector<int>& orientations){
  const int sz = descriptors.size();
  orientations.resize(sz);
  for(int i=0; i<sz; i++){
    orientations[i]=descriptors[i].orientation();
  }
}

void rotation_normalize(std::vector<Descriptor>& descriptors, std::vector<int>& orientations, std::vector<Descriptor>& normalized){
  const int sz = descriptors.size();
  normalized.resize(sz);
  for(int i=0; i<sz; i++){
    normalized[i] = descriptors[i].rotate(-orientations[i]);
  }
}

int global_orientation(std::vector<Descriptor>& test_norm, std::vector<int>& test_orient, PointSetIndex& psi, std::vector<int>& ref_orient, int match_threshold){
  int hist[16]={0};
  const int sz = test_norm.size();
  for(int i=0; i<sz; i++){
    pair<int, int> match = psi.best_match(test_norm[i], match_threshold);
    if(match.second!=-1){
      const int relative_orientation = (ref_orient[match.second]-test_orient[i])&15;
      hist[relative_orientation]++;
    }
  }

  int best=0;
  int score=hist[0];
  for(int i=1; i<16; i++){
    if(hist[i] > score){
      score = hist[i];
      best=i;
    }
  }
  return best;
}

void rotate(std::vector<Descriptor>& descriptors, int rotation, std::vector<Descriptor>& rotated){
  int sz = descriptors.size();
  rotated.resize(sz);
  for(int i=0; i<sz; i++){
    rotated[i] = descriptors[i].rotate(rotation);
  }
}
