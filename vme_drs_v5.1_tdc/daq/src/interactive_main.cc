#include"control_funcs.hh"
#include"daq_funcs.hh"

int main(int argc, char* argv[]){
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "interactive_command [ip address]" << std::endl;
    return 0;
  }
  
  (void) signal(SIGINT, UserStop_FromCtrlC);
  // Initialize -----------------------------------------------------------
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  // Connection confirm ---------------------------------------------------
  {
    FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);
    unsigned int version = 0;
    if(0xeeeeeeee == ( version = fModule.ReadModule_nCycle(BCT::mid, BCT::laddr_Version, 4))){
      std::cerr << "#E : Connection fail (Read)" << std::endl;
      return -1;
    }

    std::cout << "#D : Connection Done" << std::endl;
    std::cout << "#D : Firmware version is " 
	      << std::hex << version << std::dec << std::endl;
  }

  // User -----------------------------------------------------------------
  bool quit = false;
  while(!quit){
    std::cout << std::endl;
    
    std::cout << "Menu (" << board_ip << ")\n"
	      << " 1. Interactive RBCP\n"
	      << " 2. Load DAC\n"
	      << " 3. Module initialize\n"
	      << " 4. Initialize DRS\n"
	      << " 5. Do DAQ\n"
	      << " 6. Load Baseline corrections\n"
	      << " 7. Set NIMOUT\n"
	      << " 8. debug\n"
	      << " 9. exit program" << std::endl;
    std::cout << "input number ==> " << std::flush;
    char buf[10];
    fgets(buf, sizeof(buf), stdin);
    char num;
    sscanf(buf, "%c", &num);

    switch(num){
    case '1':
      interactive_rbcp(board_ip, &rbcpHeader);
      break;
    case '2':
      dac_spi(board_ip, &rbcpHeader);
      break;
    case '3':
      module_initialize(board_ip, &rbcpHeader);
      break;
    case '4':
      drs_init(board_ip, &rbcpHeader);
      break;
    case '5':
      daq_reg(board_ip, &rbcpHeader);
      daq(board_ip, &rbcpHeader);
      break;
    case '6':
      lut_load(board_ip, &rbcpHeader);
      blcor1_load(board_ip, &rbcpHeader);
      blcor2_load(board_ip, &rbcpHeader);
      break;
    case '7':
      nimout(board_ip, &rbcpHeader);
      break;
    case '8':
      debug(board_ip, &rbcpHeader);
      break;
    case '9':
      quit = true;
      break;
      
    default:
      break;
    };
  }

  return 0;
}
