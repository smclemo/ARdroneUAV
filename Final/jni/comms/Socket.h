/*
 * socket.h
 *
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <iostream>
#include <cstdlib>
#include <errno.h>
#include <string.h>

#define DEBUG_SOCKET_TAG "Socket"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

int getpeer(sockaddr &addr);

class Socket
{
 public:
  Socket() :
    m_sock ( -1 )
  {}

  ~Socket()
  {
    if ( is_valid() )
    {
      ::close ( m_sock );
      freeaddrinfo(m_addr);
    }
  }

  int isReadable(int * error,int timeOut);
  int isWriteable(int * error,int timeOut);
  // Socket initialization
  bool create(std::string serverip, int const port);

  // Data Transmission
  bool sendto(const char buffer[], sockaddr * destinfo, socklen_t &destinfo_len) const;
  bool sendto ( const char * buffer, int bytes, sockaddr * destinfo, socklen_t &destinfo_len) const;
  int recvfrom(char buffer[], sockaddr_storage &senderinfo, socklen_t &senderinfo_len) const;
  int recvfrom(char buffer[], int const bytes, sockaddr_storage &senderinfo, socklen_t &senderinfo_len) const;

  void set_non_blocking (bool const b);

  bool is_valid() const { return m_sock != -1; }

  static int const MAXBUFLEN = 65535;
  int m_sock;
  addrinfo *m_addr;
};


#endif /* SOCKET_H_ */

