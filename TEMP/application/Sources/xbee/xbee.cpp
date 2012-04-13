#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <pthread.h>

#include "xbee.hpp"

unsigned char xbee_get_next_char(Xbee *xbee)
{
  unsigned char buff;
  read(xbee->fd, &buff, 1);
  //printf("%02X ", buff);
  return buff;
}

#define XBEENEXT xbee_get_next_char(parent)

//This is the static function to be run as a thread.  It is passed 'this'.
void *xbee_thread(void *parm)
{
	Xbee *parent = (Xbee *)parm;
	int temp1=0, temp2=0, temp3=0, temp0=0;
	//int prev1=0, prev2=0, prev3=0, prev0=0;
	
	printf("XBee Thread Successfully Started.\n");
	
	while(!(parent->done))
	{
	  //printf("x");
	  //if header found
    if(XBEENEXT == 0x7E)
    { 
      //printf("\n");
      //printf("7E ");
      //then shift through the buffer to the address
      if(XBEENEXT != 0x00) continue; //Size check MSB
      if(XBEENEXT != 0x10) continue; //Size check LSB
      if(XBEENEXT != 0x83) continue; //Content Flag (16-bit adc)
      //XBEENEXT; XBEENEXT; XBEENEXT;
      //printf("%02X %02X %02X ", XBEENEXT, XBEENEXT, XBEENEXT);
      //check that it's from the proper source:
      if(((XBEENEXT << 8) | XBEENEXT) == XBEESENDERADDRESS)
      {
        //printf("%04X ", XBEESENDERADDRESS);
        //then shift through the buffer to the data
        XBEENEXT; XBEENEXT; XBEENEXT; XBEENEXT; XBEENEXT;
        //printf("%02X ",  XBEENEXT);
        //printf("%02X ",  XBEENEXT);
       // printf("%02X ",  XBEENEXT);
       // printf("%02X ",  XBEENEXT);
       // printf("%02X ",  XBEENEXT);
        //and read data:
        temp0 = ((XBEENEXT) << 8) | (XBEENEXT);
        temp1 = ((XBEENEXT) << 8) | (XBEENEXT);
        temp2 = ((XBEENEXT) << 8) | (XBEENEXT);
        temp3 = ((XBEENEXT) << 8) | (XBEENEXT);
        //printf("%04X %04X %04X %04X ", temp0, temp1, temp2, temp3);
        //printf("Raw - 1:%4d 2:%4d 3:%4d 4:%4d\n", temp0, temp1, temp2, temp3); //****
      }
      else
        continue;
      //printf("\n");
    }
    //Apply averaging filter.  Note: sometimes the serial acts up, and the xbee only has a 10-bit ADC, so thats why there's an if check.
    /* old 
    if(temp0 < 1023) parent->adc0 = ((parent->adc0 * parent->avgRate) + temp0) / (parent->avgRate + 1);
    if(temp1 < 1023) parent->adc1 = ((parent->adc1 * parent->avgRate) + temp1) / (parent->avgRate + 1);
    if(temp2 < 1023) parent->adc2 = ((parent->adc2 * parent->avgRate) + temp2) / (parent->avgRate + 1);
    if(temp3 < 1023) parent->adc3 = ((parent->adc3 * parent->avgRate) + temp3) / (parent->avgRate + 1); */
    
    //insert into median filter
    if(temp0 < 1023 && temp0 > 0) {parent->adc0->pushValue(temp0); }//printf("%d %f\n", temp0, parent->adc0->getMedian()); }
    if(temp1 < 1023 && temp1 > 0) 
    {
      parent->adc1->pushValue(temp1);
    }
    if(temp2 < 1023 && temp2 > 0) 
    {
      parent->adc2->pushValue(temp2);
    }
    if(temp3 < 1023 && temp3 > 0) parent->adc3->pushValue(temp3);
  } 
  
  close(parent->fd);
	return 0;
}

//constructor
Xbee::Xbee(int medianFilterValue, double smoothingRange)
{
  //avgRate = DataAverageRate;
  //if(avgRate < 0) avgRate = 0;

  deriv0 = 0;
  deriv1 = 0;
  deriv2 = 0;
  deriv3 = 0;

  adc0 = new SmoothingMedianFilter(medianFilterValue, smoothingRange, 0.0);
  adc1 = new SmoothingMedianFilter(medianFilterValue, smoothingRange, 0.0);
  adc2 = new SmoothingMedianFilter(medianFilterValue, smoothingRange, 0.0);
  adc3 = new SmoothingMedianFilter(medianFilterValue, smoothingRange, 0.0);
  
  fd = open(XBEESERIALPORT, O_RDWR | O_NOCTTY);
  if (fd == -1)
  {
    /*
    * Could not open the port.
    */

    printf("\n      ERROR: open_port: Unable to open %s, xbee sensors will not be available.", XBEESERIALPORT);
    printf("\n      This can be ingored if no xbee is being used.\n\n");
  }
  else
  {
    fcntl(fd, F_SETFL, 0);
    
    struct termios options;

    /*
     * Get the current options for the port...
     */

    tcgetattr(fd, &options);

    /*
     * Set the baud rates
     */

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    /*
     * Enable the receiver and set local mode...
     */

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS; //disable hardware flow control
    
    options.c_iflag &= ~(INPCK | ISTRIP); //disable parity
    options.c_iflag &= ~(IXON | IXOFF | IXANY); //disable software flow control
    options.c_iflag &= ~(IGNPAR | IGNBRK | IGNCR); //don't ignore special characters
    
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //enable raw input
    
    options.c_cc[VTIME] = 10;

    /*
     * Set the new options for the port...
     */

    tcsetattr(fd, TCSAFLUSH, &options);
    
    done = false;
    //Start thread
    threadid = pthread_create( &xbeethread, NULL, xbee_thread, (void*) this);
  }
}

Xbee::~Xbee()
{
  done = true;
}

int Xbee::getLeftDist()
{
  return adc1->getMedian();
}

int Xbee::getRightDist()
{
  return adc0->getMedian();
}

int Xbee::getFrontDist()
{
  return adc2->getMedian();
}

int Xbee::getFrontDeriv() //internal (private)
{
  int diff = adc2->getMedian() - deriv2; //deriv2 != deriv2s; deriv2s is smoothed derivative; deriv2 is a tool used to find the derivative (last median)
  deriv2 = adc2->getMedian(); 
  return diff;
}

int Xbee::updateFrontDeriv()
{
  deriv2s = (deriv2s*2) + getFrontDeriv();
  deriv2s /= 3;
  return deriv2s;
}

int Xbee::lastFrontDeriv()
{
  return deriv2s;
}



///////////////////////////////////////////////////////////////////
/////////////Delete Below
/*
#define XBEEPACKETSIZE 20
#define XBEEADDRESS 0xD00D
unsigned int adc0;
unsigned int adc1;
unsigned int adc2;
unsigned int adc3;

int processXbeeData(unsigned char* buff, int bufflength)
{
  int i = 0, j = 0;
  
  while(bufflength - i > XBEEPACKETSIZE)
  {
    //if header found
    if(buff[i] == 0x7E)
    { 
      //then shift through the buffer to the address
      i += 4;
      //check that it's from the proper source:
      if(((buff[i] << 8) | (buff[i+1])) == XBEEADDRESS)
      {
        //then shift through the buffer to the data
        i+=7;
        //printf("Packet: ");
        //for(j = 0; j < XBEEPACKETSIZE; j++)
        //  printf("%02X ", buff[j+i-11]);
        //printf("\n");
        //and read:
        adc0 = ((buff[i]) << 8) | (buff[i+1]); i+=2;
        adc1 = ((buff[i]) << 8) | (buff[i+1]); i+=2;
        adc2 = ((buff[i]) << 8) | (buff[i+1]); i+=2;
        adc3 = ((buff[i]) << 8) | (buff[i+1]); i+=2;
        return 1;
      }
    }
    i++;
  }
  return 0;
}  

int
//open_port(void)
main()
{

    
    unsigned char buff[XBEEPACKETSIZE*2];
    int i;
    while(1)
    {
      for(i = 0; i < XBEEPACKETSIZE*2; i++)
        read(fd, &buff[i], 1);
      //read(fd, buff, XBEEPACKETSIZE*2);
      if(processXbeeData(buff, XBEEPACKETSIZE*2))
        printf("ADC0:%5d ADC1:%5d ADC2:%5d ADC3:%5d\n", adc0, adc1, adc2, adc3);
    }
    
    close(fd);
  }

  return 0;
}*/
  
  
  
