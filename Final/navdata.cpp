/**
 * @file my_client.c
 * @author karl leplat
 * @date 2009/07/01
 */

#include "navdata.h"


/* Private variables */
static pthread_t nav_thread = 0;
static int32_t nav_thread_alive = 1;

static void navdata_open_server (void);
static void navdata_write (int8_t *buffer, int32_t len);
static void mykonos_navdata_unpack_all(navdata_unpacked_t* navdata_unpacked, navdata_t* navdata, uint32_t* cks);
static void* navdata_loop(void *arg);

static inline int get_mask_from_state( uint32_t state, uint32_t mask )
{
    return state & mask ? TRUE : FALSE;
}

static inline uint8_t* navdata_unpack_option( uint8_t* navdata_ptr, uint8_t* data, uint32_t size )
{
    memcpy(data, navdata_ptr, size);

    return (navdata_ptr + size);
}


static inline navdata_option_t* navdata_next_option( navdata_option_t* navdata_options_ptr )
{
    uint8_t* ptr;

    ptr  = (uint8_t*) navdata_options_ptr;
    ptr += navdata_options_ptr->size;

    return (navdata_option_t*) ptr;
}

navdata_option_t* navdata_search_option( navdata_option_t* navdata_options_ptr, uint32_t tag );


static inline uint32_t navdata_compute_cks( uint8_t* nv, int32_t size )
{
    int32_t i;
    uint32_t cks;
    uint32_t temp;

    cks = 0;

    for( i = 0; i < size; i++ )
	{
		temp = nv[i];
		cks += temp;
	}

    return cks;
}

#define navdata_unpack( navdata_ptr, option ) (navdata_option_t*) navdata_unpack_option( (uint8_t*) navdata_ptr, \
                                                                                         (uint8_t*) &option, \
                                                                                         navdata_ptr->size )
static void mykonos_navdata_unpack_all(navdata_unpacked_t* navdata_unpacked, navdata_t* navdata, uint32_t* cks)
{
    navdata_cks_t navdata_cks = { 0 };
    navdata_option_t* navdata_option_ptr;

    navdata_option_ptr = (navdata_option_t*) &navdata->options[0];

    memset( navdata_unpacked, 0, sizeof(*navdata_unpacked) );

  navdata_unpacked->nd_seq   = navdata->sequence;
  navdata_unpacked->ardrone_state   = navdata->ardrone_state;
  navdata_unpacked->vision_defined  = navdata->vision_defined;

    while( navdata_option_ptr != NULL )
	{
		// Check if we have a valid option
		if( navdata_option_ptr->size == 0 )
		{
			INFO ("One option is not a valid because its size is zero\n");
			navdata_option_ptr = NULL;
		}
		else
		{
			if( navdata_option_ptr->tag <= NAVDATA_NUM_TAGS)
			{
				navdata_unpacked->last_navdata_refresh |= NAVDATA_OPTION_MASK(navdata_option_ptr->tag);
			}
	  
			switch( navdata_option_ptr->tag )
			{
				#define NAVDATA_OPTION(STRUCTURE,NAME,TAG) \
			case TAG: \
			navdata_option_ptr = ardrone_navdata_unpack( navdata_option_ptr, navdata_unpacked->NAME); \
			break;

			#define NAVDATA_OPTION_DEMO(STRUCTURE,NAME,TAG)  NAVDATA_OPTION(STRUCTURE,NAME,TAG)
			#define NAVDATA_OPTION_CKS(STRUCTURE,NAME,TAG) {}

			#include "navdata_keys.h"

			case NAVDATA_CKS_TAG:
			  navdata_option_ptr = ardrone_navdata_unpack( navdata_option_ptr, navdata_cks );
			  *cks = navdata_cks.cks;
			  navdata_option_ptr = NULL; // End of structure
			  break;

			default:
			  PRINT("Tag %d is an unknown navdata option tag\n", (int) navdata_option_ptr->tag);
			  navdata_option_ptr = (navdata_option_t *)(((uint32_t)navdata_option_ptr) + navdata_option_ptr->size);
			  break;
			}
		}
	}
}

static int navdata_udp_socket  = -1;
static void navdata_write (int8_t *buffer, int32_t len)
{
    struct sockaddr_in to;
    int32_t flags;

    if (navdata_udp_socket < 0)
	{
		/// Open udp socket to broadcast at commands to other mykonos
		struct sockaddr_in navdata_udp_addr;

		memset( (char*)&navdata_udp_addr, 0, sizeof(navdata_udp_addr) );

		navdata_udp_addr.sin_family      = AF_INET;
		navdata_udp_addr.sin_addr.s_addr = INADDR_ANY;
		navdata_udp_addr.sin_port        = htons(NAVDATA_PORT + 100);

		navdata_udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );

		if( navdata_udp_socket >= 0 )
		{
			flags = fcntl(navdata_udp_socket, F_GETFL, 0);
			if( flags >= 0 )
			{
				flags |= O_NONBLOCK;
				flags = fcntl(navdata_udp_socket, F_SETFL, flags );
			}
			else
			{
				INFO("Get Socket Options failed\n");
			}
		}
	}
    if( navdata_udp_socket >= 0 ) 
	{
        int res;

        memset( (char*)&to, 0, sizeof(to) );
        to.sin_family       = AF_INET;
        to.sin_addr.s_addr  = inet_addr(REMOTE_SERVER_ADDRESS); 
        to.sin_port         = htons( NAVDATA_PORT );

        res = sendto( navdata_udp_socket, (char*)buffer, len, 0, (struct sockaddr*)&to, sizeof(to) );
    }
}

static void navdata_open_server (void)
{
    int32_t one = 1;
    navdata_write ((int8_t*)&one, sizeof( one ));
}

static void* navdata_loop(void *arg)
{
	uint8_t msg[NAVDATA_BUFFER_SIZE];
    navdata_unpacked_t navdata_unpacked;
    unsigned int cks, navdata_cks, sequence = NAVDATA_SEQUENCE_DEFAULT-1;
    int sockfd = -1, addr_in_size;
	struct sockaddr_in *my_addr, *from;

	INFO("NAVDATA thread starting (thread=%d)...\n", (int)pthread_self());

    navdata_open_server();

	addr_in_size = sizeof(struct sockaddr_in);

    navdata_t* navdata = (navdata_t*) &msg[0];

	from = (struct sockaddr_in *)malloc(addr_in_size);
	my_addr = (struct sockaddr_in *)malloc(addr_in_size);
    assert(from);
    assert(my_addr);

	memset((char *)my_addr,(char)0,addr_in_size);
	my_addr->sin_family = AF_INET;
	my_addr->sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr->sin_port = htons( NAVDATA_PORT );

	if((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
        INFO ("socket: %s\n", strerror(errno));
        goto fail;
	};

	{
		struct timeval tv;
		// 1 second timeout
		tv.tv_sec   = 1;
		tv.tv_usec  = 0;
		setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	}

	INFO("Ready to receive\n");

	while ( nav_thread_alive ) 
	{
		int size;
		size = recvfrom (sockfd, &msg[0], NAVDATA_BUFFER_SIZE, 0, (struct sockaddr *)from,
                         (socklen_t *)&addr_in_size);

		if( size == 0 )
		{
			INFO ("Lost connection \n");
			navdata_open_server();
			sequence = NAVDATA_SEQUENCE_DEFAULT-1;
		}
		if( navdata->header == NAVDATA_HEADER )
		{
			/*mykonos_state = navdata->mykonos_state;
		
			if( get_mask_from_state(navdata->mykonos_state, MYKONOS_COM_WATCHDOG_MASK) ) 
			{ 
				INFO ("[NAVDATA] Detect com watchdog\n");
				sequence = NAVDATA_SEQUENCE_DEFAULT-1; 

				if( get_mask_from_state(navdata->mykonos_state, MYKONOS_NAVDATA_BOOTSTRAP) == FALSE ) 
				{
					const char cmds[] = "AT*COMWDG\r";
					at_write ((int8_t*)cmds, strlen( cmds ));
				}
			} */
			if( get_mask_from_state(navdata->ardrone_state, ARDRONE_COM_WATCHDOG_MASK) )
			{
				// reset sequence number because of com watchdog
				// This code is mandatory because we can have a com watchdog without detecting it on mobile side :
				//        Reconnection is fast enough (less than one second)
				sequence = NAVDATA_SEQUENCE_DEFAULT-1;

				if( get_mask_from_state(navdata->ardrone_state, ARDRONE_NAVDATA_BOOTSTRAP) == FALSE )
				  ardrone_tool_send_com_watchdog(); // acknowledge
			}

			if( navdata->sequence > sequence ) 
			{ 
				ardrone_navdata_unpack_all(&navdata_unpacked, navdata, &navdata_cks);
				cks = navdata_compute_cks( &navdata_buffer[0], size - sizeof(navdata_cks_t) );

				if( cks == navdata_cks )
				{
					INFO ("Unpack navdata\n");
				}
			
			} 
			else 
				INFO ("[Navdata] Sequence pb : %d (distant) / %d (local)\n", navdata->sequence, sequence); 

			sequence = navdata->sequence;
		}
	}
 fail:
    free(from);
    free(my_addr);

    if (sockfd >= 0)
        close(sockfd);

    if (navdata_udp_socket >= 0)
	{
        close(navdata_udp_socket);
        navdata_udp_socket = -1;
    }

	INFO("NAVDATA thread stopping\n");
    return NULL;
}

/* Public functions */
void navdata_stop( void )
{
	if ( !nav_thread )
		return;
   
    nav_thread_alive = 0;
	pthread_join(nav_thread, NULL);
	nav_thread = 0;

	if (navdata_udp_socket >= 0)
	{
		close(navdata_udp_socket);
		navdata_udp_socket = -1;
	}
}

void navdata_run( void )
{
	if ( nav_thread )
		return;

	nav_thread_alive = 1;
	if ( pthread_create( &nav_thread, NULL, navdata_loop, NULL ) )
		INFO("pthread_create: %s\n", strerror(errno));
}

