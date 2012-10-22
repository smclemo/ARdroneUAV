#ifndef _OBJECT_DETECT_H
#define _OBJECT_DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../attitude/attitude.h"


struct object_detect_struct
{
    //horizontal speed estimate
    int locX; // x-position in frame of blob (pixel index)
    int locY; // y-position in frame of blob (pixel index)
    double dt; // time since last position estimate update in sec;
    unsigned long seqNum;
};

struct point {
	int x;
	int y;
};

struct point findBlob(struct img_struct* frame, unsigned char Ymin, unsigned char Ymax, unsigned char Umin, unsigned char Umax, unsigned char Vmin, unsigned char Vmax);

struct img_struct* compress_frame(struct img_struct* input_frame);

int object_detect_init(struct object_detect_struct *od);

/** @param att a up-to-date attitute struct */
void object_detect_getSample(struct object_detect_struct *od);

void object_detect_print(struct object_detect_struct *od, double xpos, double ypos);

void object_detect_close();

#ifdef __cplusplus
}  
#endif

#endif
