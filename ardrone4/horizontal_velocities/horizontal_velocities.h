#ifndef _HORIZONTAL_VELOCITIES_H
#define _HORIZONTAL_VELOCITIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../attitude/attitude.h"


struct horizontal_velocities_struct
{
    //horizontal speed estimate
    float xv; // speed on x axis in m/s, front moving forwards is positive
    float yv; // speed on y axis in m/s, side moving left is positive 
    double dt; // time since last horizontal speed estimate update in sec;
    unsigned long seqNum;
    
};

int horizontal_velocities_init(struct horizontal_velocities_struct *hv);

/** @param att a up-to-date attitute struct */
void horizontal_velocities_getSample(struct horizontal_velocities_struct *hv, struct att_struct *att);

void horizontal_velocities_print(struct horizontal_velocities_struct *hv, double xpos, double ypos, double h);

void horizontal_velocities_close();

#ifdef __cplusplus
}  
#endif

#endif
