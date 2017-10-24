// -*- C++ -*-

#include <iostream>
#include <iomanip>

#include "UDPRBCP.hh"
#include "rbcp.h"
#include "TDCController.hh"

#ifdef TDC_CH_BUF
const unsigned int kBufLen = 128;
const unsigned int kBitMask = 0x7f;
#endif
#ifdef TDC_TRIG_BUF
const unsigned int kBufLen = 128;
const unsigned int kBitMask = 0x7f;
#endif
#ifdef TDC_L1BUF
const unsigned int kBufLen = 256;
const unsigned int kBitMask = 0xff;
#endif
#ifdef TDC_ROFIFO
const unsigned int kBufLen = 256;
const unsigned int kBitMask = 0xff;
#endif
#ifdef TDC_L2BUF
const unsigned int kBufLen = 2048;
const unsigned int kBitMask = 0x07ff;
#endif

//______________________________________________________________________________
int
printUsage(char* prog, int buf_len){
  std::cout << "Usage\n";
  std::cout << prog << " [ip address] [threshold assert (0--" << buf_len
            << ")] [threshold negate (0--" << buf_len << ")]"
            << std::endl;
  return 0;
}


//______________________________________________________________________________
int main(int argc, char* argv[]){

  
  if (4 != argc){
    return printUsage(argv[0], kBufLen-1);
  }

  char* board_ip  = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  int th_assert = 0;
  int th_negate = 0;

  th_assert = atoi(argv[2]) & kBitMask;
  th_negate = atoi(argv[3]) & kBitMask;

  TDCController tdcCon(board_ip, &rbcpHeader, 0);


#ifdef TDC_CH_BUF  
  tdcCon.SetChannelBufferAlmostFullThreshold(th_assert, th_negate);
#endif
#ifdef TDC_TRIG_BUF
  tdcCon.SetReferenceChannelBufferAlmostFullThreshold(th_assert, th_negate);
#endif
#ifdef TDC_L1BUF  
  tdcCon.SetL1BufferAlmostFullThreshold(th_assert, th_negate);
#endif
#ifdef TDC_ROFIFO  
  tdcCon.SetReadOutFIFOAlmostFullThreshold(th_assert, th_negate);
#endif
#ifdef TDC_L2BUF 
  tdcCon.SetL2BufferAlmostFullThreshold(th_assert, th_negate);
#endif
  
  
  return 0;
}
