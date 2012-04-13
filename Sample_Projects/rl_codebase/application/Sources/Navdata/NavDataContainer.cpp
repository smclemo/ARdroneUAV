#include <iostream>
//#include <ostream>
#include "NavDataContainer.hpp"

using namespace std;


void printSomething()
{
    cout << "Printed something from C++ :)" << endl;
}

NavDataContainer globalNavData;

void NavDataContainer::printHeader(ostream & os) const
{
    // From quadrotor
    os << " altitude roll pitch yaw vx vy vz acc_x acc_y acc_z";

    os << " controlState vbat "
       << "vphi_trim vtheta_trim vstate "
       << "vmisc vdelta_phi vdelta_theta "
       << "vdelta_psi "
       << "vbat_raw "
       << "ref_theta ref_phi ref_theta_I "
       << "ref_phi_I ref_pitch ref_roll "
       << "ref_yaw ref_psi "
       << "rc_ref_pitch rc_ref_roll rc_ref_yaw "
       << "rc_ref_gaz rc_ref_ag "
       << "euler_theta euler_phi "
       << "pwm_motor1 pwm_motor2 pwm_motor3 "
       << "pwm_motor4 pwm_sat_motor1 pwm_sat_motor2 "
       << "pwm_sat_motor3 pwm_sat_motor4 pwm_u_pitch "
       << "pwm_u_roll pwm_u_yaw pwm_yaw_u_I "
       << "pwm_u_pitch_planif pwm_u_roll_planif pwm_u_yaw_planif "
       << "pwm_current_motor1 pwm_current_motor2 "
       << "pwm_current_motor3 pwm_current_motor4 "
       << "gyros_offsetx gyros_offsety gyros_offsetz "
       << "trim_angular_rates trim_theta trim_phi ";
}

int NavDataContainer::getPrintCount()
{
  return 60; //Hard Coded, returns the number of numbers printed by NavDataContainer::print
}

void NavDataContainer::print(ostream & os) const
{
    // From quadrotor
    os << " " << altitude
       << " " << roll << " " << pitch << " " << yaw
       << " " << vx << " " << vy << " " << vz
       << " " << acc_x << " " << acc_y << " " << acc_z << " ";

    os << controlState << " " << vbat << " ";
    
    os << vphi_trim << " " << vtheta_trim << " " << vstate << " "
       << vmisc << " " << vdelta_phi << " " << vdelta_theta << " "
       << vdelta_psi << " ";

    os << vbat_raw << " ";

    os << ref_theta << " " << ref_phi << " " << ref_theta_I << " "
       << ref_phi_I << " " << ref_pitch << " " << ref_roll << " "
       << ref_yaw << " " << ref_psi << " ";

    os << rc_ref_pitch << " " << rc_ref_roll << " " << rc_ref_yaw << " "
       << rc_ref_gaz << " " << rc_ref_ag << " ";

    os << euler_theta << " " << euler_phi << " ";

    os << pwm_motor1 << " " << pwm_motor2 << " " << pwm_motor3 << " "
       << pwm_motor4 << " " << pwm_sat_motor1 << " " << pwm_sat_motor2 << " "
       << pwm_sat_motor3 << " " << pwm_sat_motor4 << " " << pwm_u_pitch << " "
       << pwm_u_roll << " " << pwm_u_yaw << " " << pwm_yaw_u_I << " "
       << pwm_u_pitch_planif << " " << pwm_u_roll_planif << " " << pwm_u_yaw_planif << " "
       << pwm_current_motor1 << " " << pwm_current_motor2 << " "
       << pwm_current_motor3 << " " << pwm_current_motor4 << " ";

    os << gyros_offsetx << " " << gyros_offsety << " " << gyros_offsetz << " ";

    os << trim_angular_rates << " " << trim_theta << " " << trim_phi;

}

void initializeGlobalNavData()
{

    globalNavData.altitude = 0;
    globalNavData.roll = -1e6;
    globalNavData.pitch = -1e6;
    globalNavData.yaw = -1e6;

};


void saveNavData(const navdata_unpacked_t* const navdata)
{
    
    globalNavData.altitude = navdata->navdata_altitude.altitude_raw;
    globalNavData.vx       = navdata->navdata_demo.vx;
    globalNavData.vy       = navdata->navdata_demo.vy;
    globalNavData.vz       = navdata->navdata_demo.vz;

    globalNavData.yaw      = navdata->navdata_demo.psi / 1000.0;
    globalNavData.pitch    = navdata->navdata_demo.theta / 1000.0;
    globalNavData.roll     = navdata->navdata_demo.phi / 1000.0;

    //printf("Psi:%9f Theta:%9f Phi:%9f\n", navdata->navdata_demo.psi, navdata->navdata_demo.theta, navdata->navdata_demo.phi);

    globalNavData.acc_x    = navdata->navdata_phys_measures.phys_accs[ACC_X] / 1000.0;
    globalNavData.acc_y    = navdata->navdata_phys_measures.phys_accs[ACC_Y] / 1000.0;
    globalNavData.acc_z    = navdata->navdata_phys_measures.phys_accs[ACC_Z] / 1000.0;
    
    globalNavData.controlState = navdata->navdata_demo.ctrl_state;
    globalNavData.vbat = navdata->navdata_demo.vbat_flying_percentage;
    
    globalNavData.vphi_trim = navdata->navdata_vision.vision_phi_trim;
    globalNavData.vtheta_trim = navdata->navdata_vision.vision_theta_trim;
    globalNavData.vstate = navdata->navdata_vision.vision_state;
    globalNavData.vmisc = navdata->navdata_vision.vision_misc;
    globalNavData.vdelta_phi = navdata->navdata_vision.delta_phi;
    globalNavData.vdelta_theta = navdata->navdata_vision.delta_theta;
    globalNavData.vdelta_psi = navdata->navdata_vision.delta_psi;

    globalNavData.vbat_raw = navdata->navdata_raw_measures.vbat_raw;

    globalNavData.ref_theta = navdata->navdata_references.ref_theta;
    globalNavData.ref_phi = navdata->navdata_references.ref_phi;
    globalNavData.ref_theta_I = navdata->navdata_references.ref_theta_I;
    globalNavData.ref_phi_I = navdata->navdata_references.ref_phi_I;
    globalNavData.ref_pitch = navdata->navdata_references.ref_pitch;
    globalNavData.ref_roll = navdata->navdata_references.ref_roll;
    globalNavData.ref_yaw = navdata->navdata_references.ref_yaw;
    globalNavData.ref_psi = navdata->navdata_references.ref_psi;

    globalNavData.rc_ref_pitch = navdata->navdata_rc_references.rc_ref_pitch;
    globalNavData.rc_ref_roll = navdata->navdata_rc_references.rc_ref_roll;
    globalNavData.rc_ref_yaw = navdata->navdata_rc_references.rc_ref_yaw;
    globalNavData.rc_ref_gaz = navdata->navdata_rc_references.rc_ref_gaz;
    globalNavData.rc_ref_ag = navdata->navdata_rc_references.rc_ref_ag;

    globalNavData.euler_theta = navdata->navdata_euler_angles.theta_a;
    globalNavData.euler_phi = navdata->navdata_euler_angles.phi_a;

    globalNavData.pwm_motor1 = (unsigned int) navdata->navdata_pwm.motor1;
    globalNavData.pwm_motor2 = (unsigned int) navdata->navdata_pwm.motor2;
    globalNavData.pwm_motor3 = (unsigned int) navdata->navdata_pwm.motor3;
    globalNavData.pwm_motor4 = (unsigned int) navdata->navdata_pwm.motor4;
    globalNavData.pwm_sat_motor1 = (unsigned int) navdata->navdata_pwm.sat_motor1;
    globalNavData.pwm_sat_motor2 = (unsigned int) navdata->navdata_pwm.sat_motor2;
    globalNavData.pwm_sat_motor3 = (unsigned int) navdata->navdata_pwm.sat_motor3;
    globalNavData.pwm_sat_motor4 = (unsigned int) navdata->navdata_pwm.sat_motor4;
    globalNavData.pwm_u_pitch = (int) navdata->navdata_pwm.u_pitch;
    globalNavData.pwm_u_roll = (int) navdata->navdata_pwm.u_roll;
    globalNavData.pwm_u_yaw = (int) navdata->navdata_pwm.u_yaw;
    globalNavData.pwm_yaw_u_I = navdata->navdata_pwm.yaw_u_I;
    globalNavData.pwm_u_pitch_planif = (int) navdata->navdata_pwm.u_pitch_planif;
    globalNavData.pwm_u_roll_planif = (int) navdata->navdata_pwm.u_roll_planif;
    globalNavData.pwm_u_yaw_planif = (int) navdata->navdata_pwm.u_yaw_planif;
    globalNavData.pwm_current_motor1 = (int) navdata->navdata_pwm.current_motor1;
    globalNavData.pwm_current_motor2 = (int) navdata->navdata_pwm.current_motor2;
    globalNavData.pwm_current_motor3 = (int) navdata->navdata_pwm.current_motor3;
    globalNavData.pwm_current_motor4 = (int) navdata->navdata_pwm.current_motor4;

    globalNavData.gyros_offsetx = navdata->navdata_gyros_offsets.offset_g[GYRO_X];
    globalNavData.gyros_offsety = navdata->navdata_gyros_offsets.offset_g[GYRO_Y];
    globalNavData.gyros_offsetz = navdata->navdata_gyros_offsets.offset_g[GYRO_Z];

    globalNavData.trim_angular_rates = navdata->navdata_trims.angular_rates_trim_r;
    globalNavData.trim_theta = navdata->navdata_trims.euler_angles_trim_theta;
    globalNavData.trim_phi = navdata->navdata_trims.euler_angles_trim_phi;

}

std::ostream& operator << (std::ostream& os, const NavDataContainer& nd)
{
    nd.print(os);
    return os; 
}
