/*
 *  ATComSerialProxyServer
 *  v0.2.2
 *  ARDroneTools
 *
 *  Created by nosaari on 25.02.11.
 *

 Very basic proxy server that reads any lines in given tty device and sends it
 via UDP to given server.
 Any line thats not starting with 'AT' is considered as debug output and will
 be ignored!

 Based on server code from
 http://www.gamedev.net/topic/310343-udp-client-server-echo-example/

 For detailed infos read
 http://www.linuxhowtos.org/C_C++/socket.htm
 
 *
 */
 
 #include "udp.h"

// Address of control server on drone, should be 192.168.1.1
// Test the server locally by setting address to "127.0.0.1" and
// typing "nc -l -u 127.0.0.1 5556" in a terminal window!
#define REMOTE_SERVER_ADDRESS   "192.168.1.1"
// Port of control server for AT commands, leave at 5556
#define REMOTE_SERVER_PORT      5556

int connectionSocket = 0;
struct sockaddr_in remoteAddress;
uint addressLength = 0;	
char command[100];	//Protected by semaphore
pthread_t udp_thread;
pthread_mutex_t udp_access_mutex = PTHREAD_MUTEX_INITIALIZER;
	
void* udp_thread_main(void* data)
{
	int rc;
	
	while(1)
	{
		pthread_mutex_lock(&udp_access_mutex);
		
		// Send message to remote server
		rc = sendto(connectionSocket, command, strlen(command), 0, 
				    (struct sockaddr*) &remoteAddress, addressLength);
		pthread_mutex_unlock(&udp_access_mutex);
		
		if (rc < 0)
			fprintf(stderr, "ERROR: Cannot send data!\n");

		// Update every 30ms as per SDK advice (must be less than 250ms)
		usleep(30000);
	}
}

int udp_init()
{
    // Create socket with UDP setting (SOCK_DGRAM)
    connectionSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (connectionSocket < 0)
    {
        fprintf(stderr, "ERROR: Cannot open socket!\n");
        return 1;
    }

    // Initialise remote server address
    memset(&remoteAddress, 0, sizeof(struct sockaddr_in));
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_addr.s_addr = inet_addr(REMOTE_SERVER_ADDRESS);
    remoteAddress.sin_port = htons(REMOTE_SERVER_PORT);
    addressLength = sizeof(remoteAddress);

	memset(command, 0x0, 100);
	sprintf(command, "AT*PCMD=%d,0,0,0,0,0\rAT*REF=%d,290717696\r", 1, 1);
	strncat(command, "\r", 2);
	
	int rc = pthread_create(&udp_thread, NULL, udp_thread_main, NULL);
	if (rc) {
		printf("ctl_Init: Return code from pthread_create(udp_thread) is %d\n", rc);
		return 1;
	}
	
	return 0;
}

int udp_update(char* message)
{
	// CONVENTION: Only send messages that begin with the chars 'AT'!
	// Every AT command must begin with these, everything else is ignored!
	if (message[0] == 'A' && message[1] == 'T')
	{
		pthread_mutex_lock(&udp_access_mutex);
		memset(command, 0x0, 100);
		strcpy(command, message);
		strncat(command, "\r", 2);
		pthread_mutex_unlock(&udp_access_mutex);
		
		return 0;
	}

	return 1;
}

void udp_close()
{
	close(connectionSocket);
	pthread_cancel(udp_thread);
}