#ifndef NATIVE_H
#define NATIVE_H

#include "cvd_lite/image.h"

struct pose {
	float x;
	float y;
	float z;
	float yaw;
//	float pitch;
//	float roll;
};

void initnative();

void uninitnative();

bool get_pose(pose* location);

void undistort(CVD::Image<unsigned char>& im);

void saveKeypoints(CVD::Image<unsigned char>& imageToSave);

#endif
