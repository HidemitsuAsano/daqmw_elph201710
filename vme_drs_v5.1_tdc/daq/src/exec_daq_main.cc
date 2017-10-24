#include"control_funcs.hh"
#include"daq_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "exec_daq [ip address] [run_no] [event_no]" << std::endl;
    return 0;
  }

  (void) signal(SIGINT, UserStop_FromCtrlC);  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  int   runno    = atoi(argv[2]);
  int   eventno  = atoi(argv[3]);
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;
  daq_reg(board_ip, &rbcpHeader);
  daq(board_ip, &rbcpHeader, runno, eventno);

  return 0;
}
