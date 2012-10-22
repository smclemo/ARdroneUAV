#ifndef TOM_FAST_SCORE_H
#define TOM_FAST_SCORE_H

#include "cvd_lite/image.h"
#include "util/container.h"

void compute_scores(const CVD::Image<unsigned char>& im, const Container<CVD::ImageRef>& corners, int barrier, Container<int>& scores);







#endif
