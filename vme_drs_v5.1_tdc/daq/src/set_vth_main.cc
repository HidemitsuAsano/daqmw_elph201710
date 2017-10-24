#include"control_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "set_vth [ip address] [filename]" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  char* filename = argv[2];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;
  dac_spi(board_ip, &rbcpHeader, filename);

  return 0;
}
