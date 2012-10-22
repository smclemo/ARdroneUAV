
#include "halfsize_neon.h"

#include <arm_neon.h>

#include "cvd_lite/image.h"

using namespace CVD;

// generate a half size image by averaging 2x2 blocks of pixels together
// resulting value is guaranteed to be correct to within +/- 0.5 but note:
// 1 2                        1 1
// 1 2  averages to 1         2 2  averages to 2
void halfsize(unsigned char* halfim, unsigned char* im, int width, int height){
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


Image<unsigned char> halfsize(const Image<unsigned char>& from){
	Image<unsigned char> result(from.size()/2);
	const unsigned char* halfim = result.data();
	const unsigned char* im = from.data();
	const int width = from.size().x;
	const int height = from.size().y;

	for(int iy=0; iy<height/2; iy++){
		for(int ix=0; ix<width/16; ix++){
			uint8x16_t r1pixels = vld1q_u8((uint8_t*)im+(16*ix+2*iy*width)); // load 16 8-bit pixels from row 1
			uint8x16_t r2pixels = vld1q_u8((uint8_t*)im+(16*ix+2*iy*width+width)); // load 16 8-bit pixels from row 2

			uint8x16_t avpixels = vrhaddq_u8(r1pixels,r2pixels); // average the rows into a 8x16 (rounding up here)
			uint16x8_t averaged_16 = vpaddlq_u8(avpixels);        //average the columns into a 16x8
			uint8x8_t averaged = vshrn_n_u16(averaged_16,1);  // divide by two and narrow into 8x8 (rounding down here)

			vst1_u8((uint8_t*)(halfim+8*ix+(width/2)*iy),averaged); // store 8 8-bit values into destination image
		}
	}
	return result;
}


Image<unsigned char> three_two_size(const Image<unsigned char>& from){

	ImageRef result_size(16*(from.size().x/24),2*(from.size().y/3));
	Image<unsigned char> result (result_size);

	const unsigned char* im32 = result.data();
	const unsigned char* im = from.data();
	const int width = from.size().x;
	const int height = from.size().y;
	const int dwidth = result.size().x;
	const int dheight = result.size().y;


	const uint16_t multipliers1[8] = {2,1,2,2,1,2,2,1};
	const uint16_t multipliers2[8] = {2,2,1,2,2,1,2,2};
	const uint16_t multipliers3[8] = {1,2,2,1,2,2,1,2};

	const uint16x8_t m1 = vld1q_u16(multipliers1);
	const uint16x8_t m2 = vld1q_u16(multipliers2);
	const uint16x8_t m3 = vld1q_u16(multipliers3);

	const uint8_t lookup1[8]={0,1,3,4,6,7,9,10};
	const uint8_t lookup2[8]={12,13,15,16,18,19,21,22};

	const uint8x8_t l1 = vld1_u8(lookup1);
	const uint8x8_t l2 = vld1_u8(lookup2);


	for(int y=0; y<height/3; y++) { // three rows at a time
		for(int x = 0; x < width/24; x++){

			const int ix = x*24;
			const int iy = y*3;

			const int dx = x*16; // destination offsets
			const int dy = y*2;

			// load top row
			uint8x8_t p11 = vld1_u8((uint8_t*)im+iy*width+ix);
			uint8x8_t p12 = vld1_u8((uint8_t*)im+iy*width+ix+8);
			uint8x8_t p13 = vld1_u8((uint8_t*)im+iy*width+ix+16);

			// load middle row
			uint8x8_t p21 = vld1_u8((uint8_t*)im+iy*width+width+ix);
			uint8x8_t p22 = vld1_u8((uint8_t*)im+iy*width+width+ix+8);
			uint8x8_t p23 = vld1_u8((uint8_t*)im+iy*width+width+ix+16);

			// middle into 16 bit
			uint16x8_t lp21 = vmovl_u8(p21);
			uint16x8_t lp22 = vmovl_u8(p22);
			uint16x8_t lp23 = vmovl_u8(p23);

			// 2* top into 16 bit
			uint16x8_t c1 = vshll_n_u8(p11,1);
			uint16x8_t c2 = vshll_n_u8(p12,1);
			uint16x8_t c3 = vshll_n_u8(p13,1);

			// add middle
			c1 = vaddq_u16(c1,lp21);
			c2 = vaddq_u16(c2,lp22);
			c3 = vaddq_u16(c3,lp23);

			c1 = vmulq_u16(c1,m1); // multiply by 2,1,2,2,1,2,2,1
			c2 = vmulq_u16(c2,m2); // 2,2,1,2,2,1,2,2
			c3 = vmulq_u16(c3,m3); // 1,2,2,1,2,2,1,2

			// shift vectors along by one
			uint16x8_t c1s = vextq_u16(c1,c2,1);
			uint16x8_t c2s = vextq_u16(c2,c3,1);
			uint16x8_t c3s = vextq_u16(c3,c1,1);  // byte from c1 is irrelevant

			// add to get 9 * final pixel values
			c1 = vaddq_u16(c1,c1s); // contains p1,p2,x,p3,p4,x,p5,p6
			c2 = vaddq_u16(c2,c2s); // contains x,p7,p8,x,p9,p10,x,p11
			c3 = vaddq_u16(c3,c3s); // contains p12,x,p13,p14,x,p15,p16,x

			// divide by 9
			c1 = vqdmulhq_n_s16(c1,3641); // doubling multiply by closest approx of (2^15)/9
			c2 = vqdmulhq_n_s16(c2,3641); // doubling multiply by closest approx of (2^15)/9
			c3 = vqdmulhq_n_s16(c3,3641); // doubling multiply by closest approx of (2^15)/9

			// narrow back to 8x8
			uint8x8x3_t padded_results;
			padded_results.val[0] = vqmovn_u16(c1);
			padded_results.val[1] = vqmovn_u16(c2);
			padded_results.val[2] = vqmovn_u16(c3);

			// get rid of the garbage and pack into two vectors
			uint8x8_t result1 = vtbl3_u8(padded_results,l1);
			uint8x8_t result2 = vtbl3_u8(padded_results,l2);

			// store to dest
			vst1_u8((uint8_t*)im32+dy*dwidth+dx,result1);
			vst1_u8((uint8_t*)im32+dy*dwidth+dx+8,result2);

			// load bottom row
			uint8x8_t p31 = vld1_u8((uint8_t*)im+iy*width+2*width+ix);
			uint8x8_t p32 = vld1_u8((uint8_t*)im+iy*width+2*width+ix+8);
			uint8x8_t p33 = vld1_u8((uint8_t*)im+iy*width+2*width+ix+16);

			// 2* top into 16 bit
			c1 = vshll_n_u8(p31,1);
			c2 = vshll_n_u8(p32,1);
			c3 = vshll_n_u8(p33,1);

			// add middle
			c1 = vaddq_u16(c1,lp21);
			c2 = vaddq_u16(c2,lp22);
			c3 = vaddq_u16(c3,lp23);

			c1 = vmulq_u16(c1,m1); // multiply by 2,1,2,2,1,2,2,1
			c2 = vmulq_u16(c2,m2); // 2,2,1,2,2,1,2,2
			c3 = vmulq_u16(c3,m3); // 1,2,2,1,2,2,1,2

			// shift by 1 value
			c1s = vextq_u16(c1,c2,1);
			c2s = vextq_u16(c2,c3,1);
			c3s = vextq_u16(c3,c1,1);  // byte from c1 is irrelevant

			// add to get 9 * final pixel values
			c1 = vaddq_u16(c1,c1s); // contains p1,p2,x,p3,p4,x,p5,p6
			c2 = vaddq_u16(c2,c2s); // contains x,p7,p8,x,p9,p10,x,p11
			c3 = vaddq_u16(c3,c3s); // contains p12,x,p13,p14,x,p15,p16,x

			// divide by 9
			c1 = vqdmulhq_n_s16(c1,3641); // doubling multiply by closest approx of (2^15)/9
			c2 = vqdmulhq_n_s16(c2,3641); // doubling multiply by closest approx of (2^15)/9
			c3 = vqdmulhq_n_s16(c3,3641); // doubling multiply by closest approx of (2^15)/9

			// narrow back to 8x8
			padded_results.val[0] = vqmovn_u16(c1);
			padded_results.val[1] = vqmovn_u16(c2);
			padded_results.val[2] = vqmovn_u16(c3);

			// get rid of the garbage and pack into two vectors
			result1 = vtbl3_u8(padded_results,l1);
			result2 = vtbl3_u8(padded_results,l2);

			// store to dest
			vst1_u8((uint8_t*)im32+dy*dwidth+dwidth+dx,result1);
			vst1_u8((uint8_t*)im32+dy*dwidth+dwidth+dx+8,result2);
		}
	}
	return result;
}
