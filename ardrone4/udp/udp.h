#ifndef __UDP_H
#define __UDP_H
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

struct udp_struct
{
  int s;
  struct sockaddr_in si_me;
  struct sockaddr_in si_other;
  socklen_t slen;
};

int  udpClient_Init(struct udp_struct *udp, const char* host, int port);
int  udpClient_Send(struct udp_struct *udp, char* buf, int len);
void udpClient_Close(struct udp_struct *udp);

int udpServer_Init(struct udp_struct *udp, int port, int blocking);
//returns size of packet received or -1 on error
int udpServer_Receive(struct udp_struct *udp, char* buf, int len);
void udpServer_Close(struct udp_struct *udp);

#ifdef __cplusplus
} 
#endif


#endif