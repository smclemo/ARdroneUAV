/**
 * @file my_client.c
 * @author karl leplat
 * @date 2009/07/01
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "app.h"

static pthread_mutex_t at_cmd_lock;
static pthread_t at_thread = 0;
static int32_t at_thread_alive = 1;

uint32_t mykonos_state = 0;

/* AT constant */
#define AT_PORT                   5556
#define AT_BUFFER_SIZE            1024

typedef enum {
    MYKONOS_UI_BIT_AG             = 0, /* Button turn to left */
    MYKONOS_UI_BIT_AB             = 1, /* Button altitude down (ah - ab)*/
    MYKONOS_UI_BIT_AD             = 2, /* Button turn to right */
    MYKONOS_UI_BIT_AH             = 3, /* Button altitude up (ah - ab)*/
    MYKONOS_UI_BIT_L1             = 4, /* Button - z-axis (r1 - l1) */
    MYKONOS_UI_BIT_R1             = 5, /* Not used */
    MYKONOS_UI_BIT_L2             = 6, /* Button + z-axis (r1 - l1) */ 
    MYKONOS_UI_BIT_R2             = 7, /* Not used */
    MYKONOS_UI_BIT_SELECT         = 8, /* Button emergency reset all */
    MYKONOS_UI_BIT_START          = 9, /* Button Takeoff / Landing */
    MYKONOS_UI_BIT_TRIM_THETA     = 18, /* y-axis trim +1 (Trim increase at +/- 1??/s) */
    MYKONOS_UI_BIT_TRIM_PHI       = 20, /* x-axis trim +1 (Trim increase at +/- 1??/s) */
    MYKONOS_UI_BIT_TRIM_YAW       = 22, /* z-axis trim +1 (Trim increase at +/- 1??/s) */
    MYKONOS_UI_BIT_X              = 24, /* x-axis +1 */
    MYKONOS_UI_BIT_Y              = 28, /* y-axis +1 */
} mykonos_ui_bitfield_t;

typedef struct _radiogp_cmd_t {
  int32_t pitch; 
  int32_t roll;
  int32_t gaz; 
  int32_t yaw;
} radiogp_cmd_t;
static radiogp_cmd_t radiogp_cmd = {0};

#define MYKONOS_NO_TRIM							\
    ((1 << MYKONOS_UI_BIT_TRIM_THETA) |			\
	 (1 << MYKONOS_UI_BIT_TRIM_PHI) |			\
	 (1 << MYKONOS_UI_BIT_TRIM_YAW) |			\
	 (1 << MYKONOS_UI_BIT_X) |					\
	 (1 << MYKONOS_UI_BIT_Y))

static int at_udp_socket  = -1;
static int overflow = 0;
static unsigned long ocurrent = 0;
char cmd[AT_BUFFER_SIZE];

static void send_command();
static void* at_cmds_loop(void *arg);
static void boot_drone(void);
static unsigned long get_time_ms(void);
unsigned int nb_sequence = 1;

static inline int get_mask_from_state( uint32_t state, uint32_t mask )
{
    return state & mask ? true : false;
}

static unsigned long get_time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

static void boot_drone(void)
{
	char cmds[AT_BUFFER_SIZE];
	snprintf(cmds, AT_BUFFER_SIZE, "AT*CONFIG=%d,\"general:navdata_demo\",\"FALSE\"\r", nb_sequence++);

//	if ( get_mask_from_state(mykonos_state, MYKONOS_NAVDATA_BOOTSTRAP)) 
	//{
		at_write((int8_t*)cmds, strlen (cmds));

	/*	int retry = 20;
		int bcontinue = true;
		int next = 0;
		while (bcontinue && retry) 
		{
			if (next == 0) 
			{
				if ( get_mask_from_state( mykonos_state, MYKONOS_COMMAND_MASK )) 
				{
					INFO ("[CONTROL] Processing the current command ... \n");
					next++;
				}
			}
			else 
			{
				char str[AT_BUFFER_SIZE];
				memset (str, 0, AT_BUFFER_SIZE); 
				sprintf (str, "AT*CTRL=%d,%d\r", ACK_CONTROL_MODE, 0);
				at_write ((int8_t*)str, strlen (str));

				if ( !get_mask_from_state( mykonos_state, MYKONOS_COMMAND_MASK )) 
				{
					INFO ("[CONTROL] Ack control event OK, send navdata demo\n");
					bcontinue = FALSE;
				}
			}
			sleep (1);
			retry--;
		}
	}*/
}

static void* at_cmds_loop(void *arg)
{
	unsigned long current, deadline;

	INFO("AT commands thread starting (thread=%d)...\n", (int)pthread_self());

	ocurrent = get_time_ms();

	INFO("attempting to boot drone...\n");
	boot_drone();
			
    while (at_thread_alive) 
	{
		// compute next loop iteration deadline
		deadline = get_time_ms() + MYKONOS_REFRESH_MS;

		// send pilot command
		send_command();

		// sleep until deadline
		current = get_time_ms();
		if (current < deadline) 
		{
			usleep(1000*(deadline-current));
		}
	}

	INFO("AT commands thread stopping\n");
    return NULL;
}

static void send_command()
{
	unsigned long current;

	current = get_time_ms();

	//Send AT command	
	at_write ((int8_t*)cmd, strlen (cmd));

	// check 30 ms overflow
	if (current > ocurrent + 30) 
	{
		overflow += current - ocurrent - MYKONOS_REFRESH_MS;
	}
	ocurrent = current;

	/* dump command every Xs */
/*	if ((nb_sequence % 50) == 0) 
	{
		pthread_mutex_lock( &at_cmd_lock );
		INFO("seq=%d radgp(%d,%d,%d,%d) over=%d\n", nb_sequence,
			 radiogp_cmd.pitch, radiogp_cmd.roll, radiogp_cmd.gaz, radiogp_cmd.yaw, overflow);
		pthread_mutex_unlock( &at_cmd_lock );
		overflow = 0;
	}*/
}

void at_write(int8_t *buffer, int32_t len)
{
	struct sockaddr_in to;
	int32_t flags;

	pthread_mutex_lock( &at_cmd_lock );
	if( at_udp_socket < 0 ) 
	{
		struct sockaddr_in at_udp_addr;

		memset( (char*)&at_udp_addr, 0, sizeof(at_udp_addr) );

		at_udp_addr.sin_family      = AF_INET;
		at_udp_addr.sin_addr.s_addr = INADDR_ANY;
		at_udp_addr.sin_port        = htons( AT_PORT + 100 );

		at_udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );

		if(at_udp_socket >= 0)
		{
			flags = fcntl(at_udp_socket, F_GETFL, 0);
			if( flags >= 0 )
			{
				flags |= O_NONBLOCK;
				flags = fcntl(at_udp_socket, F_SETFL, flags );
			}
			else
				INFO("Get Socket Options failed\n");

			if (bind(at_udp_socket, (struct sockaddr*)&at_udp_addr, sizeof(struct sockaddr)) < 0) 
				INFO ("at_write:bind: %s\n", strerror(errno));
		}
	}

	if(at_udp_socket >= 0)
	{
		int res;

		memset( (char*)&to, 0, sizeof(to) );
		to.sin_family       = AF_INET;
		to.sin_addr.s_addr  = inet_addr(REMOTE_SERVER_ADDRESS); 
		to.sin_port         = htons (AT_PORT);

		res = sendto( at_udp_socket, (char*)buffer, len, 0, (struct sockaddr*)&to, sizeof(to) );
	}
    pthread_mutex_unlock( &at_cmd_lock );
}

/* Public functions */
void at_stop(void)
{
	if (!at_thread)
		return;

	at_land();
	
	at_thread_alive = 0;
	pthread_join(at_thread, NULL);
	at_thread = 0;
	pthread_mutex_destroy(&at_cmd_lock);

	if (at_udp_socket >= 0)
	{
		close(at_udp_socket);
		at_udp_socket = -1;
	}
}

/************* at_init ****************
* Description : Initialize AT process.    
*/
void at_run(void)
{
	if (at_thread)
		return;

	/* Initialize mutex */
	if (pthread_mutex_init(&at_cmd_lock, NULL) != 0)
	{
	   INFO("AT mutex init failed: %s\n", strerror(errno));
	   return;
	}

	memset(cmd, 0x0, AT_BUFFER_SIZE);
	snprintf(cmd, AT_BUFFER_SIZE, "AT*PCMD=%d,0,0,0,0,0\rAT*REF=%d,290717696\r", 1, 1);

	// AT cmds loop
	at_thread_alive = 1;
	if (pthread_create(&at_thread, NULL, at_cmds_loop, NULL))
	{
		INFO("pthread_create: %s\n", strerror(errno));
	}
}

/************* at_set_flat_trim ****************
* Description : Calibration of the ARDrone.   
*/
void at_set_flat_trim(void)
{
	if (!at_thread)
		return;
		
	pthread_mutex_lock( &at_cmd_lock );
	memset(cmd, 0x0, AT_BUFFER_SIZE);
	snprintf(cmd, AT_BUFFER_SIZE, "AT*FTRIM=%d\r", nb_sequence++);
	pthread_mutex_unlock( &at_cmd_lock );
	
	sleep(1);
}

/************* at_takeoff ****************
* Description : Takeoff
*/
void at_takeoff(void)
{
	if (!at_thread)
		return;
		
	at_set_flat_trim();
	
	pthread_mutex_lock( &at_cmd_lock );
	memset(cmd, 0x0, AT_BUFFER_SIZE);
	snprintf(cmd, AT_BUFFER_SIZE, "AT*PCMD=%d,0,%ld,%ld,%ld,%ld\rAT*REF=%d,290718208\r", 
							nb_sequence++, 0, 0, 0, 0, nb_sequence++);
	pthread_mutex_unlock( &at_cmd_lock );
	
	sleep(5);
	at_hover();
}

/************* at_land ****************
* Description : Landing  
*/
void at_land(void)
{
	if (!at_thread)
		return;
		
	pthread_mutex_lock(&at_cmd_lock);
	memset(cmd, 0x0, AT_BUFFER_SIZE);
	//snprintf(cmd, AT_BUFFER_SIZE, "AT*PCMD=%d,0,%ld,%ld,%ld,%ld\rAT*REF=%d,290717696\r", 
		//					nb_sequence++, 0, 0, 0, 0, nb_sequence++);
	snprintf(cmd, AT_BUFFER_SIZE, "AT*REF=%d,290717696\r", nb_sequence++);
	pthread_mutex_unlock(&at_cmd_lock);
	
	sleep(5);
}

/************* at_hover ****************
* Description : Stabilised hovering  
*/
void at_hover(void)
{
	if (!at_thread)
		return;

	pthread_mutex_lock(&at_cmd_lock);
	memset(cmd, 0x0, AT_BUFFER_SIZE);
	snprintf(cmd, AT_BUFFER_SIZE, "AT*PCMD=%d,0,%ld,%ld,%ld,%ld\r", nb_sequence++, 0, 0, 0, 0);
	pthread_mutex_unlock(&at_cmd_lock);
}

/************* at_set_radiogp_input ****************
* Description : Fill struct radiogp_cmd, 
* used with at_cmds_loop function.
* pitch : y-axis (rad) (-1, +1)  
* roll : x-axis (rad) (-1, +1) positive rolls right
* gaz :  altitude (mm/s) (-1, +1) positive moves up
* yaw : z-axis (rad/s) (-1, +1)
*/
void at_set_radiogp_input(int32_t pitch, int32_t roll, int32_t gaz, int32_t yaw)
{
	if (!at_thread)
		return;
	
    pthread_mutex_lock(&at_cmd_lock);
	radiogp_cmd.pitch = pitch;
	radiogp_cmd.roll = roll;
	radiogp_cmd.gaz = gaz;
	radiogp_cmd.yaw = yaw;
	
	memset(cmd, 0x0, AT_BUFFER_SIZE);
	snprintf(cmd, AT_BUFFER_SIZE, "AT*PCMD=%d,1,%d,%d,%d,%d\r", nb_sequence++, roll, pitch, gaz, yaw);
	pthread_mutex_unlock( &at_cmd_lock );
}
