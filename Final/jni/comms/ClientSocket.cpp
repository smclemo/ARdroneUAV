
#include "ClientSocket.h"

//#include <android/log.h>

ClientSocket::ClientSocket ( std::string host, int const port )
{
  if(!Socket::create(host, port))
  {
  //  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Could not create client socket.");
  }
}

void ClientSocket::create(std::string host, int const port)
{
  if(!Socket::create(host, port))
  {
  //  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Could not create client socket.");
  }
//  Socket::set_non_blocking(true);
}

void ClientSocket::sendto(PointSet &ps, sockaddr_storage &senderinfo, socklen_t &senderinfo_len)
{
//  int readyflag = -1;
//  char *prdyflg = reinterpret_cast<char*>(&readyflag);
  int ps_size = ps.size();
  char *pc = reinterpret_cast<char*>(&ps_size);

//  int recvflag = 0;
////  Socket::set_non_blocking(true);
////  while(recvflag !=readyflag)
////  {
////    int *pps_size =&ps_size;
//    Socket::sendto(prdyflg, sizeof(int), m_addr->ai_addr, m_addr->ai_addrlen);
////    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Sending size: %i, Packets: %i", ps_size, sizeof(int) );
////    select(m_sock+1, &stReadFDS, NULL, NULL, &timeout);
////    if(FD_ISSET(m_sock, &stReadFDS))
////    usleep(1500);
//    {
//      Socket::recvfrom(buffer, sizeof(int), senderinfo, senderinfo_len);
//      recvflag = *(reinterpret_cast<int*>(buffer));
////      __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Received flag: %i", recvflag);
//    }
////    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG, "Received: %i", recvflag);
////  }
////  usleep(100);
  Socket::sendto(pc, sizeof(int), m_addr->ai_addr, m_addr->ai_addrlen);

  int socket_flag = 0;

  for(int i=0; i<ps_size; i++)
  {
    ps[i].serialise(&outbuffer[40*i]);
    ps[i].descriptor.serialise(&outbuffer[40*i+8]);
  }
  
  Socket::sendto(outbuffer, 40*ps_size, m_addr->ai_addr, m_addr->ai_addrlen);

//  Socket::set_non_blocking(false);
//  for(int i=0; i<=ps_size/25; i++)
//  {
//    int ncornersproc = ps_size- 25*i;
//    if(ncornersproc > 25) ncornersproc = 25;
//
//    for(int j=0; j<ncornersproc; j++)
//    {
//      ps[i*25+j].serialise(&outbuffer[40*j]);
//      ps[i*25+j].descriptor.serialise(&outbuffer[40*j+8]);
//    }
//    if(ncornersproc>0)
//    {
//      if(Socket::isWriteable(&socket_flag,10))
//      Socket::sendto(outbuffer, 40*ncornersproc, m_addr->ai_addr, m_addr->ai_addrlen);
//    }
//  }


//  for(int i=0; i<ps_size; i++)
//  {
//    ps[i].serialise(&outbuffer[0]);
//    Socket::sendto(outbuffer, 8, m_addr->ai_addr, m_addr->ai_addrlen);
//
//    /*
//    for(int j=0; j<8; j++)
//    {
//      __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG , "Image Point: %X", pimagepoint[j]);
//    }*/
//
//    ps[i].descriptor.serialise(outbuffer);
//    //Debug only
////    unsigned long long int* des = reinterpret_cast<unsigned long long int*>(pdescriptorserial);
////
////    for(int j=0; j<4; j++)
////    {
////            __android_log_print(ANDROID_LOG_DEBUG, DEBUG_SOCKET_TAG , "Patch: %016llX", des[j]);
////    }
//    Socket::sendto(outbuffer, 32, m_addr->ai_addr, m_addr->ai_addrlen);
//  }
}

int ClientSocket::recvfrom(Commands &command, sockaddr_storage &senderinfo, socklen_t &senderinfo_len)
{
//  char buffer[1024];
//  receive(buffer, 4);
//
//  int *no_commands = reinterpret_cast<int*>(buffer);
//  int n_c = *no_commands;
//  for(int i =0; i < n_c; i++)
//  {
//          receive(buffer, 8);
//
//          float* p_receivedcommands = reinterpret_cast<float *>(buffer);
//          TooN::Vector<2> rc;
//          float receivedcommandsx = p_receivedcommands[0];
//          float receivedcommandsy = p_receivedcommands[1];
//          rc[0] = receivedcommandsx;
//          rc[1] = receivedcommandsy;
//
//          command.command = rc;
//          //command.add(rc);
//  }
//  return *this;
  //Receive 4x4 float transformation matrix for pose and position and orientation of ball
  int socket_flag = 0;
  if(Socket::isReadable(&socket_flag, 5))
  {
    Socket::recvfrom(buffer, 6*4, senderinfo, senderinfo_len);
	}
	else{
	return 0;
	}

  float *tf = reinterpret_cast<float*>(buffer);
//  memcpy(command.transform, tf, 16*4);
  command.get_command(tf);
/*
  int socket_flag = 0;
  //Receive occlusion mask
//  for(int i=0; i<48;i++)
//  {
//    if(Socket::isReadable(&socket_flag, 5))
//    {
//      Socket::recvfrom(buffer, 1000, senderinfo, senderinfo_len);
//      memcpy(command.packet.mask+1000*i, buffer, 1000);
//    }
//    else
//    {
//      return 0;
//    }
//  }
  if(Socket::isReadable(&socket_flag, 5))
  {
    Socket::recvfrom(buffer, 48000, senderinfo, senderinfo_len);
    memcpy(command.packet.mask, buffer, 48000);
  }
*/
  return 1;
}
