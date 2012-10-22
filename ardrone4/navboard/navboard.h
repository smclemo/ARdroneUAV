/*
    navboard.h - AR.Drone navboard driver

    Copyright (C) 2011 Hugo Perquin - http://blog.perquin.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/
#ifndef _NAVBOARD_H
#define _NAVBOARD_H
#include "../util/type.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// nav_struct
///////////////////////////////////////////////////////////////////////////////
/*
Microchip PIC24HJ16GP304 
12 bit ADC, Vref=3.3V -> 0.806mV/lsb

IDG-500 (Dual-axis gyroscope) 
X-/Y-Out Pins: 500°/s full scale range, 2.0mV/°/s sensitivity
X/Y4.5Out Pins: 110°/s full scale range, 9.1mV/°/s sensitivity
Vref = 1350±50mV
Temperature Sensor: Range -20 to +85°C, Sensitivity 4mV/°C, Offset 1.25V at room temperature

Epson XV-3500CB (Z-axis gyroscope) 
0.67mV/deg/s  Vref=1350+/-30mV

Bosch BMA150 (3-axis accelerometer) 
i2c interface
Acceleration Sensor: 10bit resolution, range -2 to 2G 
Temperature Sensor: 8bit, 0.5°C/lsb, range -30 to 97.5°C
*/
struct nav_struct
{
//	u16 size;                // Size of the following data (always 0x2C)  // not read into struct anymore
	u16 seq;                 // Sequence number, increases every update 
	u16 acc[3];              // Raw data (10-bit) of the accelerometers multiplied by 4 
	s16 gyro[3];             // Raw data for the gyros, 16-bit gyro values
	u16 unk1;    
    u16 unk2;
	u16 us_echo;             // bit15=1 echo pulse transmitted, bit14-0 first echo. Value 30 = 1cm. min value: 784 = 26cm
	u16 us_echo_start;       // Array with starts of echos (8 array values @ 25Hz, 9 values @ 22.22Hz)
	u16 us_echo_end;         // Array with ends of echos   (8 array values @ 25Hz, 9 values @ 22.22Hz)
	u16 us_association_echo; // Ultrasonic parameter -- echo number starting with 0. max value 3758. examples: 0,1,2,3,4,5,6,7  ; 0,1,2,3,4,86,6,9
	u16 us_distance_echo;    // Ultrasonic parameter -- no clear pattern
	u16 us_courbe_temps;     // Ultrasonic parameter -- counts up from 0 to approx 24346 in 192 sample cycles of which 12 cylces have value 0
	u16 us_courbe_valeur;    // Ultrasonic parameter -- value between 0 and 4000, no clear pattern. 192 sample cycles of which 12 cylces have value 0
	u16 unk3;                
	u16 us_number_echo;      // Number of selected echo for height measurement (1,2,3)
	u16 us_sum_echo_1;
	u16 us_sum_echo_2;
	u16 unk4;                // unknown +0x26 Ultrasonic parameter -- no clear pattern
	u16 us_initialized;      // always 1
	u16 unk5;
	u16 unk6;
	u16 unk7;
	s16 mag[3];
	u16 checksum; 		 // Checksum = sum of all values except checksum 
	

	
	//end of data received from nav board  
	double ts; //timestamp in seconds with microsecond resolution
	float dt; //time since last sample
	float ax;   // acceleration x-axis in [G] front facing up is positive         
	float ay;   // acceleration y-axis in [G] left facing up is positive                
	float az;   // acceleration z-axis in [G] top facing up is positive             
	float gx;   // gyro value x-axis in [deg/sec] right turn, i.e. roll right is positive           
	float gy;   // gyro value y-axis in [deg/sec] right turn, i.e. pirch down is positive                     
	float gz;   // gyro value z-axis in [deg/sec] right turn, i.e. yaw left is positive                           
	float h;    // height above ground in [cm] 
        char h_meas;// 1 if this is a new h measurement, 0 otherwise
        
        float mag_x;
        float mag_y;
        float mag_z;
        
};

int nav_Init(struct nav_struct* nav);
int nav_FlatTrim();
int nav_GetSample(struct nav_struct* nav);
void nav_Print(struct nav_struct* nav);
void nav_Close();

#ifdef __cplusplus
}
#endif


#endif