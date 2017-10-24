#include"control_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "set_nimout [ip address] [reg_out1] [reg_out2]" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  unsigned int reg_out1 = atoi(argv[2]);
  unsigned int reg_out2 = atoi(argv[3]);
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;
  nimout(board_ip, &rbcpHeader, reg_out1, reg_out2);

  return 0;
}
