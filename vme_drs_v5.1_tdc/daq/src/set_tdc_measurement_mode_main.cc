// -*- C++ -*-

#include <iostream>
#include <iomanip>
#include <sstream>

#include "UDPRBCP.hh"
#include "rbcp.h"
#include "TDCController.hh"
#include "RegisterMap.hh"

const int kNCh = 16;

//______________________________________________________________________________
int
printUsage(char* prog)
{
  std::cout << "#D Usage\n";
  std::cout << prog << " [ip address] [bit pattern in hex]\n"
            << " e.g. \n"
            << "> " << prog << " 192.168.10.16  0xffff\n"
            << " --> set all channels in trailing edge measurement\n"
            << "> " << prog << " 192.168.10.16  0x0000\n"
            << " --> set all channels in very high resolution (x2 leading edges)"
            << std::endl;
  return 0;
}

//______________________________________________________________________________
int main(int argc, char* argv[])
{
  if (3 != argc) return printUsage(argv[0]);

  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  //  int flags = atoi(argv[2]);
  int flags=0;
  std::istringstream iss(argv[2]);
  iss >> std::hex >> flags;

  TDCController tdcCon(board_ip, &rbcpHeader, 0);

  for (int ich=0; ich<kNCh; ++ich) {
    int val = (flags>>ich) & 0x1;
    int mode = 0;
    if (val == 1)
      mode = TDC::kMeasurementTrailing;
    else if (val == 0)
      mode = TDC::kMeasurementVeryHighResolution;
    tdcCon.SetMeasurementMode(ich, mode);
  }
  
  return 0;
}
