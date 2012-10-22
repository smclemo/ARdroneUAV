// -*- c++ -*-

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <arm_neon.h>

#include "cvd_lite/image.h"
#include <arm_neon.h>
#include <math.h>
#include <string.h>

static const uint32x2_t magic_multiplier = vdup_n_u32(0b00000001000000010000000100000001);



inline unsigned int bitcount64(const uint8x8_t& reg){
	const uint8x8_t counts8 = vcnt_u8(reg);
	const uint32x2_t accum1 = vmul_u32(counts8,magic_multiplier);  // bytes 3 and 7 of accum hold bitcounts for bytes 0-3 and bytes 4-7
	const uint32x2_t accum2 = (uint32x2_t) vpaddl_u32(accum1); // combine bytes 3 and 7 into byte 3

	return vget_lane_u8(accum2,3);
}


class Descriptor {
public:
	Descriptor(){}

	inline void get_from_patch(CVD::ImageRef& p, CVD::Image<unsigned char>& im);
	inline void rget_from_patch(CVD::ImageRef& p, CVD::Image<unsigned char>& im);

	inline Descriptor operator|(const Descriptor& rhs) {
		Descriptor d;
		d.my_patch[0]=vorr_u8(my_patch[0],rhs.my_patch[0]);
		d.my_patch[1]=vorr_u8(my_patch[1],rhs.my_patch[1]);
		d.my_patch[2]=vorr_u8(my_patch[2],rhs.my_patch[2]);
		d.my_patch[3]=vorr_u8(my_patch[3],rhs.my_patch[3]);
		return d;
	}

	inline Descriptor operator&(const Descriptor& rhs) {
		Descriptor d;
		d.my_patch[0]=vand_u8(my_patch[0],rhs.my_patch[0]);
		d.my_patch[1]=vand_u8(my_patch[1],rhs.my_patch[1]);
		d.my_patch[2]=vand_u8(my_patch[2],rhs.my_patch[2]);
		d.my_patch[3]=vand_u8(my_patch[3],rhs.my_patch[3]);
		return d;
	}

	inline Descriptor operator~(){
		Descriptor d;
		d.my_patch[0] = vmvn_u8 (my_patch[0]);
		d.my_patch[1] = vmvn_u8 (my_patch[1]);
		d.my_patch[2] = vmvn_u8 (my_patch[2]);
		d.my_patch[3] = vmvn_u8 (my_patch[3]);
		return d;
	}

	inline unsigned int bitcount(){
		return bitcount64(my_patch[0]) + bitcount64(my_patch[1]) + bitcount64(my_patch[2]) + bitcount64(my_patch[3]);
	}


	// more efficient way of computing (fp & ~ *this).bitcount();
	// where fp is a descriptor computed from a patch
	// (ie has only 1 bit set in each slot across all 4 layers)
	inline unsigned int error_of(const Descriptor& fp) const {

		uint8x8_t diff = vbic_u8(fp.my_patch[0], my_patch[0]);  // fp.my_patch & ~ my_patch
		uint8x8_t diff1 = vbic_u8(fp.my_patch[1], my_patch[1]);
		diff = vorr_u8(diff,diff1);
		uint8x8_t diff2 = vbic_u8(fp.my_patch[2], my_patch[2]);
		diff = vorr_u8(diff,diff2);
		uint8x8_t diff3 = vbic_u8(fp.my_patch[3], my_patch[3]);
		diff = vorr_u8(diff,diff3);

		return bitcount64(diff);
	}

	void serialise(char * serialised)
	{
		memcpy(serialised, my_patch, 32);
	}
public:
	uint8x8_t my_patch[4];
	static CVD::ImageRef rhips[64];
};



inline void Descriptor::get_from_patch(CVD::ImageRef& p, CVD::Image<unsigned char>& im) {

	const int stride = im.size().x;
	uint8x8_t patch_rows[8];

	uint16x8_t squared_values;
	uint32x4_t accumulate_squares;

	uint16x4_t accumulate_values;

	CVD::ImageRef offset(-7,-7); // start of patch relative to pixel
	unsigned char* prow = &(im[p+offset]);

	for(int r=0; r<8; prow += 2*stride, r++){
		uint8x16_t row = vld1q_u8(prow);
		patch_rows[r] = vmovn_u16(row);
	}

	accumulate_values = vpaddl_u8(patch_rows[0]);
	squared_values = vmull_u8(patch_rows[0],patch_rows[0]);
	accumulate_squares = vpaddlq_u16(squared_values);

	for(int r=1;r<8;r++){
		accumulate_values = vpadal_u8(accumulate_values,patch_rows[r]);
		squared_values = vmull_u8(patch_rows[r],patch_rows[r]);
		accumulate_squares = vpadalq_u16(accumulate_squares,squared_values);
	}
	uint32x2_t acc32 = vpaddl_u16(accumulate_values);
	uint64x1_t acc64 = vpaddl_u32(acc32);

	const unsigned int sumval = vget_lane_u32((uint32x2_t)acc64,0);

	uint64x2_t accsq64 = vpaddlq_u32(accumulate_squares);

	const unsigned int sumsqval = vgetq_lane_u32((uint32x4_t)accsq64,0)
								+ vgetq_lane_u32((uint32x4_t)accsq64,2);

	const float avg = sumval / 64.0;
	const float var = (64*sumsqval - sumval*sumval)/(64.0*64.0);
	const float sigma = (int) sqrtf(var);

	const float quarter_dist = 0.675*sigma; // 0.675 sigma corresponds to 25% of a normal distribution

	// round thresholds down to integer and use > to test
	const int low_thresh = avg - quarter_dist;
	const int mid_thresh = avg;
	const int high_thresh = avg + quarter_dist;

	uint8x8_t avgx8 = vdup_n_u8(mid_thresh);
	uint8x8_t avgmsigmax8 = vdup_n_u8(low_thresh);
	uint8x8_t avgpsigmax8 = vdup_n_u8(high_thresh);

	uint8x8_t test_low = vcgt_u8(patch_rows[0],avgmsigmax8);
	uint8x8_t low1 = vcgt_u8(patch_rows[1],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low1,1);
	uint8x8_t low2 = vcgt_u8(patch_rows[2],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low2,2);
	uint8x8_t low3 = vcgt_u8(patch_rows[3],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low3,3);
	uint8x8_t low4 = vcgt_u8(patch_rows[4],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low4,4);
	uint8x8_t low5 = vcgt_u8(patch_rows[5],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low5,5);
	uint8x8_t low6 = vcgt_u8(patch_rows[6],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low6,6);
	uint8x8_t low7 = vcgt_u8(patch_rows[7],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low7,7);

	uint8x8_t test_mid = vcgt_u8(patch_rows[0],avgx8);
	uint8x8_t mid1 = vcgt_u8(patch_rows[1],avgx8);
	test_mid = vsli_n_u8(test_mid,mid1,1);
	uint8x8_t mid2 = vcgt_u8(patch_rows[2],avgx8);
	test_mid = vsli_n_u8(test_mid,mid2,2);
	uint8x8_t mid3 = vcgt_u8(patch_rows[3],avgx8);
	test_mid = vsli_n_u8(test_mid,mid3,3);
	uint8x8_t mid4 = vcgt_u8(patch_rows[4],avgx8);
	test_mid = vsli_n_u8(test_mid,mid4,4);
	uint8x8_t mid5 = vcgt_u8(patch_rows[5],avgx8);
	test_mid = vsli_n_u8(test_mid,mid5,5);
	uint8x8_t mid6 = vcgt_u8(patch_rows[6],avgx8);
	test_mid = vsli_n_u8(test_mid,mid6,6);
	uint8x8_t mid7 = vcgt_u8(patch_rows[7],avgx8);
	test_mid = vsli_n_u8(test_mid,mid7,7);

	uint8x8_t test_high = vcgt_u8(patch_rows[0],avgpsigmax8);
	uint8x8_t high1 = vcgt_u8(patch_rows[1],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high1,1);
	uint8x8_t high2 = vcgt_u8(patch_rows[2],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high2,2);
	uint8x8_t high3 = vcgt_u8(patch_rows[3],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high3,3);
	uint8x8_t high4 = vcgt_u8(patch_rows[4],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high4,4);
	uint8x8_t high5 = vcgt_u8(patch_rows[5],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high5,5);
	uint8x8_t high6 = vcgt_u8(patch_rows[6],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high6,6);
	uint8x8_t high7 = vcgt_u8(patch_rows[7],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high7,7);

	my_patch[0] = vmvn_u8(test_low);
	my_patch[1] = vbic_u8(test_low, test_mid);
	my_patch[2] = vbic_u8(test_mid, test_high);
	my_patch[3] = test_high;

//		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG,
//				"patch pattern = \n %016llX \n %016llX \n %016llX \n %016llX", my_patch[0], my_patch[1], my_patch[2], my_patch[3] );

}


inline void Descriptor::rget_from_patch(CVD::ImageRef& p, CVD::Image<unsigned char>& im) {

	const int stride = im.size().x;
	uint8x8_t patch_rows[8];

	uint16x8_t squared_values;
	uint32x4_t accumulate_squares;

	uint16x4_t accumulate_values;

//	CVD::ImageRef offset(-7,-7); // start of patch relative to pixel
//	unsigned char* prow = &(im[p+offset]);
//
//	for(int r=0; r<8; prow += 2*stride, r++){
//		uint8x16_t row = vld1q_u8(prow);
//		patch_rows[r] = vmovn_u16(row);
//	}

	uint8_t patches[64];

	for(int r=0; r < 64; r++)
	{
		patches[r] = im[p+rhips[r] ];
	}

	for(int r=0; r<8; r++)
	{
		patch_rows[r] = vld1_u8(&patches[8*r]);
	}

	accumulate_values = vpaddl_u8(patch_rows[0]);
	squared_values = vmull_u8(patch_rows[0],patch_rows[0]);
	accumulate_squares = vpaddlq_u16(squared_values);

	for(int r=1;r<8;r++){
		accumulate_values = vpadal_u8(accumulate_values,patch_rows[r]);
		squared_values = vmull_u8(patch_rows[r],patch_rows[r]);
		accumulate_squares = vpadalq_u16(accumulate_squares,squared_values);
	}
	uint32x2_t acc32 = vpaddl_u16(accumulate_values);
	uint64x1_t acc64 = vpaddl_u32(acc32);

	const unsigned int sumval = vget_lane_u32((uint32x2_t)acc64,0);

	uint64x2_t accsq64 = vpaddlq_u32(accumulate_squares);

	const unsigned int sumsqval = vgetq_lane_u32((uint32x4_t)accsq64,0)
								+ vgetq_lane_u32((uint32x4_t)accsq64,2);

	const float avg = sumval / 64.0;
	const float var = (64*sumsqval - sumval*sumval)/(64.0*64.0);
	const float sigma = (int) sqrtf(var);

	const float quarter_dist = 0.675*sigma; // 0.675 sigma corresponds to 25% of a normal distribution

	// round thresholds down to integer and use > to test
	const int low_thresh = avg - quarter_dist;
	const int mid_thresh = avg;
	const int high_thresh = avg + quarter_dist;

	uint8x8_t avgx8 = vdup_n_u8(mid_thresh);
	uint8x8_t avgmsigmax8 = vdup_n_u8(low_thresh);
	uint8x8_t avgpsigmax8 = vdup_n_u8(high_thresh);

	uint8x8_t test_low = vcgt_u8(patch_rows[0],avgmsigmax8);
	uint8x8_t low1 = vcgt_u8(patch_rows[1],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low1,1);
	uint8x8_t low2 = vcgt_u8(patch_rows[2],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low2,2);
	uint8x8_t low3 = vcgt_u8(patch_rows[3],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low3,3);
	uint8x8_t low4 = vcgt_u8(patch_rows[4],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low4,4);
	uint8x8_t low5 = vcgt_u8(patch_rows[5],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low5,5);
	uint8x8_t low6 = vcgt_u8(patch_rows[6],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low6,6);
	uint8x8_t low7 = vcgt_u8(patch_rows[7],avgmsigmax8);
	test_low = vsli_n_u8(test_low,low7,7);

	uint8x8_t test_mid = vcgt_u8(patch_rows[0],avgx8);
	uint8x8_t mid1 = vcgt_u8(patch_rows[1],avgx8);
	test_mid = vsli_n_u8(test_mid,mid1,1);
	uint8x8_t mid2 = vcgt_u8(patch_rows[2],avgx8);
	test_mid = vsli_n_u8(test_mid,mid2,2);
	uint8x8_t mid3 = vcgt_u8(patch_rows[3],avgx8);
	test_mid = vsli_n_u8(test_mid,mid3,3);
	uint8x8_t mid4 = vcgt_u8(patch_rows[4],avgx8);
	test_mid = vsli_n_u8(test_mid,mid4,4);
	uint8x8_t mid5 = vcgt_u8(patch_rows[5],avgx8);
	test_mid = vsli_n_u8(test_mid,mid5,5);
	uint8x8_t mid6 = vcgt_u8(patch_rows[6],avgx8);
	test_mid = vsli_n_u8(test_mid,mid6,6);
	uint8x8_t mid7 = vcgt_u8(patch_rows[7],avgx8);
	test_mid = vsli_n_u8(test_mid,mid7,7);

	uint8x8_t test_high = vcgt_u8(patch_rows[0],avgpsigmax8);
	uint8x8_t high1 = vcgt_u8(patch_rows[1],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high1,1);
	uint8x8_t high2 = vcgt_u8(patch_rows[2],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high2,2);
	uint8x8_t high3 = vcgt_u8(patch_rows[3],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high3,3);
	uint8x8_t high4 = vcgt_u8(patch_rows[4],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high4,4);
	uint8x8_t high5 = vcgt_u8(patch_rows[5],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high5,5);
	uint8x8_t high6 = vcgt_u8(patch_rows[6],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high6,6);
	uint8x8_t high7 = vcgt_u8(patch_rows[7],avgpsigmax8);
	test_high = vsli_n_u8(test_high,high7,7);

	my_patch[0] = vmvn_u8(test_low);
	my_patch[1] = vbic_u8(test_low, test_mid);
	my_patch[2] = vbic_u8(test_mid, test_high);
	my_patch[3] = test_high;

//		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG,
//				"patch pattern = \n %016llX \n %016llX \n %016llX \n %016llX", my_patch[0], my_patch[1], my_patch[2], my_patch[3] );

}


// Next two functions are NON NEON method of counting error bits between two descriptors
//	static inline unsigned int bitcount64(unsigned long long int v){
//		v = v - ((v >> 1) & ~(unsigned long long int)0/3);                           // temp
//		v = (v & ~(unsigned long long int)0/15*3) + ((v >> 2) & ~(unsigned long long int)0/15*3);      // temp
//		v = (v + (v >> 4)) & ~(unsigned long long int)0/255*15;                      // temp
//		unsigned long long int c = (v * (~(unsigned long long int)0/255)) >> (sizeof(v) - 1) * CHAR_BIT; // count
//		return c;
//	}
//
//	inline unsigned int error(const Descriptor& fp){
//		const unsigned long long int* d1 = (unsigned long long int*) my_patch;
//		const unsigned long long int* d2 = (unsigned long long int*) fp.my_patch;
//
//		unsigned int err_count=0;
//		for(int i=0; i<4; i++){
//			unsigned long long int diff = d1[i] & (~d2[i]);
//			err_count += bitcount64(diff);
//		}
//		return err_count;
//	}


#endif
