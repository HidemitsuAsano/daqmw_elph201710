#include <string>

#include"control_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "init_module [ip address] ([register filename .txt])" << std::endl;
    return 0;
  }
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  std::string reg_file_name;
  if (argc>2) reg_file_name = argv[2];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;
  module_initialize(board_ip, &rbcpHeader, reg_file_name);

  return 0;
}
