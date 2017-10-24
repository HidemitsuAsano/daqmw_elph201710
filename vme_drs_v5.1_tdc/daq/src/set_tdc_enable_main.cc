// -*- C++ -*-

#include <unistd.h>

#include <iostream>
#include <iomanip>

#include <string>
#include <sstream>

#include "UDPRBCP.hh"
#include "rbcp.h"
#include "TDCController.hh"
#include "RegisterMap.hh"

//______________________________________________________________________________
int main(int argc, char* argv[]){
  if (1 == argc){
    std::cout << "Usage\n"
              << argv[0] << " [ip address]" << std::endl;
    return 0;
  }

  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  TDCController tdcCon(board_ip, &rbcpHeader, 0);

  tdcCon.Disable(false);

  return 0;
}
