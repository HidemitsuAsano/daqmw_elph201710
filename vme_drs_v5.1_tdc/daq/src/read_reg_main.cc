#include"control_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "read_reg [ip address] [module id] [local address] [nword]" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  unsigned int mid    = atoi(argv[2]);
  unsigned int laddr  = atoi(argv[3]);
  unsigned int ncycle = atoi(argv[4]);
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);
  unsigned int reg = fModule.ReadModule_nCycle(mid, laddr, ncycle);
  std::cout << "#D : Result " << std::hex << reg << std::endl;

  return 0;
}
