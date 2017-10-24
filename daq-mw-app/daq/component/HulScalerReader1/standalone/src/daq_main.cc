#include <iostream>
#include <cstdio>

#include "RegisterMap.hh"
#include "network.hh"
#include "UDPRBCP.hh"
#include "CommandMan.hh"
#include "FPGAModule.hh"
#include "rbcp.h"
#include "errno.h"
#include "daq_funcs.hh"

using namespace HUL_Scaler;
enum argIndex{i_bin, i_ip, i_runno, i_eventno};

int main(int argc, char* argv[])
{
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "hul_main [IP address]" << std::endl;
    return 0;
  }// usage
  
  // body ------------------------------------------------------
  char* board_ip = argv[i_ip];
  int runno      = atoi(argv[i_runno]);
  int eventno    = atoi(argv[i_eventno]);
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);
  //fModule.WriteModule(BCT::mid, BCT::laddr_Reset, 0);
  fModule.WriteModule(DCT::mid, DCT::laddr_evb_reset, 1);
  fModule.WriteModule(SCR::mid, SCR::laddr_enable_block, 0x3);
  fModule.WriteModule(SCR::mid, SCR::laddr_enable_hdrst, 0x0);
  //  fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_ModuleBusy);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout2, IOM::reg_o_clk1MHz);

  fModule.WriteModule(IOM::mid, IOM::laddr_extL1, IOM::reg_i_nimin1);
  //fModule.WriteModule(IOM::mid, IOM::laddr_extL2, 0);
  //fModule.WriteModule(IOM::mid, IOM::laddr_extClr, 0);
  fModule.WriteModule(IOM::mid, IOM::laddr_extSpillGate, IOM::reg_i_nimin2);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout2, IOM::reg_o_clk10kHz);

  daq(board_ip, &rbcpHeader, runno, eventno);

  return 0;

}// main
