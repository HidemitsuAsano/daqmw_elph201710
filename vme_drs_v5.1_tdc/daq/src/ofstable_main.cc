#include"control_funcs.hh"
#include"daq_funcs.hh"
#include "TDCController.hh"

#include <cstdlib>

int main(int argc, char* argv[]){
  if(2 == argc){
    std::cout << "Usage\n";
    std::cout << "ofstable [ip address] [run number]" << std::endl;
    return 0;
  }


  std::cout << "#D " << argv[0] << " " << argv[1] << " " << argv[2] << std::endl;
  (void) signal(SIGINT, UserStop_FromCtrlC);
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  // ModuleInitialize
  module_initialize(board_ip, &rbcpHeader);

  // set daq regs ---------------------------------------------
  FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);

  // DRS4 and EVB read count
  unsigned int read_count = 1024;
  set_read_count(board_ip, &rbcpHeader, read_count);

  // Set global trigger in MTM
  fModule.WriteModule(MTM::mid, MTM::laddr_sel_trig,
		      MTM::reg_L1clk);
  fModule.WriteModule(MTM::mid, MTM::laddr_delay, 0);
  set_clktrig_freq(board_ip, &rbcpHeader, MTM::reg_freq_xxxhz);

  // DAQ mode
  EventBuilder event_builder(fModule);
  event_builder.WriteRegister(EVB::laddr_control, 0);
  event_builder.WriteRegister(EVB::laddr_en_wf, 0xffff);
  
  unsigned int size_event_buf = 1;
  event_builder.WriteRegister(EVB::laddr_nevent, size_event_buf);
  event_builder.WriteOfsTable("./pede_zero");

  noise_filter_th(board_ip, &rbcpHeader);
  zero_suppress_th(board_ip, &rbcpHeader);

  // disable TDC
  {
    TDCController tdcCon(board_ip, &rbcpHeader);
    tdcCon.Disable(true);

    // ignore busy from TDC
    fModule.WriteModule(DAQ::mid, DAQ::laddr_EnBusy,
                        DAQ::reg_self | DAQ::reg_DRS | DAQ::reg_EVB);
  }
  

  // exec daq ------------------------------------------------
  int run = atoi(argv[2]);
  daq(board_ip, &rbcpHeader, run, 10000);

  return 0;
}
