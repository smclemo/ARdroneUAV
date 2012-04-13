/*
 * NavDataContainer is a simple struct to store information from the
 * incoming set of NavData.  This allows global access to the
 * following NavData information.
 */

#ifndef _NAVDATACONTAINER_H_
#define _NAVDATACONTAINER_H_

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <strings.h>
#include <string.h>
//#include <time.h>
#include <sys/time.h>

#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#ifdef __cplusplus
#include <iostream>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct NavDataContainer
{
    // From quadrotor
    int altitude;
    float vx;
    float vy;
    float vz;
    float roll;
    float pitch;
    float yaw;
    float acc_x;
    float acc_y;
    float acc_z;

    int controlState;
    int vbat;
    
    float vphi_trim;
    float vtheta_trim;
    uint vstate;
    int vmisc;
    float vdelta_phi;
    float vdelta_theta;
    float vdelta_psi;

    uint vbat_raw;

    int ref_theta;
    int ref_phi;
    int ref_theta_I;
    int ref_phi_I;
    int ref_pitch;
    int ref_roll;
    int ref_yaw;
    int ref_psi;

    int rc_ref_pitch;
    int rc_ref_roll;
    int rc_ref_yaw;
    int rc_ref_gaz;
    int rc_ref_ag;

    float euler_theta;
    float euler_phi;

    uint pwm_motor1;
    uint pwm_motor2;
    uint pwm_motor3;
    uint pwm_motor4;
    uint pwm_sat_motor1;
    uint pwm_sat_motor2;
    uint pwm_sat_motor3;
    uint pwm_sat_motor4;
    int pwm_u_pitch;
    int pwm_u_roll;
    int pwm_u_yaw;
    float pwm_yaw_u_I;
    int pwm_u_pitch_planif;
    int pwm_u_roll_planif;
    int pwm_u_yaw_planif;
    uint pwm_current_motor1;
    uint pwm_current_motor2;
    uint pwm_current_motor3;
    uint pwm_current_motor4;

    float gyros_offsetx;
    float gyros_offsety;
    float gyros_offsetz;

    float trim_angular_rates;
    float trim_theta;
    float trim_phi;

#ifdef __cplusplus
    void printHeader(std::ostream & os) const;
    void print(std::ostream & os) const;
  int getPrintCount();
#endif

};

void printSomething();
void initializeGlobalNavData();
void saveNavData(const navdata_unpacked_t* const navdata);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
    std::ostream& operator << (std::ostream& os, const NavDataContainer& nd);
#endif


#endif
