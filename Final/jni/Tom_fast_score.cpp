#include "Tom_fast_score.h"


using namespace CVD;

static void make_offsets(int pixel[], int row_stride)
{
        pixel[0] = 0 + row_stride * 3;
        pixel[1] = 1 + row_stride * 3;
        pixel[2] = 2 + row_stride * 2;
        pixel[3] = 3 + row_stride * 1;
        pixel[4] = 3 + row_stride * 0;
        pixel[5] = 3 + row_stride * -1;
        pixel[6] = 2 + row_stride * -2;
        pixel[7] = 1 + row_stride * -3;
        pixel[8] = 0 + row_stride * -3;
        pixel[9] = -1 + row_stride * -3;
        pixel[10] = -2 + row_stride * -2;
        pixel[11] = -3 + row_stride * -1;
        pixel[12] = -3 + row_stride * 0;
        pixel[13] = -3 + row_stride * 1;
        pixel[14] = -2 + row_stride * 2;
        pixel[15] = -1 + row_stride * 3;
}



void compute_scores(const Image<unsigned char>& im, const Container<ImageRef>& corners, int barrier, Container<int>& scores){
	float transitions[33];
	int pixel[17];
	make_offsets(pixel, im.size().x);
	pixel[16]=pixel[0];

	scores.set_size(corners.size());

	for(int c=0; c<corners.size(); c++){

		const ImageRef cpos = corners[c];
		const int cval = im[cpos];
		const unsigned char* cptr = &(im[cpos]);
		const int c_plus = cval+barrier;
		const int c_minus = cval-barrier;

		// get the transitions
		int t=0;
		for(int p=0; p<16; p++){
			const int ring1 = cptr[pixel[p]];
			const int ring2 = cptr[pixel[p+1]];

			if((ring1 < c_minus) ^ (ring2 < c_minus)) {
				float fraction = ((float)(c_minus-ring1))/(ring2-ring1);
				transitions[t++] = p+fraction;
			}
			if((ring1 > c_plus) ^ (ring2 > c_plus)) {
				float fraction = ((float)(c_plus-ring1))/(ring2-ring1);
				transitions[t++] = p+fraction;
			}
		}

		transitions[t]=transitions[0]+16;
		float max_band=0;
		for(int i=0; i<t; i++){
			float band = transitions[i+1]-transitions[i];
			if(band>max_band){
				max_band=band;
			}
		}
		if(t==0){
			max_band=16;
		}

		scores[c] = 1000*max_band;
	}
}
