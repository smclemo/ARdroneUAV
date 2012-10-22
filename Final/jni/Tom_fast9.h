#ifndef TOM_FAST9_H
#define TOM_FAST9_H

#include "fast.h"

#include "cvd_lite/image.h"

#include "util/container.h"

void Tom_fast9_detect(const CVD::Image<unsigned char>& im, int barrier, int border, Container<CVD::ImageRef>& container);

#endif
