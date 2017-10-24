// -*- C++ -*-

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "getaddr.h"


//______________________________________________________________________________
unsigned int
getaddr_ipv4(const char* hostname)
{
  unsigned int ret = 0xffffffff;
  
  addrinfo hints;
  addrinfo* res;
  sockaddr_in* addr_in;
  int s;

  // initialize hints
  memset(&hints, 0, sizeof(addrinfo));
  //  hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = 0;
  hints.ai_protocol = 0; // any protocol

  // resolve name
  s = getaddrinfo(hostname, 0, &hints, &res);

  if (s!=0) {
    std::string errormess = gai_strerror(s);
    std::cerr << "getaddrinfo(): " << errormess << std::endl;
  }

  // std::cout << "ai_family: " << res->ai_family << std::endl;
  // std::cout << "ai_socktype: " << res->ai_socktype << std::endl;
  // std::cout << "ai_protocol: " << res->ai_protocol << std::endl;
  // std::cout << "ai_addrlen: " << res->ai_addrlen << std::endl;
  //  std::cout << "ai_canonname: " << res->ai_canonname << std::endl;

  addr_in = reinterpret_cast<sockaddr_in*>(res->ai_addr);
  //  std::cout << std::hex << addr_in->sin_addr.s_addr << std::dec << std::endl;
  ret = addr_in->sin_addr.s_addr;
  
  freeaddrinfo(res);
  return ret;
}
