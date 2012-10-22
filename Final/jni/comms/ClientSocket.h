
#ifndef ClientSocket_class
#define ClientSocket_class

#include "Socket.h"
#include "../pointset.h"
#include "../commands.h"
#include "../TooN/TooN.h"

#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

class ClientSocket : public Socket
{
 public:
  ClientSocket(){}
  ClientSocket(std::string host, int const port);

  void create(std::string host, int const port);
  void sendto(PointSet &ps, sockaddr_storage &senderinfo, socklen_t &senderinfo_len);
  int recvfrom(Commands &command, sockaddr_storage &senderinfo, socklen_t &senderinfo_len);

  char buffer[Socket::MAXBUFLEN];
  char outbuffer[Socket::MAXBUFLEN];
};


#endif
