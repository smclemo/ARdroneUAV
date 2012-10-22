/*
 * camera_coord_lookup.cpp
 *
 *  Created on: 27/08/2012
 *      Author: winston
 */

#include "camera_coord_lookup.h"

void generate_xy_lookup(Camera::Quintic &cam_model, CVD::Image<TooN::Vector<2, float> >& xy_lookup)
{
	int const ysz = 720;//xy_lookup.size().y;
	int const xsz = 1280;//xy_lookup.size().x;
	
	//printf("xsz=%i, ysz=%i\n", xsz, ysz);
	
	//TooN::Vector<6> cam_params = cam_model.get_parameters();
	
	for(int y=0; y<ysz; y++)
	{
		for(int x=0; x<xsz; x++)
		{
			TooN::Vector<2, float> distorted_coords = TooN::makeVector((float) x, (float) y);
			TooN::Vector<2, float> undistorted_coords = cam_model.unproject(distorted_coords);
			
			//undistorted_coords[0] += 640;//cam_params[2];
			//undistorted_coords[1] += 360;//cam_params[3];
			
			//undistorted_coords[0] = (int) (undistorted_coords[0]+0.5);
			//undistorted_coords[1] = (int) (undistorted_coords[1]+0.5);
			
			CVD::ImageRef distorted_coords_ref = CVD::ImageRef(x, y);
			//CVD::ImageRef undistorted_coords_ref = CVD::ImageRef((int) undistorted_coords[0], (int) undistorted_coords[1]);
			
			//printf("in: x=%f, y=%f out: x=%f, y=%f\n", distorted_coords[0], distorted_coords[1], undistorted_coords[0], undistorted_coords[1]);
			//if(xy_lookup.in_image(undistorted_coords_ref))
			//{
				xy_lookup[distorted_coords_ref] = undistorted_coords;
			//}
		}
	}
}
