/*
 * Socket.cpp
 *
 */


#include "Socket.h"
#include <string.h>
#include <sstream>
#include <errno.h>
#include <fcntl.h>


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int getpeer(sockaddr &addr)
{
//  for(int i=0; i<14; i++)
//  {
//    std::cout << (int) (unsigned char) addr.sa_data[i] << std::endl;
//  }
//  std::cout << (int) (unsigned char) addr.sa_data[5] << std::endl;
  return (int) (unsigned char) addr.sa_data[5];
}


int Socket::isReadable(int * error,int timeOut) { // milliseconds
  fd_set socketReadSet;
  FD_ZERO(&socketReadSet);
  FD_SET(m_sock,&socketReadSet);
  struct timeval tv;
  if (timeOut) {
    tv.tv_sec  = timeOut / 1000;
    tv.tv_usec = (timeOut % 1000) * 1000;
  } else {
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
  } // if
  if (select(m_sock+1,&socketReadSet,0,0,&tv) == -1) {
    *error = 1;
    return 0;
  } // if
  *error = 0;
  return FD_ISSET(m_sock,&socketReadSet) != 0;
} /* isReadable */

int Socket::isWriteable(int * error,int timeOut)
{
  fd_set socketWriteSet;
  FD_ZERO(&socketWriteSet);
  FD_SET(m_sock,&socketWriteSet);
  struct timeval tv;
  if (timeOut) {
    tv.tv_sec  = timeOut / 1000;
    tv.tv_usec = (timeOut % 1000) * 1000;
  } else {
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
  } // if
  if (select(m_sock+1,0,&socketWriteSet,0,&tv) == -1) {
    *error = 1;
    return 0;
  } // if
  *error = 0;
  return FD_ISSET(m_sock,&socketWriteSet) != 0;
}

bool Socket::create(std::string serverip, int const port)
{
  addrinfo hints, *servinfo;
  int rv=0;

  std::stringstream portsstr;
  portsstr << port;

  //Load address hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(serverip.c_str(), portsstr.str().c_str() , &hints, &servinfo)) != 0)
  {
  //  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "getaddrinfo %s", gai_strerror(rv));
    return false;
  }

  // loop through all the results and create socket with given address type
  for(m_addr = servinfo; m_addr != NULL; m_addr = m_addr->ai_next)
  {
    if ((m_sock = socket(m_addr->ai_family, m_addr->ai_socktype,
        m_addr->ai_protocol)) == -1) {
     // __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Talker: Socket");
      continue;
    }
    break;
  }

  if (m_addr == NULL)
  {
    //__android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Failed to create socket");
    return false;
  }

//  freeaddrinfo(servinfo);

  int buffsize = 150000; // 65536
  setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (void*)&buffsize, sizeof(buffsize));
 // __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "UDP OS buffer size %d", buffsize);

  return true;
}

bool Socket::sendto(const char buffer[], sockaddr * destinfo, socklen_t &destinfo_len) const
{
  return ::sendto ( m_sock, buffer, sizeof(buffer), 0, destinfo, destinfo_len);
}

bool Socket::sendto ( const char * buffer, int bytes, sockaddr * destinfo, socklen_t &destinfo_len) const
{
  return ::sendto( m_sock, buffer, bytes, 0, destinfo, destinfo_len);
}

int Socket::recvfrom(char buffer[], sockaddr_storage &senderinfo, socklen_t &senderinfo_len) const
{
//  memset ( buffer, 0, MAXBUFLEN + 1 );
  return ::recvfrom ( m_sock, buffer, MAXBUFLEN-1, 0, (sockaddr *)&senderinfo, &senderinfo_len);
  //std::cout << "Bytes received: " << status << std::endl;
}

int Socket::recvfrom(char buffer[], int const bytes, sockaddr_storage &senderinfo, socklen_t &senderinfo_len) const
{
//  memset ( buffer, 0, bytes + 1 );
  return ::recvfrom ( m_sock, buffer, bytes , MSG_WAITALL, (sockaddr *)&senderinfo, &senderinfo_len);
  //std::cout << "Bytes received: " << status << std::endl;
}

void Socket::set_non_blocking (bool const b)
{
  int opts = fcntl ( m_sock, F_GETFL );

  if ( opts < 0 )
  {
    return;
  }

  if ( b )
    opts = ( opts | O_NONBLOCK );
  else
    opts = ( opts & ~O_NONBLOCK );

  fcntl ( m_sock, F_SETFL,opts );
}
