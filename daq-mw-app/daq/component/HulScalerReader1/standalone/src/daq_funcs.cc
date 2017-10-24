#include"daq_funcs.hh"

#include<fstream>
#include<cstdlib>
#include"gzfilter.hh"

int user_stop = 0;

static const int NofHead = 3;
static const int NofBody = 256;
static const int NofData = NofHead + NofBody;

static const int print_step = 10000;

using namespace HUL_Scaler;
#define COMPRESS 1

// signal -----------------------------------------------------------------
void
UserStop_FromCtrlC(int signal)
{
  std::cout << "Stop request" << std::endl;
  user_stop = 1;
}

// execute daq ------------------------------------------------------------
void
daq(char* ip, rbcp_header *rbcpHeader, int runno, int eventno)
{
  (void) signal(SIGINT, UserStop_FromCtrlC);

  // TCP socket
  int sock;
  if(-1 == (sock = ConnectSocket((const char*)ip))) return;
  std::cout << "socket connected" << std::endl;

  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);  

  // set sel trig
  //  unsigned int sel_trig = TRM::reg_L1RM | TRM::reg_L2RM | TRM::reg_EnRM | TRM::reg_EnL2;
  //  unsigned int sel_trig = TRM::reg_L1J0 | TRM::reg_L2J0 | TRM::reg_EnJ0 | TRM::reg_EnL2;
  unsigned int sel_trig = TRM::reg_L1Ext;
  fModule.WriteModule(TRM::mid, TRM::laddr_sel_trig,  sel_trig);

  // Start DAQ
  fModule.WriteModule(SCR::mid, SCR::laddr_counter_reset,  1);
  fModule.WriteModule(DCT::mid, DCT::laddr_gate,  1);

  char filename[256];
#if COMPRESS  
  sprintf(filename, "data/run%d.dat.gz", runno);
#else
  sprintf(filename, "data/run%d.dat", runno);
#endif

  std::ofstream ofs(filename, std::ios::binary);
#if COMPRESS
  h_Utility::ogzfilter cofs(ofs);
#endif

  std::cout << "Start DAQ" << std::endl;
  // DAQ Cycle
  unsigned int buf[NofData];
  for(int n = 0; n<eventno; ++n){
    int n_word;
    while( -1 == ( n_word = Event_Cycle(sock, buf)) && !user_stop) continue;
    if(user_stop) break;
#if COMPRESS
    cofs.write((char*)buf, n_word*sizeof(unsigned int));
#else
    ofs.write((char*)buf, n_word*sizeof(unsigned int));
#endif
    //std::cout << n_word << std::endl;
    if(n%print_step==0){
      printf("\033[2J");
      printf("Event %d\n", n);
      for(int i = 0; i<n_word; ++i){
	printf("%8x ", buf[i]);
	if((i+1)%8 == 0) printf("\n");
      }// for(i)

      printf("\n");
    }
  }// For(n)
  //

  fModule.WriteModule(DCT::mid, DCT::laddr_gate,  0);
  sleep(1);
  std::cout << "clearing buffer... " << std::endl;
  while(-1 != Event_Cycle(sock, buf));

  //  shutdown(sock, SHUT_RDWR);
  close(sock);
#if !COMPRESS
  ofs.close();
#endif

  return;
}

// ConnectSocket ----------------------------------------------------------
int
ConnectSocket(const char* ip)
{
  struct sockaddr_in SiTCP_ADDR;
  unsigned int port = 24;

  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  SiTCP_ADDR.sin_family      = AF_INET;
  SiTCP_ADDR.sin_port        = htons((unsigned short int) port);
  SiTCP_ADDR.sin_addr.s_addr = inet_addr(ip);

  struct timeval tv;
  tv.tv_sec  = 3;
  //tv.tv_sec  = 1;
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

  int flag = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

  if(0 > connect(sock, (struct sockaddr*)&SiTCP_ADDR, sizeof(SiTCP_ADDR))){
    std::cerr << "#E : TCP connection error" << std::endl;
    close(sock);
    return -1;
  }
  
  return sock;
}

// Event Cycle ------------------------------------------------------------
int
Event_Cycle(int sock, unsigned int* buffer)
{
  // data read ---------------------------------------------------------
  static const unsigned int sizeHeader = NofHead*sizeof(unsigned int);
  int ret = receive(sock, (char*)buffer, sizeHeader);
  if(ret < 0) return -1;

  unsigned int n_word_data  = buffer[1] & 0x3ff;
  unsigned int sizeData     = n_word_data*sizeof(unsigned int);

  if(n_word_data == 0){
    std::cout << "no data" << std::endl;
    return NofHead;
  }

  ret = receive(sock, (char*)(buffer + NofHead), sizeData);
  if(ret < 0) return -1;
  
  return NofHead+ n_word_data;
}

// receive ----------------------------------------------------------------
int
receive(int sock, char* data_buf, unsigned int length)
{
  unsigned int revd_size = 0;
  int tmp_ret            = 0;

  while(revd_size < length){
    tmp_ret = recv(sock, data_buf + revd_size, length -revd_size, 0);

    if(tmp_ret == 0) break;
    if(tmp_ret < 0){
      int errbuf = errno;
      perror("TCP receive");
      if(errbuf == EAGAIN){
	// this is time out
       std::cerr <<__FILE__ << " L." << __LINE__ << " time out" << std::endl;
      }else{
	// something wrong
	std::cerr << "TCP error : " << errbuf << std::endl;
      }

      revd_size = tmp_ret;
      break;
    }

    revd_size += tmp_ret;
  }

  return revd_size;
}

