/*
 * camera_coord_lookup.h
 *
 *  Created on: 27/08/2012
 *      Author: winston
 */

#ifndef CAMERA_COORD_LOOKUP_H_
#define CAMERA_COORD_LOOKUP_H_

#include "cvd_lite/image.h"
#include "cvd_lite/camera.h"
#include "cvd_lite/image_ref.h"
#include "TooN/TooN.h"

void generate_xy_lookup(Camera::Quintic &cam_model, CVD::Image<TooN::Vector<2, float> >& xy_lookup);

#endif /* CAMERA_COORD_LOOKUP_H_ */
