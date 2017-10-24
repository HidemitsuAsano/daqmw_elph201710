// -*- C++ -*-

#include <iostream>
#include <iomanip>

#include "UDPRBCP.hh"
#include "rbcp.h"
#include "TDCController.hh"

const int kNOctet = 25;

//______________________________________________________________________________
int printUsage(char* proc)
{
  std::cout << "Usage: " << proc
            << " [ip] "
            << " [ch]"
            << std::endl;
  return 0;
}

//______________________________________________________________________________
int main(int argc, char* argv[])
{
  if (argc!=3) {
    return printUsage(argv[0]);
  }
  
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  int ch = atoi(argv[2]);

  TDCController tdcCon(board_ip, &rbcpHeader, 0);

  std::vector<unsigned char> bit_vector(kNOctet);

  tdcCon.GetCDCBits(ch, bit_vector);
  return 0;
}
