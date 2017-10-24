#include"DRS4Controller.hh"
#include"daq_funcs.hh"
#include"gzfilter.hh"

#include "getaddr.h"

#include "TDCController.hh"

int user_stop = 0;

// signal -----------------------------------------------------------------
void
UserStop_FromCtrlC(int signal)
{
  user_stop = 1;
}

// execute daq ------------------------------------------------------------
void
daq(char* ip, rbcp_header *rbcpHeader,
    int run_no, int event_no
    )
{
  std::cout << "#D daq()" << std::endl;
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);

  std::cout << "#D Set DRS4 addr" << std::endl;
  // Set DRS4 addr
  DRS4Controller drsMan(fModule);
  drsMan.WriteDRSAddr(DRS::addr_mux_ch8);


  // check TDC is enabled/disabled
  bool isTdcDisabled = false;
  {
    TDCController tdcCon(ip, rbcpHeader, 0);
    isTdcDisabled = tdcCon.IsDisabled();
  }

  std::cout << "#D get header size" << std::endl;
  // Header size
  EventBuilder event_builder(fModule);
  int header_size = event_builder.GetHeaderSize();
  std::cout << "#D ADC header size = " << header_size << std::endl;
  std::cout << "#D reset FIFO" << std::endl;
  event_builder.ResetFIFO();

  std::cout << "#D turn on DAQ mode" << std::endl;
  // Turn on DAQ mode
  fModule.WriteModule(DAQ::mid, DAQ::laddr_ExecDAQ, 0x1);

  // TCP socket
  int sock;
  if(-1 == (sock = ConnectSocket((const char*)ip))) return;

  // file
  char namebuf[256];
  sprintf(namebuf, "data/run%05d.dat.gz", run_no);

  {
    // check file ++++++++++++++++++++
    std::ifstream ifile(namebuf);
    if (!ifile.fail()) {
      std::cout << "#D file " << ifile << " already exists. exit RUN " << run_no << std::endl;
      std::cout << "#D turn off DAQ mode" << std::endl;
      // Turn off DAQ mode
      fModule.WriteModule(DAQ::mid, DAQ::laddr_ExecDAQ, 0x0);
      std::cout << "#D user FIFO reset" << std::endl;
      // User reset
      event_builder.ResetFIFO();
  
      shutdown(sock, SHUT_RDWR);
      close(sock);
      
      std::cout << "#D reset DRS4 addr" << std::endl;
      // Reset DRS4 addr
      drsMan.WriteDRSAddr(DRS::addr_mux_ch8);
      std::exit(1);
    }
  }
  //std::ofstream ofs("/dev/null", std::ios::binary);
  //std::ofstream ofs("hoge.dat", std::ios::binary);
  std::ofstream ofs_org(namebuf, std::ios::binary);
   h_Utility::ogzfilter ofs(ofs_org);

  std::cout << "#D : datafile: " << namebuf
	    << " , MaxEvent: " << event_no << std::endl;

  int NofEvent = event_no;
  int EventNum = 0;
  while(1){
    if(EventNum == NofEvent){break;}
    if(user_stop == 1){
      std::cout << "Stop request" << std::endl;
      ofs.flush();
      break;
    }

    unsigned int data[20000];

    int n_word = Event_Cycle(sock, data, header_size);
    if(n_word < 0) continue;
    ofs.write((char*)data, n_word*sizeof(unsigned int));


    // read TDC data
    if (!isTdcDisabled) {
      //      std::cout << "#D Event = " << EventNum << std::endl;
      std::vector<unsigned int> tdc_data;
      if (!Event_Cycle_tdc(sock, tdc_data)) break;
      int nword = tdc_data.size();
      unsigned int type     = (tdc_data[nword-2]>>28) & 0xf;
      unsigned int num_hits = tdc_data[nword-2] & 0xffff;
      // if (type == 0xa && num_hits != 4) {
      //   std::cout << "#D====================" << std::endl;
      //   std::cout << "#D tdc hit loss. event =  " << EventNum << " N hits = " << num_hits << std::endl;
      //   //        for (int id=header_size; id<nword-3; ++id) {
      //   for (int id=0; id<tdc_data.size(); ++id) {
      //     std::cout << "#D " << std::hex << tdc_data[id] << std::dec << std::endl;
      //     // unsigned int dtype = (tdc_data[id]>>28) & 0xf;
      //     // if (dtype == 0x0) {
      //     //   unsigned int ch = (tdc_data[id]>>24) & 0xf;
      //     //   std::cout << ch << std::endl;
      //     // }

      //   }
      //   std::cout << "#D====================" << std::endl;
      // }
      
      ofs.write(reinterpret_cast<char*>(&(tdc_data[0])), tdc_data.size() * sizeof(unsigned int));
    }


    
    if(EventNum%1000 == 0) std::cout << "Event: " << EventNum << std::endl;
    //std::cout << "Event: " << EventNum << std::endl;
    
    ++EventNum;
  }

  std::cout << "#D turn off DAQ mode" << std::endl;
  // Turn off DAQ mode
  fModule.WriteModule(DAQ::mid, DAQ::laddr_ExecDAQ, 0x0);
  std::cout << "#D user FIFO reset" << std::endl;
  // User reset
  event_builder.ResetFIFO();
  
  shutdown(sock, SHUT_RDWR);
  close(sock);

  //  ofs.close();
  std::cout << "#D : End of RUN: " << run_no << " , " << EventNum << " events" << std::endl;

  std::cout << "#D reset DRS4 addr" << std::endl;
  // Reset DRS4 addr
  drsMan.WriteDRSAddr(DRS::addr_mux_ch8);

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
  //    SiTCP_ADDR.sin_addr.s_addr = inet_addr(ip);
  SiTCP_ADDR.sin_addr.s_addr = getaddr_ipv4(ip);
  

  struct timeval tv;
  tv.tv_sec  = 3;
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
Event_Cycle(int sock, unsigned int* buffer, int header_size)
{
  // header read ---------------------------------------------------------
  unsigned int n_word_header = (unsigned int)header_size;

  static const unsigned int
    sizeHeader = sizeof(unsigned int)*n_word_header;
  int ret = receive(sock, (char*)buffer, sizeHeader);
  if(ret < 0) return -1;

  // std::cout << "#D Event_Cycle (header)" << std::endl;
  // std::for_each(buffer, buffer+n_word_header, hddaq::HexDump());
  
  // header read ---------------------------------------------------------
  unsigned int n_word_data = *(buffer+1) & 0xffff;
  unsigned int sizeData    = sizeof(unsigned int)*n_word_data;
  ret = receive(sock, (char*)(buffer+n_word_header), sizeData);
  if(ret < 0) return -1;

  // std::cout << "#D Event_Cycle (body)" << std::endl;
  // std::for_each(buffer+n_word_header,
  // 		buffer+n_word_header+n_word_data,
  // 		hddaq::HexDump());

  // std::cout << "#D Event_Cycle (ADC all)" << std::endl;
  // std::for_each(buffer, buffer+n_word_header+n_word_data, hddaq::HexDump());
  
  return n_word_header + n_word_data;
}

// Event Cycle for TDC ------------------------------------------------------------
bool
Event_Cycle_tdc(int sock, std::vector<unsigned int>& buffer)
{
  // header read ---------------------------------------------------------
  unsigned int n_word_event_header = 6;
  // ---------- event header ----------
  // magic                       0x40434454
  // local L1 event counter      0x5XXXXXXX
  // local L2 event counter      0x6XXXXXXX
  // L1 trigger time stamp       0x7XXXXXXX
  // L1 trigger time stamp (2nd) 0x7XXXXXXX
  // frame size                  0xCXXXXXXX
  // frame size of last frame    0xDXXXXXXX
  
  // tdc                         0x3XXXXXXX

  // ---------- event trailer ----------
  // error                     0x9XXXXXXX
  // number of hits in 1 event 0xAXXXXXXX
  // event trailer             0xBA3A3A3A

  static const unsigned int
    sizeHeader = sizeof(unsigned int)*n_word_event_header;
  buffer.resize(n_word_event_header);
  for (int nseq=0;;++nseq) {
    int ret = receive(sock, reinterpret_cast<char*>(&(buffer[0])), sizeHeader);
    if (nseq>1) std::cout << "#D tdc header read ??? " << std::endl;
    if(ret >= 0) break;
    else {
      std::cout << "#E Event_Cycle_tdc read error header" << std::endl;
      break;
    }
  }
  // std::cout << "#D Event_Cycle_tdc (header)" << std::endl;
  // std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());

  
  bool isLastFrame = false;
  // body read ---------------------------------------------------------
  for (int nseq=0;;++nseq) {
    unsigned int data_type   = (buffer.back()>>28) & 0xf;
    if (data_type != 0xc && data_type != 0xd) {
      int n = buffer.size();
      std::cout << "#E @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
                << " invalid data type for frame size "
                << " nseq = " << nseq << std::endl;
                // << std::hex << buffer.back()
                // << " " << buffer[n-2]
                // << " " << buffer[n-3]
                // << " : " << buffer[0]
                // << " " << buffer[1]
                // << " " << buffer[2]
                // << " " << buffer[3]
                // << " " << buffer[4]
                // << " " << buffer[5]
                // << std::dec << std::endl;
      std::cout << "#D Event_Cycle_tdc (all)" << std::endl;
      std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
      return false;
    }
    unsigned int frame_id    = (buffer.back()>>16) & 0xfff;
    unsigned int n_word_data = buffer.back() & 0xfff;
    isLastFrame              = (data_type == 0xd);

    // std::cout << "#D frame_id = " << frame_id
    //           << ", n word = " << n_word_data << "(" << std::hex << n_word_data << std::dec << ")"
    //           << ", last = " << isLastFrame << std::endl;
    
    int widx = buffer.size();
    //    if (!isLastFrame && (frame_id == 0)) n_word_data += 1;
    if (!isLastFrame) n_word_data += 1;
    //    else n_word_data -= 1;
    if (frame_id==0)  n_word_data -= n_word_event_header;
    else n_word_data -= 1;
    buffer.resize(widx + n_word_data);

    // std::cout << "#D frame_id = " << frame_id
    //           << ", n word = " << n_word_data << "(" << std::hex << n_word_data << std::dec << ")"
    //           << ", last = " << isLastFrame << std::endl;
    
    // std::cout << "#D n word to be read = " << n_word_data << std::endl;

    
    unsigned int sizeData = sizeof(unsigned int)*(n_word_data);
    //    std::cout << "#D n word =" << n_word_data << " to be read" << std::endl;
    for (;;) {
      usleep(1000);
      int ret = receive(sock, reinterpret_cast<char*>(&buffer[widx]), sizeData);
      //      std::cout << "#D Event_Cycle_tdc (tmp) " << n_word_data << " " << ret << std::endl;
      if (ret >=0) break;
      else {
        std::cout << "#E Event_Cycle_tdc read error body" << std::endl;
        break;
      }
    }

    // std::cout << "#D Event_Cycle_tdc (all)" << std::endl;
    // std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
    if (isLastFrame) break;
  }
  //  sleep(1);

  return isLastFrame;
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

