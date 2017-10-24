#include <iostream>
#include <cstdio>
#include <sys/time.h>

#include "RegisterMap.hh"
#include "network.hh"
#include "UDPRBCP.hh"
#include "CommandMan.hh"
#include "FPGAModule.hh"
#include "rbcp.h"

using namespace HUL_Scaler;

int main(int argc, char* argv[])
{
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "set_nimio [IP address]" << std::endl;
    return 0;
  }// usage
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);
  std::cout << "FPGA version     " << std::hex << fModule.ReadModule(BCT::mid, BCT::laddr_Version, 4) << std::endl;
  std::cout << "IOM::nimout1     " << std::hex << fModule.ReadModule(IOM::mid, IOM::laddr_nimout1, 1) << std::endl;
  std::cout << "IOM::nimout2     " << fModule.ReadModule(IOM::mid, IOM::laddr_nimout2, 1) << std::endl;
  std::cout << "IOM::nimout3     " << fModule.ReadModule(IOM::mid, IOM::laddr_nimout3, 1) << std::endl;
  std::cout << "IOM::nimout4     " << fModule.ReadModule(IOM::mid, IOM::laddr_nimout4, 1) << std::endl;
  std::cout << "IOM::extL1       " << fModule.ReadModule(IOM::mid, IOM::laddr_extL1, 1) << std::endl;
  std::cout << "IOM::extL2       " << fModule.ReadModule(IOM::mid, IOM::laddr_extL2, 1) << std::endl;
  std::cout << "IOM::extCtr      " << fModule.ReadModule(IOM::mid, IOM::laddr_extClr, 1) << std::endl;
  std::cout << "IOM::extSpilgate " << fModule.ReadModule(IOM::mid, IOM::laddr_extSpillGate, 1) << std::endl;
  //  fModule.WriteModule(IOM::mid, IOM::laddr_nimout3, IOM::reg_o_clk1kHz);
  //  std::cout << fModule.ReadModule(IOM::mid, IOM::laddr_nimout3, 1) << std::endl;;
  //  fModule.WriteModule(IOM::mid, IOM::laddr_nimout4, IOM::reg_o_clk1kHz);
  std::cout << "SCR::enabled block " << fModule.ReadModule(SCR::mid, SCR::laddr_enable_block,1) << std::endl;
  std::cout << "SCR::enabled hdrst " << fModule.ReadModule(SCR::mid, SCR::laddr_enable_hdrst,1) << std::endl;
  std::cout << "DCT::DAQ gate      " << fModule.ReadModule(DCT::mid, DCT::laddr_gate,1) << std::endl;
  std::cout << "TRM::set_trig      " << fModule.ReadModule(TRM::mid, TRM::laddr_sel_trig,1) << std::endl;
  std::cout << "NIM out test " << std::endl;
  
  fModule.WriteModule(BCT::mid, BCT::laddr_Reset, 0);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_DaqGate);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_DIP8);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_DaqGate);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_RML1);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_clk1MHz);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_RMSnInc);
  //std::cout << fModule.ReadModule(IOM::mid, IOM::laddr_nimout1,1) << std::endl;
  /*
  while(1){
  fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_clk1MHz);
  //struct timespec ts;
  //ts.tv_sec  = 0;
  //ts.tv_nsec = 5;
  //nanosleep(&ts,NULL);
  usleep(1);
  fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_DIP8);
  std::cout << fModule.ReadModule(IOM::mid, IOM::laddr_nimout1,1) << std::endl;
  //ts.tv_sec  = 1;
  //ts.tv_nsec = 0;
  //nanosleep(&ts,NULL);
  }
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_DIP8);
  */
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_clk10kHz);
  //usleep(1000);
  //fModule.WriteModule(IOM::mid, IOM::laddr_nimout1, IOM::reg_o_DIP8);

  return 0;

}// main
