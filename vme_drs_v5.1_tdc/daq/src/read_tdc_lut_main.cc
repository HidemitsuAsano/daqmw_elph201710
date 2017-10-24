// -*- C++ -*-

#include <iostream>
#include <iomanip>

#include <string>
#include <sstream>

#include <unistd.h>

#include "control_funcs.hh"


int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << argv[0] << " [ip address] [0x3ffffffff (enable read)] [-f]" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  unsigned int en_ch = 0x1ffff;
  
  for (int i=0; i<argc; ++i) {
    std::string opt(argv[i]);
    if (opt.find("0x")==0) {
      std::istringstream iss(opt);
      iss >> std::hex >> en_ch;
      en_ch &= 0x3ffffffff;
      std::cout << "#D enable channel = " << std::hex << opt << " " << en_ch << std::dec << std::endl;
    } else if (opt=="-f") {
      tdc_start_calibration(board_ip, &rbcpHeader);
    }
  }
    
  //  const int nch = 5;
  //const int nch = 9;
  const int nch = 34;
  for (unsigned int ich=0; ich<nch; ++ich){
    if ( ((en_ch>>ich) & 0x1) == 1) 
      tdc_peek_lut(board_ip, &rbcpHeader, ich, 512);
    usleep(100000);
  }

  return 0;
  
}
