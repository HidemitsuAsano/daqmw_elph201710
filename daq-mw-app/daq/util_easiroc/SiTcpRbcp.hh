// -*- C++ -*-

#ifndef SiTCPRBCP_hh
#define SiTCPRBCP_hh

#include <string>

#include "Sock.h"

#include "rbcp.h"

class SiTcpRbcp
{
public:
  static const int kDefaultPort = 4660;
  
public:
  SiTcpRbcp(const std::string& host,
            int port);
  ~SiTcpRbcp();

  int read(unsigned int offsetAddress,
           unsigned char* buffer,
           int length);
  int write(unsigned int offsetAddress,
            unsigned char* buffer,
            int length);
  void Verbosity(int val){m_verbose = val;} 
private:
  DAQMW::Sock m_sock;
  rbcp_header m_header;
  int m_verbose;
};

#endif
