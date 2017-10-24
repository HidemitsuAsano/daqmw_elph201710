#include"control_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "write_reg [ip address] [module id] [local address] [register]" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  unsigned int mid   = atoi(argv[2]);
  unsigned int laddr = atoi(argv[3]);
  unsigned int reg   = atoi(argv[4]);
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);
  fModule.WriteModule(mid, laddr, reg);

  return 0;
}
