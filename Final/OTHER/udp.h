/*
    udp.h - udp driver
*/ 
#ifndef _UDP_H
#define _UDP_H

/*#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h> 
#include <sys/time.h>*/

#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>	      // low-level i/o 
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "util.h"

int udp_init();
int udp_update(char* message);
void udp_close();

#endif
