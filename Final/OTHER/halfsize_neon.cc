#include <arm_neon.h>

// generate a half size image by averaging 2x2 blocks of pixels together
// resulting value is guaranteed to be correct to within +/- 0.5 but note:
// 1 2                        1 1
// 1 2  averages to 1         2 2  averages to 2
//

void halfsize(unsigned char* halfim, unsigned char* im, int width, int height)
{
	int ix, iy;
	for(iy=0; iy<height/2; iy++){
		for(ix=0; ix<width/16; ix++){
			uint8x16_t r1pixels = vld1q_u8((uint8_t*)im+(16*ix+2*iy*width)); // load 16 8-bit pixels from row 1
			uint8x16_t r2pixels = vld1q_u8((uint8_t*)im+(16*ix+2*iy*width+width)); // load 16 8-bit pixels from row 2

			uint8x16_t avpixels = vrhaddq_u8(r1pixels,r2pixels); // average the rows into a 8x16 (rounding up here)
			uint16x8_t averaged_16 = vpaddlq_u8(avpixels);        //average the columns into a 16x8
			uint8x8_t averaged = vshrn_n_u16(averaged_16,1);  // divide by two and narrow into 8x8 (rounding down here)

			vst1_u8((uint8_t*)(halfim+8*ix+(width/2)*iy),averaged); // store 8 8-bit values into destination image
		}
	}
}
