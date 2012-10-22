#include "imagepoint.h"
#include <string.h>

using namespace TooN;
using namespace CVD;
using namespace std;

void ImagePoint::build_from_image(CVD::Image<unsigned char>& im, CVD::ImageRef& pos, CVD::Image<TooN::Vector<2, float> > const &xy_lookup, double scale){
	descriptor.get_from_patch(pos,im);
	convert_pos_scale(pos,xy_lookup,scale);
}

void ImagePoint::rbuild_from_image(CVD::Image<unsigned char>& im, CVD::ImageRef& pos, CVD::Image<TooN::Vector<2, float> > const &xy_lookup, double scale){
	descriptor.rget_from_patch(pos,im);
	convert_pos_scale(pos,xy_lookup,scale);
}

/*
ImageRef OffsetList[8] = {
  ImageRef(0,-1),
  ImageRef(-1,0),
  ImageRef(1,0),
  ImageRef(0,1),
  ImageRef(-1,-1),
  ImageRef(1,-1),
  ImageRef(-1,1),
  ImageRef(1,1)
};

void ImagePoint::sample_from_image(const CVD::Image<unsigned char>& im, CVD::ImageRef pos, double scale){
  //pure_descriptor.build_from_image(im, pos);
  descriptor.build_from_image(im, pos);
  for(int i=0; i<8; i++){ // at the moment only blur with 4 compass directions
    Descriptor d;
    d.build_from_image(im,pos+OffsetList[i]);
    descriptor |= d;
    //cout << "i " << i << " descrip" << descriptor.bitcount() << endl;
  }
  convert_pos_scale(pos,cam,scale);
}
*/
void ImagePoint::convert_pos_scale(const CVD::ImageRef& pos, CVD::Image<TooN::Vector<2, float> > const &xy_lookup, double scale){
	Vector<2> scale_offset = makeVector(scale/2,scale/2);
	Vector<2> scaled_pos = makeVector(pos[0]*scale, pos[1]*scale);
	TooN::Vector<2> image_coords = scaled_pos + scale_offset;
	//cam_coords = image_coords;
	ImageRef distorted_coords_ref = ImageRef(image_coords[0], image_coords[1]);
	cam_coords = xy_lookup[distorted_coords_ref];
	
	//ImageRef undistorted_coords_ref = ImageRef((int) undistorted_coords[0], (int) undistorted_coords[1]);
	//if(undistortedImage.in_image(undistorted_coords_ref))
		//undistortedImage[undistorted_coords_ref] = distortedImage[distorted_coords_ref];
}

void ImagePoint::serialise(char * serialised)
{
	//__android_log_print(ANDROID_LOG_DEBUG, "ImagePoint" , "TooN: %f", cam_coords[0]);
//	memcpy(serialised, &(cam_coords[0]), 4);
//	memcpy(serialised+4, &(cam_coords[1]), 4);
    memcpy(serialised, &(cam_coords[0]), 8);
}

