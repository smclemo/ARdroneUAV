/*This is mechanically generated code*/
#include <stdlib.h>

#include <arm_neon.h>

#include "Tom_fast9.h"

#include "cvd_lite/image.h"

#include "util/container.h"

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


/**
int* fast9_score(const byte* i, int stride, xy* corners, int num_corners, int b)
{	
	int* scores = (int*)malloc(sizeof(int)* num_corners);
	int n;

	int pixel[16];
	make_offsets(pixel, stride);

    for(n=0; n < num_corners; n++)
        scores[n] = fast9_corner_score(i + corners[n].y*stride + corners[n].x, pixel, b);

	return scores;
}
 **/
static byte bitpos[16]={
		1,2,4,8,16,32,64,128,1,2,4,8,16,32,64,128
};


inline unsigned long int TEST_16_PIXELS_NOSIGN(uint8x16_t low, uint8x16_t high, uint8x16_t other, uint8x16_t vbitpos) {
	uint8x16_t test1, test2;

	test1 = vcgtq_u8(low,other);     // test1 now holds 255 in locations where low barrier test passed
	test2 = vcgtq_u8(other,high);    // test2 now holds 255 in locations where high barrier test passed
	test1 = vorrq_u8(test1,test2);   // OR the two results together since we don't care about sign of light/dark for corner
	test1 = vandq_u8(test1,vbitpos); // test1 now holds 1,2,4,8,..,1,2,4,8,... in these locations
	test1 = vpaddlq_u8(test1);       // combine test1 in pairs in 16x8
	test1 = vpaddlq_u16(test1);      // combine into fours
	test1 = vpaddlq_u32(test1);      // combine into eights

	// the results are now 8 bit numbers in lanes 0 (pixels 0-7) and 8 (pixels 8-15)
	unsigned long int result = vgetq_lane_u8(test1,8);
	result <<= 8;
	result += vgetq_lane_u8(test1,0);

	return result;
}


void Tom_fast9_detect(const Image<unsigned char>& im, int barrier, int border, Container<CVD::ImageRef>& container) {
	container.clear();
	int pixel[16];
	int x, y, xoffset, yoffset;

	const int stride = im.stride();

	make_offsets(pixel, stride);

	uint8x16_t barr = vdupq_n_u8(barrier);
	uint8x16_t vbitpos = vld1q_u8(bitpos);

	// the border must be at least 3 pixels to allow for the FAST pixel ring to fit
	if(border < 3){border=3;}

	// exclude elements in first and last rows of 16 that fall within the border
	const unsigned int last_mask = 0b1111111111111111 >> 15-((im.size().x-border-1) % 16);
	const unsigned int first_mask = 0b1111111111111111 << (border % 16);


	const int stride_3 = 3*stride;
	const int xlimit = im.size().x-border-17;

	for(x=border-(border%16); x < im.size().x-border; x+=16) {
		unsigned int a_mask = 0b1111111111111111;
		if(x<=border){a_mask &= first_mask;}
		if(x>xlimit) {a_mask &= last_mask;}
		const unsigned int mask = a_mask;

		for(yoffset=border-3; yoffset<border; yoffset++){

			const unsigned char* p_16 = im.data() + (yoffset)*stride + x; // points to row of 16 pixels in one of top 3 rows

			uint8x16_t pixels = vld1q_u8(p_16);
			uint8x16_t below = vld1q_u8(p_16+3*stride);
			uint8x16_t above;

			for(y=3+yoffset; y < im.size().y - border; y+=3) {
				p_16 += stride_3; // now points to row of pixels being considered
				above = pixels;
				pixels = below;
				below = vld1q_u8(p_16+stride_3);

				uint8x16_t low = vqsubq_u8(pixels,barr);  // centre pixels minus barrier (saturating at 0)
				uint8x16_t high = vqaddq_u8(pixels,barr); // center pixels plus barrier (saturating at 255)

				unsigned long int possible = TEST_16_PIXELS_NOSIGN(low,high,above,vbitpos);
				possible |= TEST_16_PIXELS_NOSIGN(low,high,below,vbitpos);

				possible &= mask;

//				if(possible==0){continue;} // rapid check for non-cornerness of all 16 pixels
//				unsigned int testbit=1;
//				for(xoffset=0; xoffset<16; testbit<<=1, xoffset++){
//					if(!(possible&testbit)){continue;}

				for(xoffset=0; possible; possible >>=1, xoffset++) {
					if((possible&1)==0){ continue; }

					const unsigned char* p = im.data() + y*stride + x+xoffset;

					int cb = *p + barrier;
					int c_b= *p - barrier;
					if(p[pixel[0]] > cb)
						if(p[pixel[1]] > cb)
							if(p[pixel[2]] > cb)
								if(p[pixel[3]] > cb)
									if(p[pixel[4]] > cb)
										if(p[pixel[5]] > cb)
											if(p[pixel[6]] > cb)
												if(p[pixel[7]] > cb)
													if(p[pixel[8]] > cb)
													{}
													else
														if(p[pixel[15]] > cb)
														{}
														else
															continue;
												else if(p[pixel[7]] < c_b)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															continue;
													else if(p[pixel[14]] < c_b)
														if(p[pixel[8]] < c_b)
															if(p[pixel[9]] < c_b)
																if(p[pixel[10]] < c_b)
																	if(p[pixel[11]] < c_b)
																		if(p[pixel[12]] < c_b)
																			if(p[pixel[13]] < c_b)
																				if(p[pixel[15]] < c_b)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															continue;
													else
														continue;
											else if(p[pixel[6]] < c_b)
												if(p[pixel[15]] > cb)
													if(p[pixel[13]] > cb)
														if(p[pixel[14]] > cb)
														{}
														else
															continue;
													else if(p[pixel[13]] < c_b)
														if(p[pixel[7]] < c_b)
															if(p[pixel[8]] < c_b)
																if(p[pixel[9]] < c_b)
																	if(p[pixel[10]] < c_b)
																		if(p[pixel[11]] < c_b)
																			if(p[pixel[12]] < c_b)
																				if(p[pixel[14]] < c_b)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													if(p[pixel[7]] < c_b)
														if(p[pixel[8]] < c_b)
															if(p[pixel[9]] < c_b)
																if(p[pixel[10]] < c_b)
																	if(p[pixel[11]] < c_b)
																		if(p[pixel[12]] < c_b)
																			if(p[pixel[13]] < c_b)
																				if(p[pixel[14]] < c_b)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															continue;
													else
														continue;
												else if(p[pixel[13]] < c_b)
													if(p[pixel[7]] < c_b)
														if(p[pixel[8]] < c_b)
															if(p[pixel[9]] < c_b)
																if(p[pixel[10]] < c_b)
																	if(p[pixel[11]] < c_b)
																		if(p[pixel[12]] < c_b)
																			if(p[pixel[14]] < c_b)
																				if(p[pixel[15]] < c_b)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else if(p[pixel[5]] < c_b)
											if(p[pixel[14]] > cb)
												if(p[pixel[12]] > cb)
													if(p[pixel[13]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																			if(p[pixel[10]] > cb)
																				if(p[pixel[11]] > cb)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														continue;
												else if(p[pixel[12]] < c_b)
													if(p[pixel[6]] < c_b)
														if(p[pixel[7]] < c_b)
															if(p[pixel[8]] < c_b)
																if(p[pixel[9]] < c_b)
																	if(p[pixel[10]] < c_b)
																		if(p[pixel[11]] < c_b)
																			if(p[pixel[13]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else if(p[pixel[14]] < c_b)
												if(p[pixel[7]] < c_b)
													if(p[pixel[8]] < c_b)
														if(p[pixel[9]] < c_b)
															if(p[pixel[10]] < c_b)
																if(p[pixel[11]] < c_b)
																	if(p[pixel[12]] < c_b)
																		if(p[pixel[13]] < c_b)
																			if(p[pixel[6]] < c_b)
																			{}
																			else
																				if(p[pixel[15]] < c_b)
																				{}
																				else
																					continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												if(p[pixel[6]] < c_b)
													if(p[pixel[7]] < c_b)
														if(p[pixel[8]] < c_b)
															if(p[pixel[9]] < c_b)
																if(p[pixel[10]] < c_b)
																	if(p[pixel[11]] < c_b)
																		if(p[pixel[12]] < c_b)
																			if(p[pixel[13]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																			if(p[pixel[10]] > cb)
																				if(p[pixel[11]] > cb)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														continue;
												else
													continue;
											else if(p[pixel[12]] < c_b)
												if(p[pixel[7]] < c_b)
													if(p[pixel[8]] < c_b)
														if(p[pixel[9]] < c_b)
															if(p[pixel[10]] < c_b)
																if(p[pixel[11]] < c_b)
																	if(p[pixel[13]] < c_b)
																		if(p[pixel[14]] < c_b)
																			if(p[pixel[6]] < c_b)
																			{}
																			else
																				if(p[pixel[15]] < c_b)
																				{}
																				else
																					continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else if(p[pixel[4]] < c_b)
										if(p[pixel[13]] > cb)
											if(p[pixel[11]] > cb)
												if(p[pixel[12]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																			if(p[pixel[10]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																			if(p[pixel[10]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													continue;
											else if(p[pixel[11]] < c_b)
												if(p[pixel[5]] < c_b)
													if(p[pixel[6]] < c_b)
														if(p[pixel[7]] < c_b)
															if(p[pixel[8]] < c_b)
																if(p[pixel[9]] < c_b)
																	if(p[pixel[10]] < c_b)
																		if(p[pixel[12]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else if(p[pixel[13]] < c_b)
											if(p[pixel[7]] < c_b)
												if(p[pixel[8]] < c_b)
													if(p[pixel[9]] < c_b)
														if(p[pixel[10]] < c_b)
															if(p[pixel[11]] < c_b)
																if(p[pixel[12]] < c_b)
																	if(p[pixel[6]] < c_b)
																		if(p[pixel[5]] < c_b)
																		{}
																		else
																			if(p[pixel[14]] < c_b)
																			{}
																			else
																				continue;
																	else
																		if(p[pixel[14]] < c_b)
																			if(p[pixel[15]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											if(p[pixel[5]] < c_b)
												if(p[pixel[6]] < c_b)
													if(p[pixel[7]] < c_b)
														if(p[pixel[8]] < c_b)
															if(p[pixel[9]] < c_b)
																if(p[pixel[10]] < c_b)
																	if(p[pixel[11]] < c_b)
																		if(p[pixel[12]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																			if(p[pixel[10]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																			if(p[pixel[10]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													continue;
											else
												continue;
										else if(p[pixel[11]] < c_b)
											if(p[pixel[7]] < c_b)
												if(p[pixel[8]] < c_b)
													if(p[pixel[9]] < c_b)
														if(p[pixel[10]] < c_b)
															if(p[pixel[12]] < c_b)
																if(p[pixel[13]] < c_b)
																	if(p[pixel[6]] < c_b)
																		if(p[pixel[5]] < c_b)
																		{}
																		else
																			if(p[pixel[14]] < c_b)
																			{}
																			else
																				continue;
																	else
																		if(p[pixel[14]] < c_b)
																			if(p[pixel[15]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
								else if(p[pixel[3]] < c_b)
									if(p[pixel[10]] > cb)
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												continue;
										else
											continue;
									else if(p[pixel[10]] < c_b)
										if(p[pixel[7]] < c_b)
											if(p[pixel[8]] < c_b)
												if(p[pixel[9]] < c_b)
													if(p[pixel[11]] < c_b)
														if(p[pixel[6]] < c_b)
															if(p[pixel[5]] < c_b)
																if(p[pixel[4]] < c_b)
																{}
																else
																	if(p[pixel[12]] < c_b)
																		if(p[pixel[13]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
															else
																if(p[pixel[12]] < c_b)
																	if(p[pixel[13]] < c_b)
																		if(p[pixel[14]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
														else
															if(p[pixel[12]] < c_b)
																if(p[pixel[13]] < c_b)
																	if(p[pixel[14]] < c_b)
																		if(p[pixel[15]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
									else
										continue;
								else
									if(p[pixel[10]] > cb)
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																		if(p[pixel[9]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												continue;
										else
											continue;
									else if(p[pixel[10]] < c_b)
										if(p[pixel[7]] < c_b)
											if(p[pixel[8]] < c_b)
												if(p[pixel[9]] < c_b)
													if(p[pixel[11]] < c_b)
														if(p[pixel[12]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[5]] < c_b)
																	if(p[pixel[4]] < c_b)
																	{}
																	else
																		if(p[pixel[13]] < c_b)
																		{}
																		else
																			continue;
																else
																	if(p[pixel[13]] < c_b)
																		if(p[pixel[14]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
															else
																if(p[pixel[13]] < c_b)
																	if(p[pixel[14]] < c_b)
																		if(p[pixel[15]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
									else
										continue;
							else if(p[pixel[2]] < c_b)
								if(p[pixel[9]] > cb)
									if(p[pixel[10]] > cb)
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											continue;
									else
										continue;
								else if(p[pixel[9]] < c_b)
									if(p[pixel[7]] < c_b)
										if(p[pixel[8]] < c_b)
											if(p[pixel[10]] < c_b)
												if(p[pixel[6]] < c_b)
													if(p[pixel[5]] < c_b)
														if(p[pixel[4]] < c_b)
															if(p[pixel[3]] < c_b)
															{}
															else
																if(p[pixel[11]] < c_b)
																	if(p[pixel[12]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
														else
															if(p[pixel[11]] < c_b)
																if(p[pixel[12]] < c_b)
																	if(p[pixel[13]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[11]] < c_b)
															if(p[pixel[12]] < c_b)
																if(p[pixel[13]] < c_b)
																	if(p[pixel[14]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[11]] < c_b)
														if(p[pixel[12]] < c_b)
															if(p[pixel[13]] < c_b)
																if(p[pixel[14]] < c_b)
																	if(p[pixel[15]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												continue;
										else
											continue;
									else
										continue;
								else
									continue;
							else
								if(p[pixel[9]] > cb)
									if(p[pixel[10]] > cb)
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																	if(p[pixel[8]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											continue;
									else
										continue;
								else if(p[pixel[9]] < c_b)
									if(p[pixel[7]] < c_b)
										if(p[pixel[8]] < c_b)
											if(p[pixel[10]] < c_b)
												if(p[pixel[11]] < c_b)
													if(p[pixel[6]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[4]] < c_b)
																if(p[pixel[3]] < c_b)
																{}
																else
																	if(p[pixel[12]] < c_b)
																	{}
																	else
																		continue;
															else
																if(p[pixel[12]] < c_b)
																	if(p[pixel[13]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
														else
															if(p[pixel[12]] < c_b)
																if(p[pixel[13]] < c_b)
																	if(p[pixel[14]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[12]] < c_b)
															if(p[pixel[13]] < c_b)
																if(p[pixel[14]] < c_b)
																	if(p[pixel[15]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													continue;
											else
												continue;
										else
											continue;
									else
										continue;
								else
									continue;
						else if(p[pixel[1]] < c_b)
							if(p[pixel[8]] > cb)
								if(p[pixel[9]] > cb)
									if(p[pixel[10]] > cb)
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[2]] > cb)
												if(p[pixel[3]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										continue;
								else
									continue;
							else if(p[pixel[8]] < c_b)
								if(p[pixel[7]] < c_b)
									if(p[pixel[9]] < c_b)
										if(p[pixel[6]] < c_b)
											if(p[pixel[5]] < c_b)
												if(p[pixel[4]] < c_b)
													if(p[pixel[3]] < c_b)
														if(p[pixel[2]] < c_b)
														{}
														else
															if(p[pixel[10]] < c_b)
																if(p[pixel[11]] < c_b)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[10]] < c_b)
															if(p[pixel[11]] < c_b)
																if(p[pixel[12]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[10]] < c_b)
														if(p[pixel[11]] < c_b)
															if(p[pixel[12]] < c_b)
																if(p[pixel[13]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[10]] < c_b)
													if(p[pixel[11]] < c_b)
														if(p[pixel[12]] < c_b)
															if(p[pixel[13]] < c_b)
																if(p[pixel[14]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[10]] < c_b)
												if(p[pixel[11]] < c_b)
													if(p[pixel[12]] < c_b)
														if(p[pixel[13]] < c_b)
															if(p[pixel[14]] < c_b)
																if(p[pixel[15]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										continue;
								else
									continue;
							else
								continue;
						else
							if(p[pixel[8]] > cb)
								if(p[pixel[9]] > cb)
									if(p[pixel[10]] > cb)
										if(p[pixel[11]] > cb)
											if(p[pixel[12]] > cb)
												if(p[pixel[13]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[15]] > cb)
														{}
														else
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[2]] > cb)
												if(p[pixel[3]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[7]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										continue;
								else
									continue;
							else if(p[pixel[8]] < c_b)
								if(p[pixel[7]] < c_b)
									if(p[pixel[9]] < c_b)
										if(p[pixel[10]] < c_b)
											if(p[pixel[6]] < c_b)
												if(p[pixel[5]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[3]] < c_b)
															if(p[pixel[2]] < c_b)
															{}
															else
																if(p[pixel[11]] < c_b)
																{}
																else
																	continue;
														else
															if(p[pixel[11]] < c_b)
																if(p[pixel[12]] < c_b)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[11]] < c_b)
															if(p[pixel[12]] < c_b)
																if(p[pixel[13]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[11]] < c_b)
														if(p[pixel[12]] < c_b)
															if(p[pixel[13]] < c_b)
																if(p[pixel[14]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[11]] < c_b)
													if(p[pixel[12]] < c_b)
														if(p[pixel[13]] < c_b)
															if(p[pixel[14]] < c_b)
																if(p[pixel[15]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											continue;
									else
										continue;
								else
									continue;
							else
								continue;
					else if(p[pixel[0]] < c_b)
						if(p[pixel[1]] > cb)
							if(p[pixel[8]] > cb)
								if(p[pixel[7]] > cb)
									if(p[pixel[9]] > cb)
										if(p[pixel[6]] > cb)
											if(p[pixel[5]] > cb)
												if(p[pixel[4]] > cb)
													if(p[pixel[3]] > cb)
														if(p[pixel[2]] > cb)
														{}
														else
															if(p[pixel[10]] > cb)
																if(p[pixel[11]] > cb)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[10]] > cb)
															if(p[pixel[11]] > cb)
																if(p[pixel[12]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[10]] > cb)
														if(p[pixel[11]] > cb)
															if(p[pixel[12]] > cb)
																if(p[pixel[13]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[10]] > cb)
													if(p[pixel[11]] > cb)
														if(p[pixel[12]] > cb)
															if(p[pixel[13]] > cb)
																if(p[pixel[14]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[10]] > cb)
												if(p[pixel[11]] > cb)
													if(p[pixel[12]] > cb)
														if(p[pixel[13]] > cb)
															if(p[pixel[14]] > cb)
																if(p[pixel[15]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										continue;
								else
									continue;
							else if(p[pixel[8]] < c_b)
								if(p[pixel[9]] < c_b)
									if(p[pixel[10]] < c_b)
										if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[2]] < c_b)
												if(p[pixel[3]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										continue;
								else
									continue;
							else
								continue;
						else if(p[pixel[1]] < c_b)
							if(p[pixel[2]] > cb)
								if(p[pixel[9]] > cb)
									if(p[pixel[7]] > cb)
										if(p[pixel[8]] > cb)
											if(p[pixel[10]] > cb)
												if(p[pixel[6]] > cb)
													if(p[pixel[5]] > cb)
														if(p[pixel[4]] > cb)
															if(p[pixel[3]] > cb)
															{}
															else
																if(p[pixel[11]] > cb)
																	if(p[pixel[12]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
														else
															if(p[pixel[11]] > cb)
																if(p[pixel[12]] > cb)
																	if(p[pixel[13]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[11]] > cb)
															if(p[pixel[12]] > cb)
																if(p[pixel[13]] > cb)
																	if(p[pixel[14]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[11]] > cb)
														if(p[pixel[12]] > cb)
															if(p[pixel[13]] > cb)
																if(p[pixel[14]] > cb)
																	if(p[pixel[15]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												continue;
										else
											continue;
									else
										continue;
								else if(p[pixel[9]] < c_b)
									if(p[pixel[10]] < c_b)
										if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											continue;
									else
										continue;
								else
									continue;
							else if(p[pixel[2]] < c_b)
								if(p[pixel[3]] > cb)
									if(p[pixel[10]] > cb)
										if(p[pixel[7]] > cb)
											if(p[pixel[8]] > cb)
												if(p[pixel[9]] > cb)
													if(p[pixel[11]] > cb)
														if(p[pixel[6]] > cb)
															if(p[pixel[5]] > cb)
																if(p[pixel[4]] > cb)
																{}
																else
																	if(p[pixel[12]] > cb)
																		if(p[pixel[13]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
															else
																if(p[pixel[12]] > cb)
																	if(p[pixel[13]] > cb)
																		if(p[pixel[14]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
														else
															if(p[pixel[12]] > cb)
																if(p[pixel[13]] > cb)
																	if(p[pixel[14]] > cb)
																		if(p[pixel[15]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
									else if(p[pixel[10]] < c_b)
										if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												continue;
										else
											continue;
									else
										continue;
								else if(p[pixel[3]] < c_b)
									if(p[pixel[4]] > cb)
										if(p[pixel[13]] > cb)
											if(p[pixel[7]] > cb)
												if(p[pixel[8]] > cb)
													if(p[pixel[9]] > cb)
														if(p[pixel[10]] > cb)
															if(p[pixel[11]] > cb)
																if(p[pixel[12]] > cb)
																	if(p[pixel[6]] > cb)
																		if(p[pixel[5]] > cb)
																		{}
																		else
																			if(p[pixel[14]] > cb)
																			{}
																			else
																				continue;
																	else
																		if(p[pixel[14]] > cb)
																			if(p[pixel[15]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else if(p[pixel[13]] < c_b)
											if(p[pixel[11]] > cb)
												if(p[pixel[5]] > cb)
													if(p[pixel[6]] > cb)
														if(p[pixel[7]] > cb)
															if(p[pixel[8]] > cb)
																if(p[pixel[9]] > cb)
																	if(p[pixel[10]] > cb)
																		if(p[pixel[12]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else if(p[pixel[11]] < c_b)
												if(p[pixel[12]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																			if(p[pixel[10]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																			if(p[pixel[10]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													continue;
											else
												continue;
										else
											if(p[pixel[5]] > cb)
												if(p[pixel[6]] > cb)
													if(p[pixel[7]] > cb)
														if(p[pixel[8]] > cb)
															if(p[pixel[9]] > cb)
																if(p[pixel[10]] > cb)
																	if(p[pixel[11]] > cb)
																		if(p[pixel[12]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else if(p[pixel[4]] < c_b)
										if(p[pixel[5]] > cb)
											if(p[pixel[14]] > cb)
												if(p[pixel[7]] > cb)
													if(p[pixel[8]] > cb)
														if(p[pixel[9]] > cb)
															if(p[pixel[10]] > cb)
																if(p[pixel[11]] > cb)
																	if(p[pixel[12]] > cb)
																		if(p[pixel[13]] > cb)
																			if(p[pixel[6]] > cb)
																			{}
																			else
																				if(p[pixel[15]] > cb)
																				{}
																				else
																					continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else if(p[pixel[14]] < c_b)
												if(p[pixel[12]] > cb)
													if(p[pixel[6]] > cb)
														if(p[pixel[7]] > cb)
															if(p[pixel[8]] > cb)
																if(p[pixel[9]] > cb)
																	if(p[pixel[10]] > cb)
																		if(p[pixel[11]] > cb)
																			if(p[pixel[13]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else if(p[pixel[12]] < c_b)
													if(p[pixel[13]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																			if(p[pixel[10]] < c_b)
																				if(p[pixel[11]] < c_b)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														continue;
												else
													continue;
											else
												if(p[pixel[6]] > cb)
													if(p[pixel[7]] > cb)
														if(p[pixel[8]] > cb)
															if(p[pixel[9]] > cb)
																if(p[pixel[10]] > cb)
																	if(p[pixel[11]] > cb)
																		if(p[pixel[12]] > cb)
																			if(p[pixel[13]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else if(p[pixel[5]] < c_b)
											if(p[pixel[6]] > cb)
												if(p[pixel[15]] < c_b)
													if(p[pixel[13]] > cb)
														if(p[pixel[7]] > cb)
															if(p[pixel[8]] > cb)
																if(p[pixel[9]] > cb)
																	if(p[pixel[10]] > cb)
																		if(p[pixel[11]] > cb)
																			if(p[pixel[12]] > cb)
																				if(p[pixel[14]] > cb)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else if(p[pixel[13]] < c_b)
														if(p[pixel[14]] < c_b)
														{}
														else
															continue;
													else
														continue;
												else
													if(p[pixel[7]] > cb)
														if(p[pixel[8]] > cb)
															if(p[pixel[9]] > cb)
																if(p[pixel[10]] > cb)
																	if(p[pixel[11]] > cb)
																		if(p[pixel[12]] > cb)
																			if(p[pixel[13]] > cb)
																				if(p[pixel[14]] > cb)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else if(p[pixel[6]] < c_b)
												if(p[pixel[7]] > cb)
													if(p[pixel[14]] > cb)
														if(p[pixel[8]] > cb)
															if(p[pixel[9]] > cb)
																if(p[pixel[10]] > cb)
																	if(p[pixel[11]] > cb)
																		if(p[pixel[12]] > cb)
																			if(p[pixel[13]] > cb)
																				if(p[pixel[15]] > cb)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															continue;
													else
														continue;
												else if(p[pixel[7]] < c_b)
													if(p[pixel[8]] < c_b)
													{}
													else
														if(p[pixel[15]] < c_b)
														{}
														else
															continue;
												else
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															continue;
													else
														continue;
											else
												if(p[pixel[13]] > cb)
													if(p[pixel[7]] > cb)
														if(p[pixel[8]] > cb)
															if(p[pixel[9]] > cb)
																if(p[pixel[10]] > cb)
																	if(p[pixel[11]] > cb)
																		if(p[pixel[12]] > cb)
																			if(p[pixel[14]] > cb)
																				if(p[pixel[15]] > cb)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[12]] > cb)
												if(p[pixel[7]] > cb)
													if(p[pixel[8]] > cb)
														if(p[pixel[9]] > cb)
															if(p[pixel[10]] > cb)
																if(p[pixel[11]] > cb)
																	if(p[pixel[13]] > cb)
																		if(p[pixel[14]] > cb)
																			if(p[pixel[6]] > cb)
																			{}
																			else
																				if(p[pixel[15]] > cb)
																				{}
																				else
																					continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																			if(p[pixel[10]] < c_b)
																				if(p[pixel[11]] < c_b)
																				{}
																				else
																					continue;
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										if(p[pixel[11]] > cb)
											if(p[pixel[7]] > cb)
												if(p[pixel[8]] > cb)
													if(p[pixel[9]] > cb)
														if(p[pixel[10]] > cb)
															if(p[pixel[12]] > cb)
																if(p[pixel[13]] > cb)
																	if(p[pixel[6]] > cb)
																		if(p[pixel[5]] > cb)
																		{}
																		else
																			if(p[pixel[14]] > cb)
																			{}
																			else
																				continue;
																	else
																		if(p[pixel[14]] > cb)
																			if(p[pixel[15]] > cb)
																			{}
																			else
																				continue;
																		else
																			continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																			if(p[pixel[10]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																			if(p[pixel[10]] < c_b)
																			{}
																			else
																				continue;
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													continue;
											else
												continue;
										else
											continue;
								else
									if(p[pixel[10]] > cb)
										if(p[pixel[7]] > cb)
											if(p[pixel[8]] > cb)
												if(p[pixel[9]] > cb)
													if(p[pixel[11]] > cb)
														if(p[pixel[12]] > cb)
															if(p[pixel[6]] > cb)
																if(p[pixel[5]] > cb)
																	if(p[pixel[4]] > cb)
																	{}
																	else
																		if(p[pixel[13]] > cb)
																		{}
																		else
																			continue;
																else
																	if(p[pixel[13]] > cb)
																		if(p[pixel[14]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
															else
																if(p[pixel[13]] > cb)
																	if(p[pixel[14]] > cb)
																		if(p[pixel[15]] > cb)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
									else if(p[pixel[10]] < c_b)
										if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																		if(p[pixel[9]] < c_b)
																		{}
																		else
																			continue;
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												continue;
										else
											continue;
									else
										continue;
							else
								if(p[pixel[9]] > cb)
									if(p[pixel[7]] > cb)
										if(p[pixel[8]] > cb)
											if(p[pixel[10]] > cb)
												if(p[pixel[11]] > cb)
													if(p[pixel[6]] > cb)
														if(p[pixel[5]] > cb)
															if(p[pixel[4]] > cb)
																if(p[pixel[3]] > cb)
																{}
																else
																	if(p[pixel[12]] > cb)
																	{}
																	else
																		continue;
															else
																if(p[pixel[12]] > cb)
																	if(p[pixel[13]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
														else
															if(p[pixel[12]] > cb)
																if(p[pixel[13]] > cb)
																	if(p[pixel[14]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[12]] > cb)
															if(p[pixel[13]] > cb)
																if(p[pixel[14]] > cb)
																	if(p[pixel[15]] > cb)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													continue;
											else
												continue;
										else
											continue;
									else
										continue;
								else if(p[pixel[9]] < c_b)
									if(p[pixel[10]] < c_b)
										if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																	if(p[pixel[8]] < c_b)
																	{}
																	else
																		continue;
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											continue;
									else
										continue;
								else
									continue;
						else
							if(p[pixel[8]] > cb)
								if(p[pixel[7]] > cb)
									if(p[pixel[9]] > cb)
										if(p[pixel[10]] > cb)
											if(p[pixel[6]] > cb)
												if(p[pixel[5]] > cb)
													if(p[pixel[4]] > cb)
														if(p[pixel[3]] > cb)
															if(p[pixel[2]] > cb)
															{}
															else
																if(p[pixel[11]] > cb)
																{}
																else
																	continue;
														else
															if(p[pixel[11]] > cb)
																if(p[pixel[12]] > cb)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[11]] > cb)
															if(p[pixel[12]] > cb)
																if(p[pixel[13]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[11]] > cb)
														if(p[pixel[12]] > cb)
															if(p[pixel[13]] > cb)
																if(p[pixel[14]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[11]] > cb)
													if(p[pixel[12]] > cb)
														if(p[pixel[13]] > cb)
															if(p[pixel[14]] > cb)
																if(p[pixel[15]] > cb)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											continue;
									else
										continue;
								else
									continue;
							else if(p[pixel[8]] < c_b)
								if(p[pixel[9]] < c_b)
									if(p[pixel[10]] < c_b)
										if(p[pixel[11]] < c_b)
											if(p[pixel[12]] < c_b)
												if(p[pixel[13]] < c_b)
													if(p[pixel[14]] < c_b)
														if(p[pixel[15]] < c_b)
														{}
														else
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
													else
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
												else
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[3]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[2]] < c_b)
												if(p[pixel[3]] < c_b)
													if(p[pixel[4]] < c_b)
														if(p[pixel[5]] < c_b)
															if(p[pixel[6]] < c_b)
																if(p[pixel[7]] < c_b)
																{}
																else
																	continue;
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										continue;
								else
									continue;
							else
								continue;
					else
						if(p[pixel[7]] > cb)
							if(p[pixel[8]] > cb)
								if(p[pixel[9]] > cb)
									if(p[pixel[6]] > cb)
										if(p[pixel[5]] > cb)
											if(p[pixel[4]] > cb)
												if(p[pixel[3]] > cb)
													if(p[pixel[2]] > cb)
														if(p[pixel[1]] > cb)
														{}
														else
															if(p[pixel[10]] > cb)
															{}
															else
																continue;
													else
														if(p[pixel[10]] > cb)
															if(p[pixel[11]] > cb)
															{}
															else
																continue;
														else
															continue;
												else
													if(p[pixel[10]] > cb)
														if(p[pixel[11]] > cb)
															if(p[pixel[12]] > cb)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[10]] > cb)
													if(p[pixel[11]] > cb)
														if(p[pixel[12]] > cb)
															if(p[pixel[13]] > cb)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[10]] > cb)
												if(p[pixel[11]] > cb)
													if(p[pixel[12]] > cb)
														if(p[pixel[13]] > cb)
															if(p[pixel[14]] > cb)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										if(p[pixel[10]] > cb)
											if(p[pixel[11]] > cb)
												if(p[pixel[12]] > cb)
													if(p[pixel[13]] > cb)
														if(p[pixel[14]] > cb)
															if(p[pixel[15]] > cb)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
								else
									continue;
							else
								continue;
						else if(p[pixel[7]] < c_b)
							if(p[pixel[8]] < c_b)
								if(p[pixel[9]] < c_b)
									if(p[pixel[6]] < c_b)
										if(p[pixel[5]] < c_b)
											if(p[pixel[4]] < c_b)
												if(p[pixel[3]] < c_b)
													if(p[pixel[2]] < c_b)
														if(p[pixel[1]] < c_b)
														{}
														else
															if(p[pixel[10]] < c_b)
															{}
															else
																continue;
													else
														if(p[pixel[10]] < c_b)
															if(p[pixel[11]] < c_b)
															{}
															else
																continue;
														else
															continue;
												else
													if(p[pixel[10]] < c_b)
														if(p[pixel[11]] < c_b)
															if(p[pixel[12]] < c_b)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
											else
												if(p[pixel[10]] < c_b)
													if(p[pixel[11]] < c_b)
														if(p[pixel[12]] < c_b)
															if(p[pixel[13]] < c_b)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
										else
											if(p[pixel[10]] < c_b)
												if(p[pixel[11]] < c_b)
													if(p[pixel[12]] < c_b)
														if(p[pixel[13]] < c_b)
															if(p[pixel[14]] < c_b)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
									else
										if(p[pixel[10]] < c_b)
											if(p[pixel[11]] < c_b)
												if(p[pixel[12]] < c_b)
													if(p[pixel[13]] < c_b)
														if(p[pixel[14]] < c_b)
															if(p[pixel[15]] < c_b)
															{}
															else
																continue;
														else
															continue;
													else
														continue;
												else
													continue;
											else
												continue;
										else
											continue;
								else
									continue;
							else
								continue;
						else
							continue;

					ImageRef corner(x+xoffset,y);
					container.add(corner);
				}
			}
		}

	}
}


