#include"control_funcs.hh"

int main(int argc, char* argv[]){
  if(2 == argc){
    std::cout << "Usage\n";
    std::cout << "set_ofslut [ip address] [directory name]" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;
  lut_load(board_ip, &rbcpHeader, argv[2]);

  return 0;
}
