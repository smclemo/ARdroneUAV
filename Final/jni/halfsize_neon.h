#ifndef HALFSIZE_NEON_H
#define HALFSIZE_NEON_H

#include "cvd_lite/image.h"

void halfsize(unsigned char* halfim, unsigned char* im, int width, int height);
CVD::Image<unsigned char> halfsize(const CVD::Image<unsigned char>& from);


CVD::Image<unsigned char> three_two_size(const CVD::Image<unsigned char>& from);


#endif
