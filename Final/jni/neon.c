#include "neon.h"
#include <stdlib.h>
#include "fast.h"

#include <arm_neon.h>



static const unsigned short int permissible_patterns[16]={
		0b1111111110000000,
		0b0111111111000000,
		0b0011111111100000,
		0b0001111111110000,
		0b0000111111111000,
		0b0000011111111100,
		0b0000001111111110,
		0b0000000111111111,
		0b1000000011111111,
		0b1100000001111111,
		0b1110000000111111,
		0b1111000000011111,
		0b1111100000001111,
		0b1111110000000111,
		0b1111111000000011,
		0b1111111100000001};


static byte bitpos[16]={
		1,2,4,8,16,32,64,128,1,2,4,8,16,32,64,128
};
uint8x16_t vbitpos;

// return bit pattern if other is above high or below low
inline unsigned int test_16_pixels(uint8x16_t low, uint8x16_t high, uint8x16_t other){
	unsigned int result=0;
	uint8x16_t test1, test2;

	test1 = vcgtq_u8(low,other); // test1 now holds in locations where low barrier test passed
	test1 = vandq_u8(test1,vbitpos); // test1 now holds 1,2,4,8,..,1,2,4,8,... in these locations

	test2 = vcgtq_u8(other,high); // test2 now holds 255 in locations where high barrier test passed
	test2 = vandq_u8(test2,vbitpos); // test2 now holds 1,2,4,8,..,1,2,4,8,... in these locations

	test1 = vpaddlq_u8(test1); //combine test1 in pairs in 16x8
	test2 = vpaddlq_u8(test2); //ditto test2

	uint16x8_t test = vsliq_n_u16(test1, test2, 8); // interleave test1 and test2 (test 2 in high bits of each 16 bit entry)

	test = vpaddlq_u16(test); // combine into fours
	test1 = vpaddlq_u32(test); // combine into eights

	result += vgetq_lane_u8(test1,8);
	result <<= 8;
	result += vgetq_lane_u8(test1,0);
	result <<=8;
	result += vgetq_lane_u8(test1,9);
	result <<= 8;
	result += vgetq_lane_u8(test1,1);


	return result;
}


// scan down every third row to check up and down for possible fast corner
xy* Tom_all_neon_fast9_detect(const byte* im, const int xsize, const int ysize, const int stride, const int b, int* ret_num_corners){
	int num_corners=0;
	xy* ret_corners;
	int rsize=512;
	// int pixel[16];

	ret_corners = (xy*)malloc(sizeof(xy)*rsize);
	// make_offsets(pixel, stride);

	uint8x16_t barr = vdupq_n_u8(b);
	vbitpos = vld1q_u8(bitpos);

	const unsigned int last_half_mask = 0b1111111111111111 >> 15-((xsize-4) % 16);
	const unsigned int last_mask = (last_half_mask << 16) + last_half_mask;

	int row_interleave, row, col;
	for(col=0; col<(xsize-3); col+=16){
		for(row_interleave=0; row_interleave<3; row_interleave++){

			const byte* p = im + (row_interleave)*stride + col;
			uint8x16_t pixels = vld1q_u8(p);
			uint8x16_t below = vld1q_u8(p+3*stride);
			uint8x16_t above;

			for(row=row_interleave+3; row < ysize-3; row+=3){
				p += 3*stride;
				above = pixels;
				pixels = below;
				below = vld1q_u8(p+3*stride);

				uint8x16_t low = vqsubq_u8(pixels,barr); // centre pixels minus barrier (saturating at 0)
				uint8x16_t high = vqaddq_u8(pixels,barr); // pixels now holds pixels plus barrier (saturating at 255)

				unsigned int test_0 = test_16_pixels(low,high,above);
				unsigned int test_8 = test_16_pixels(low,high,below);

				unsigned int possible = test_0 | test_8;

				if(col==0){
					possible &= 0b11111111111110001111111111111000;
				} else if (col+17>xsize) {
					possible &= last_mask;
				}

				if(possible==0) continue; // tested 0 and 8

				uint8x16_t other;

				other = vld1q_u8(p+3);
				unsigned int test_4 = test_16_pixels(low,high,other);

				other = vld1q_u8(p-3);
				unsigned int test_12 = test_16_pixels(low,high,other);

				possible &= (test_4 | test_12);

				if(possible==0) continue;  // tested 4 and 12

				other = vld1q_u8(p-2*stride+2);
				unsigned int test_2 = test_16_pixels(low,high,other);

				unsigned int q_0_4 = test_0 & test_2 & test_4;
				unsigned int q_4_8 = test_4 & test_8;
				unsigned int q_8_12 = test_8 & test_12;
				unsigned int q_12_0 = test_12 & test_0;

				possible &= (q_0_4 | q_4_8 | q_8_12 | q_12_0 );

				if(possible==0) continue; // tested 2


				other = vld1q_u8(p+2*stride-2);
				unsigned int test_10 = test_16_pixels(low,high,other);
				possible &= (test_2 | test_10);
				q_8_12 &= test_10;
				possible &= (q_0_4 | q_4_8 | q_8_12 | q_12_0 );

				if(possible==0) continue; // tested 10

				other = vld1q_u8(p+2*stride+2);
				unsigned int test_6 = test_16_pixels(low,high,other);
				q_4_8 &= test_6;
				possible &= (q_0_4 | q_4_8 | q_8_12 | q_12_0 );

				if(possible==0) continue; // tested 6

				other = vld1q_u8(p-2*stride-2);
				unsigned int test_14 = test_16_pixels(low,high,other);
				possible &= (test_6 | test_14);
				q_12_0 &= test_14;
				possible &= (q_0_4 | q_4_8 | q_8_12 | q_12_0 );

				if(possible==0) continue; // tested 14

				other = vld1q_u8(p-3*stride+1);
				unsigned int test_1 = test_16_pixels(low,high,other);
				possible &= (test_1 | q_8_12 | (q_4_8 & test_10));
				q_0_4 &= test_1;

				if(possible==0) continue; // tested 1

				other = vld1q_u8(p+3*stride-1);
				unsigned int test_9 = test_16_pixels(low,high,other);
				possible &= (test_1 | test_9);
				possible &= (test_9 | q_0_4 | (q_12_0 & test_2));
				q_8_12 &= test_9;

				if(possible==0) continue; // tested 9

				other = vld1q_u8(p+stride+3);
				unsigned int test_5 = test_16_pixels(low,high,other);
				possible &= (test_5 | (q_8_12 & test_14) | (q_12_0 & test_1 & (test_2 | (test_9 & test_10))));
				q_4_8 &= test_5;

				if(possible==0) continue; // tested 5

				other = vld1q_u8(p-stride-3);
				unsigned int test_13 = test_16_pixels(low,high,other);
				possible &= (test_13 | (q_4_8 & test_9 & (test_10 | (test_1 & test_2))));
				q_12_0 &= test_13;

				if(possible==0) continue; // tested 13

				other = vld1q_u8(p-stride+3);
				unsigned int test_3 = test_16_pixels(low,high,other);
				possible &= (test_3
						| (q_8_12 & ((test_6 & test_5 & (test_4 | test_13)) | test_14))
						| (q_12_0 & test_10 & test_1 & (test_9 | test_2)));
				q_0_4 &= test_3;

				if(possible==0) continue; // tested 13

				other = vld1q_u8(p+stride-3);
				unsigned int test_11 = test_16_pixels(low,high,other);
				possible &= (test_11
						| (q_0_4 & (( test_13 & test_14 & (test_12 | test_5)) | (test_5 & test_6)))
						| (q_4_8 & test_2 & test_3 & test_9 & (test_1 | test_10)));
				q_8_12 &= test_11;

				if(possible==0) continue; // tested 11

				other = vld1q_u8(p+3*stride+1);
				unsigned int test_7 = test_16_pixels(low,high,other);
				possible &= (test_7
						| (q_12_0 & (( q_8_12| q_0_4 | (test_11 & test_1 & ((test_10 & (test_9 | test_2)) | (test_2 & test_3))))))
						| (q_0_4 & test_14 & test_5 & (test_13 | test_6)));
				q_4_8 &= test_7;

				if(possible==0) continue; // tested 7

				other = vld1q_u8(p-3*stride-1);
				unsigned int test_15 = test_16_pixels(low,high,other);
				possible &= (test_15
						| (q_4_8 & (q_0_4 | q_8_12 | (test_13 & test_9 & ((test_2 & (test_1 | test_10)) | (test_10 & test_11)))))
						| (q_8_12 & (test_13 & test_7 & test_6 & (test_5 | test_14))));

				if(possible==0) continue; // tested 7

				int x;
				unsigned int testbits= 0b00000000000000010000000000000001;
				for(x=0; x<16; x++){
					if(possible & testbits){
						// if(brute_force_fast9_check(p+x, pixel, b)){
						if(num_corners == rsize) {
							rsize*=2;
							ret_corners = (xy*)realloc(ret_corners, sizeof(xy)*rsize);
						}
						ret_corners[num_corners].x = col+x;
						ret_corners[num_corners].y = row;
						num_corners++;
						// }
					}
					testbits <<=1;
				}
			}
		}
	}
	*ret_num_corners = num_corners;
	return ret_corners;
}




int check(unsigned short int v){
	int i;
	for(i=0; i<16; i++){
		const short int p=permissible_patterns[i];
		if((v&p) == p){
			return 1;
		}
	}
	return 0;
}





int brute_force_fast9_check(byte* p, int pixel[], int b){
	int uthresh = *p + b;
	int lthresh = *p - b;

	unsigned short int hi=0;
	unsigned short int lo=0;
	unsigned short int bit=1;
	int i;
	for(i=0; i<16; i++){
		byte val = *(p+pixel[i]);
		if(val > uthresh){
			hi|=bit;
		}
		if(val < lthresh){
			lo|=bit;
		}
		bit <<= 1;
	}
	return (check(hi) || check(lo));
}

