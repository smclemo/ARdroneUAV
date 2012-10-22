/*
    navboard.c - AR.Drone navboard driver

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
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>  //exit()
#include <math.h>

#include "navboard.h"
#include "../gpio/gpio.h"
#include "../util/util.h"


#define GPIO_NAVBOARD 132
#define READ_SIZE 58

#define NUMBER_SAMPLES_TRIM 200


#define GEE 9.81

#define GYRO_RAW_MAX 32767.0
#define GYRO_VAL_MAX_RADIANS_PER_SEC DEG2RAD(2000.0)

int nav_fd;

float accs_offset[]                    = { 2048, 2048, 2048 };
const float accs_gains[]               = { 512/GEE, 512/GEE, 512/GEE }; // 512 units <-> 1 gee
float gyros_offset[]                   = { 0,0,0 };
const float gyros_gains[]              = { GYRO_VAL_MAX_RADIANS_PER_SEC/GYRO_RAW_MAX,GYRO_VAL_MAX_RADIANS_PER_SEC/GYRO_RAW_MAX,GYRO_VAL_MAX_RADIANS_PER_SEC/GYRO_RAW_MAX }; 
float mag_offset[]                     = { 0,0,0 };
const float mag_gains[]                = { 1.0, 1.0, 1.0  }; 


int safe_read(int fd, void *target, int bytesToRead)
{
    int bytesRead=0;
    char * targetC=(char *)target;
    while (bytesRead !=bytesToRead) {
        int bytesRemaining=bytesToRead-bytesRead;
        int n=read(fd,targetC+bytesRead,bytesRemaining);
        if(n < 0) return n;
        bytesRead+=n;
    }
    return bytesRead;
}

int nav_GetSample(struct nav_struct* nav)
{
	int n;
	u16 size;
	n = safe_read(nav_fd, &size, 2);
	if(n!=2) { 
		//printf("Only read %d bytes for size info\n",n);
		return 1; //no packet received
	}
	if(size!=READ_SIZE) {
		printf("nav_size is %d, but expected %d\n",size,READ_SIZE);
		return 2; //size incorrect
	}
        
	n=safe_read(nav_fd,nav,READ_SIZE);
	if(n!=READ_SIZE) { 
		printf("Only read %d bytes for nav struct\n",n);
		return 3; //no packet received
	}
        
	//check data is valid
	u16 checksum=nav->seq
		+nav->acc[0]
		+nav->acc[1]
		+nav->acc[2]
		+nav->gyro[0]
		+nav->gyro[1]
		+nav->gyro[2]
		+nav->mag[0]
		+nav->mag[1]
		+nav->mag[2]
		+nav->unk1
		+nav->unk2
		+nav->unk3
		+nav->unk4
		+nav->unk5
		+nav->unk6
		+nav->unk7
		+nav->us_initialized
		+nav->us_sum_echo_1
		+nav->us_sum_echo_2
		+nav->us_echo
		+nav->us_echo_start
		+nav->us_echo_end
		+nav->us_association_echo
		+nav->us_distance_echo
		+nav->us_courbe_temps
		+nav->us_courbe_valeur
		+nav->us_number_echo; 
	
	if(nav->checksum!=checksum) 
	{
		printf("Checksum failed.\n");
		return 3; //checksum incorrect
	}
	
	//store timestamp
	double ts_prev = nav->ts;
	nav->ts = util_timestamp();
	nav->dt = nav->ts - ts_prev;
	
	//store converted sensor data in nav structure. 
	nav->ax = (((float)nav->acc[0]) - accs_offset[0]) / accs_gains[0];
	nav->ay = (((float)nav->acc[1]) - accs_offset[1]) / accs_gains[1];
	nav->az = (((float)nav->acc[2]) - accs_offset[2]) / accs_gains[2];
	
	
	nav->gx = (((float)nav->gyro[0]) - gyros_offset[0]) * gyros_gains[0];
	nav->gy = (((float)nav->gyro[1]) - gyros_offset[1]) * gyros_gains[1];
	nav->gz = (((float)nav->gyro[2]) - gyros_offset[2]) * gyros_gains[2];	
	
	nav->mag_x = (((float)nav->mag[0]) - mag_offset[0]) * mag_gains[0];
	nav->mag_y = (((float)nav->mag[1]) - mag_offset[1]) * mag_gains[1];
	nav->mag_z = (((float)nav->mag[2]) - mag_offset[2]) * mag_gains[2];	
	
	
	nav->h  = (float)((nav->us_echo&0x7fff)) * 0.000340;
    nav->h_meas = nav->us_echo >> 15;
	
	return 0;
}

void nav_Print(struct nav_struct* nav) 
{

	printf("RAW seq=%d a=%5d,%5d,%5d g=%5d,%5d,%5d unk=%5d,%5d h=%5d \nmag=%5d,%5d,%5d\n"
		,nav->seq
		,nav->acc[0],nav->acc[1],nav->acc[2]
		,nav->gyro[0],nav->gyro[1],nav->gyro[2]
		,nav->unk1
		,nav->unk2
		,nav->us_echo
		,nav->mag[0],nav->mag[1],nav->mag[2]
	);
	
	printf("%d a=%6.3f,%6.3f,%6.3f m/s² g=%+4.2f,%+4.2f,%+4.2fdeg/s h=%3.0fm dt=%2.0fms\n\n"
		,nav->seq
		,nav->ax,nav->ay,nav->az
		,RAD2DEG(nav->gx),RAD2DEG(nav->gy),RAD2DEG(nav->gz)
		,nav->h
		,nav->dt*1000
	);
}


//calibrate offsets. Drone has to be horizontal and stationary
//TODO add timeout
//returns 0 on success
int nav_FlatTrim() 
{
	accs_offset[0]=2048;
	accs_offset[1]=2048;
	accs_offset[2]=2048;
	gyros_offset[0]=0;
	gyros_offset[1]=0;
	gyros_offset[2]=0;
	//printf("nav_Calibrate bypassed\n");
	//return 0;

	struct nav_struct nav;
	int n_samples=NUMBER_SAMPLES_TRIM;
	int n=0; //number of samples
	float x1[6],x2[6]; //sum and sqr sum
	float avg[6],std[6]; //average and standard deviation
	int i;
		
	//zero sums
	for(i=0;i<6;i++) {x1[i]=0;x2[i]=0;}
	
	//collect n_samples samples 
	while(n<n_samples) {
		int retries=1;
		while(retries<100) {
			int rc = nav_GetSample(&nav);
			if(rc==0) break;
			retries++;
			printf("nav_Calibrate: retry=%d, code=%d\r\n",retries,rc); 
		}
		n++,
		x1[0]+=(float)nav.acc[0];
		x2[0]+=(float)nav.acc[0]*(float)nav.acc[0];
		x1[1]+=(float)nav.acc[1];
		x2[1]+=(float)nav.acc[1]*(float)nav.acc[1];
		x1[2]+=(float)nav.acc[2];
		x2[2]+=(float)nav.acc[2]*(float)nav.acc[2];
		x1[3]+=(float)nav.gyro[0];
		x2[3]+=(float)nav.gyro[0]*(float)nav.gyro[0];
		x1[4]+=(float)nav.gyro[1];
		x2[4]+=(float)nav.gyro[1]*(float)nav.gyro[1];
		x1[5]+=(float)nav.gyro[2];
		x2[5]+=(float)nav.gyro[2]*(float)nav.gyro[2];
	}
	
	//calc avg and standard deviation
	for(i=0;i<6;i++) {
	  avg[i]=x1[i]/n;
	  std[i]=(x2[i]-x1[i]*x1[i]/n)/(n-1);
	  if(std[i]<=0) std[i]=0; else std[i]=sqrt(std[i]); //handle rounding errors
	}
	
	//validate
	for(i=0;i<3;i++) {
	    if(std[i]>10) {
	        printf("Std deviation of accel channel %d is too large (%f)\n",i,std[i]);
	        return 1+i; //validate accs
            }
        }

        int tol=1200;
	if(avg[0]<2048-tol || avg[0]>2048+tol) {printf("nav_Calibrate: ax_avg out of tolerance: %f\r\n",avg[0]); return 10;}
	if(avg[1]<2048-tol || avg[1]>2048+tol) {printf("nav_Calibrate: ay_avg out of tolerance: %f\r\n",avg[1]); return 11;}
	
	float az_min=2048+accs_gains[2]*GEE-tol;
	float az_max=2048+accs_gains[2]*GEE+tol;
	
	if(avg[2]<az_min || avg[2]>az_max) {printf("nav_Calibrate: az_avg out of tolerance: %f < %f < %f \r\n",az_min,avg[2],az_max); return 12;}
	
	//set offsets
	accs_offset[0]=avg[0];
	accs_offset[1]=avg[1];
	accs_offset[2]=avg[2]-accs_gains[2]*GEE; //1 gee  

	printf("nav_Calibrate: accs_offset=%f,%f,%f\n",accs_offset[0],accs_offset[1],accs_offset[2]);

	for(i=3;i<6;i++) {
	    if(std[i]>10) {
    	        printf("Std deviation of gyro channel %d is too large (%f)\n",i,std[i]);
    	        return 1+i; //validate gyros
            }
        }

	gyros_offset[0]=avg[3];
	gyros_offset[1]=avg[4];
	gyros_offset[2]=avg[5];
	
	printf("nav_Calibrate: gyros_offset=%f,%f,%f\n",gyros_offset[0],gyros_offset[1],gyros_offset[2]);
	
	return 0;	
}

int nav_Init(struct nav_struct* nav) {
	//open nav port
	//stty -F /dev/ttyPA2 460800 -parenb -parodd cs8 -hupcl -cstopb cread clocal -crtscts 
	//-ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff -iuclc -ixany -imaxbel 
	//-opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten 
	//-echo echoe echok -echonl -noflsh -xcase -tostop -echoprt echoctl echoke

	nav_fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY);
	if (nav_fd == -1)
	{
		perror("nav_Init: Unable to open /dev/ttyO1 - ");
		return 101;
	} 
	sleep(1);
	fcntl(nav_fd, F_SETFL, 0); //read calls are non blocking
	//set port options
	struct termios options;
	//Get the current options for the port
	tcgetattr(nav_fd, &options);
	//Set the baud rates to 460800
	cfsetispeed(&options, B460800);
	cfsetospeed(&options, B460800);
/*Control Options  (options.c_cflag)
B0 0 baud (drop DTR) 
B50 50 baud 
B75 75 baud 
B110 110 baud 
B134 134.5 baud 
B150 150 baud 
B200 200 baud 
B300 300 baud 
B600 600 baud 
B1200 1200 baud 
B1800 1800 baud 
B2400 2400 baud 
B4800 4800 baud 
B9600 9600 baud 
B19200 19200 baud 
B38400 38400 baud 
B57600 57,600 baud 
B76800 76,800 baud 
B115200 115,200 baud 
EXTA External rate clock 
EXTB External rate clock 
CSIZE Bit mask for data bits 
CS5 5 data bits 
CS6 6 data bits 
CS7 7 data bits 
CS8 8 data bits 
CSTOPB 2 stop bits (1 otherwise) 
CREAD Enable receiver 
PARENB Enable parity bit 
PARODD Use odd parity instead of even 
HUPCL Hangup (drop DTR) on last close 
CLOCAL Local line - do not change "owner" of port 
LOBLK Block job control output 
CNEW_RTSCTS 
CRTSCTS Enable hardware flow control (not supported on all platforms) 
*/
	options.c_cflag |= (CLOCAL | CREAD); //Enable the receiver and set local mode
/*Input Options (options.c_iflag)
INPCK Enable parity check 
IGNPAR Ignore parity errors 
PARMRK Mark parity errors 
ISTRIP Strip parity bits 
IXON Enable software flow control (outgoing) 
IXOFF Enable software flow control (incoming) 
IXANY Allow any character to start flow again 
IGNBRK Ignore break condition 
BRKINT Send a SIGINT when a break condition is detected 
INLCR Map NL to CR 
IGNCR Ignore CR 
ICRNL Map CR to NL 
IUCLC Map uppercase to lowercase 
IMAXBEL Echo BEL on input line too long 
*/
	options.c_iflag = 0; //clear input options
/*Local Options (options.c_lflag)
ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals 
ICANON Enable canonical input (else raw) 
XCASE Map uppercase \lowercase (obsolete) 
ECHO Enable echoing of input characters 
ECHOE Echo erase character as BS-SP-BS 
ECHOK Echo NL after kill character 
ECHONL Echo NL 
NOFLSH Disable flushing of input buffers after interrupt or quit characters 
IEXTEN Enable extended functions 
ECHOCTL Echo control characters as ^char and delete as ~? 
ECHOPRT Echo erased character as character erased 
ECHOKE BS-SP-BS entire line on line kill 
FLUSHO Output being flushed 
PENDIN Retype pending input at next read or input char 
TOSTOP Send SIGTTOU for background output 
*/
	options.c_lflag=0; //clear local options
/*Output Options (options.c_oflag)
OPOST Postprocess output (not set = raw output) 
OLCUC Map lowercase to uppercase 
ONLCR Map NL to CR-NL 
OCRNL Map CR to NL 
NOCR No CR output at column 0 
ONLRET NL performs CR function 
OFILL Use fill characters for delay 
OFDEL Fill character is DEL 
NLDLY Mask for delay time needed between lines 
NL0 No delay for NLs 
NL1 Delay further output after newline for 100 milliseconds 
CRDLY Mask for delay time needed to return carriage to left column 
CR0 No delay for CRs 
CR1 Delay after CRs depending on current column position 
CR2 Delay 100 milliseconds after sending CRs 
CR3 Delay 150 milliseconds after sending CRs 
TABDLY Mask for delay time needed after TABs 
TAB0 No delay for TABs 
TAB1 Delay after TABs depending on current column position 
TAB2 Delay 100 milliseconds after sending TABs 
TAB3 Expand TAB characters to spaces 
BSDLY Mask for delay time needed after BSs 
BS0 No delay for BSs 
BS1 Delay 50 milliseconds after sending BSs 
VTDLY Mask for delay time needed after VTs 
VT0 No delay for VTs 
VT1 Delay 2 seconds after sending VTs 
FFDLY Mask for delay time needed after FFs 
FF0 No delay for FFs 
FF1 Delay 2 seconds after sending FFs 
*/
	options.c_oflag &= ~OPOST; //clear output options (raw output)
	//Set the new options for the port
	tcsetattr(nav_fd, TCSANOW, &options);
  
	//set /MCLR pin
	gpio_set(GPIO_NAVBOARD,1);
	
	//start acquisition
	u08 cmd=0x01;
	write(nav_fd,&cmd,1);
  
	//init nav structure	
    nav->ts = util_timestamp();
    nav->dt = 0;
	
	return 0;
}

void nav_Close()
{
	close(nav_fd);
}