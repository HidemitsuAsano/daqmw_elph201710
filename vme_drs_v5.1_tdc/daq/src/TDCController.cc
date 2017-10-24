#include "TDCController.hh"
#include "RegisterMap.hh"
#include "UDPRBCP.hh"
#include "network.hh"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <unistd.h>
#include <bitset>


static const std::string MyName  = "TDCController";
// maximum bin id
// expected bin id of fine time saturation 
//const unsigned int ft_saturation = 130;
const unsigned int max_ft        = 1000;

// -----------------------------------------------------------------------------
TDCController::TDCController(char* ip,
                             rbcp_header *rbcpHeader, 
                             int disp_mode)
  : FPGAModule(ip,
               udp_port,
               rbcpHeader,
               disp_mode)
{
}

// -----------------------------------------------------------------------------
TDCController::~TDCController()
{
}

// -----------------------------------------------------------------------------
void
TDCController::Disable(bool val)
{
  static const std::string MyFunc = "::Disable";
  val &= val;
  std::cout << "#D " << MyName << MyFunc << " " << val
            << " "
            << ((val==false) ? "TDC enabled" : "TDC disabled")
            << std::endl;
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_disable_tdc, val);
  usleep(100000);
}

// -----------------------------------------------------------------------------
unsigned int
TDCController::GetCDCBits(int ch,
                          int i_octet)
{
  static const std::string MyFunc = "::GetCDCBits";
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_edge_ch, ch & 0xff);
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_edge_oct, i_octet & 0xff);

  unsigned int ret = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_edge_data, 1);
  return ret & 0xff;
}

// -----------------------------------------------------------------------------
bool
TDCController::GetCDCBits(int ch,
                          std::vector<unsigned char>& bit_vector)
{
  int n = bit_vector.size();

  // disable external hit
  int val = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_enable_ch + ch, 1);
  val = val & 0x3e;
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_enable_ch + ch, val);

  int count = 0;
  // read data
  for (int i=0; i<n; ++i) {
    //bit_vector[i] =  GetCDCBits(ch, i);
    // std::bitset<8> octet(bit_vector[i]);
    unsigned int v =  GetCDCBits(ch, i);
    std::bitset<8> octet(v);
    std::cout << "i = " << std::setw(2) << i << " " << std::setw(10) << v << " " << std::setw(10) << octet << std::endl;
    count += octet.count();
  }
  std::cout << "#D count = " << count << std::endl;
  if (count !=2) std::cout << "#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ count = " << count << std::endl;
  // enable external hit
  val |= 1;
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_enable_ch + ch, val);
  
  return true;
}

// -----------------------------------------------------------------------------
bool
TDCController::IsDisabled()
{
  static const std::string MyFunc = "::IsDisabled";
  //  std::cout << "#D " << MyName << MyFunc << " " << std::endl;
  int val = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_disable_tdc, 1);
  bool ret = false;
  if ((val & 0x1) == 1) ret = true;
  std::cout << "#D " << MyName << MyFunc << " " << ret
            << " " << ((ret == false) ? "TDC enabled" : "TDC disabled")
            << std::endl;
  return ret;
}

// -----------------------------------------------------------------------------
void
TDCController::PeekLUT(int ch,
                       int nbins, 
                       int verbose)
{
  static const std::string MyFunc = "::PeekLUT ";

  if (ch<0){
    std::cerr << "#E : " << MyName << MyFunc << "invalid channel ID" << std::endl;
    return;
  }
  //  usleep(1);
  // clear register before setting channel id
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_lut, 0);
  //  usleep(1);
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_ch, (ch & 0xff));
  //  usleep(1);
  
  std::vector<unsigned int> data;

  std::cout << "========================================" << std::endl;
  std::cout << "#D " << MyName << MyFunc << " ch = " << std::setw(3) << ch << " ";
  for (int ib=0; ib<nbins; ++ib){
    // clear register before setting bin id 
    FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_lut, 0);
    //    usleep(1);
    
    // set bin id
    FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_bin_id0, (ib & 0xff));
    //usleep(1);
    FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_bin_id1, ((ib>>8) & 0xff));
    //usleep(1);
    
    // start to peek
    FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_lut, 1);
    //usleep(1);

    // read lower 1 byte
    unsigned int ftime0 = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_peek_ftime0, 1);
    //usleep(1);
    unsigned int ftime1 = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_peek_ftime1, 1);
    //usleep(1);
    unsigned int ftime = (ftime1<<8) | ftime0;

    data.push_back(ftime);
    if (ib%8==0) {
      std::cout << ".";
      std::cout.flush();
    }
  }
  std::cout << std::endl;
  // clear register after peeking
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_peek_lut, 0);



  

  // bool isGood = false;
  // for (unsigned int ib=0; ib<nbins; ++ib){
  //   if ((ib > ft_saturation) && (data[ib] > max_ft)) isGood = true;
  // }
  // if (!isGood){
  //   std::cout << "#E : bad calibration " << std::endl;
  // }

  
  // print ------------------------------------------------------------
  if (verbose == 1) {
    //    std::cout << "#D " <<  MyName << MyFunc << " ch = " << ch << std::endl;
    //  std::cout << "# "  <<  std::setw(10) << "bin " << std::setw(10) << "value" << std::endl;
    for (int ib=0; ib<nbins; ib+=8){
      std::cout << std::setw(10) << ib << " : ";
      for (unsigned int j=0; j<8; ++j){
        std::cout << " " << std::setw(10) << data[ib+j];
      }
      std::cout << std::endl;
    }
  }
  
}

// -----------------------------------------------------------------------------
void
TDCController::SetBufferAlmostFullThreshold(int th_assert,
                                            int th_negate,
                                            int laddr_assert_begin,
                                            int laddr_assert_end,
                                            int laddr_negate_begin,
                                            int laddr_negate_end,
                                            const std::string& funcName)
{
  std::cout << "#D " << funcName
            << " " << std::dec << th_assert
            << " (hex: " << std::hex << th_assert << std::dec << ")"
            << " " << std::dec << th_negate
            << " (hex: " << std::hex << th_negate << std::dec << ")"
            << std::endl;
  for (int i=laddr_assert_begin; i<=laddr_assert_end; ++i) {
    unsigned int v = (th_assert >> (8*(i-laddr_assert_begin))) & 0xff;
    
    // std::cout << "#D write addr =  " << std::hex << i
    //           << " var = " << v
    //           << std::dec << std::endl;
    
    FPGAModule::WriteModule(TDC::mid, i, v);
    usleep(100000);
  }
  for (int i=laddr_negate_begin; i<=laddr_negate_end; ++i) {
    unsigned int v = (th_negate >> (8*(i-laddr_negate_begin))) & 0xff;
    // std::cout << "#D write addr =  " << std::hex << i
    //           << " var = " << v
    //           << " " << th_negate
    //           << std::dec << std::endl;
    
    FPGAModule::WriteModule(TDC::mid, i, v);
    usleep(100000);
  }
  
}

// -----------------------------------------------------------------------------
void
TDCController::SetChannelBufferAlmostFullThreshold(int th_assert,
                                                   int th_negate)
{
  static const std::string MyFunc
    = MyName + "::SetChannelBufferAlmostFullThreshold";

  SetBufferAlmostFullThreshold(th_assert, th_negate,
                               TDC::laddr_ch_th_assert,
                               TDC::laddr_ch_th_assert,
                               TDC::laddr_ch_th_negate,
                               TDC::laddr_ch_th_negate,
                               MyFunc);
}

// -----------------------------------------------------------------------------
void
TDCController::SetL1BufferAlmostFullThreshold(int th_assert,
                                              int th_negate)
{
  static const std::string MyFunc
    = MyName + "::SetL1BufferAlmostFullThreshold";

  if (th_assert%2 == 1) th_assert = std::min(th_assert+1, 254);
  if (th_negate%2 == 1) th_negate = std::max(th_negate-1, 0);

  SetBufferAlmostFullThreshold(th_assert, th_negate,
                               TDC::laddr_l1buf_th_assert0,
                               TDC::laddr_l1buf_th_assert1,
                               TDC::laddr_l1buf_th_negate0,
                               TDC::laddr_l1buf_th_negate1,
                               MyFunc);

}

// -----------------------------------------------------------------------------
void
TDCController::SetL1Delay(int val)
{
  static const std::string MyFunc = "::SetLDelay";
  std::cout << "#D " << MyName << MyFunc << " " << val << std::endl;
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_l1_delay0, (val & 0xff));
  usleep(100000);
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_l1_delay1, ((val>>8) & 0xff));
  usleep(100000);
}

// -----------------------------------------------------------------------------
void
TDCController::SetL1TriggerFIFOAlmostFullThreshold(int th_assert,
                                                   int th_negate)
{
  return SetReferenceChannelBufferAlmostFullThreshold(th_assert, th_negate);
}

// -----------------------------------------------------------------------------
void
TDCController::SetL2BufferAlmostFullThreshold(int th_assert,
                                              int th_negate)
{
  static const std::string MyFunc
    = MyName + "::SetL2BufferAlmostFullThreshold";

  SetBufferAlmostFullThreshold(th_assert, th_negate,
                               TDC::laddr_l2buf_th_assert0,
                               TDC::laddr_l2buf_th_assert1,
                               TDC::laddr_l2buf_th_negate0,
                               TDC::laddr_l2buf_th_negate1,
                               MyFunc);

}

// -----------------------------------------------------------------------------
void
TDCController::SetL2FrameIndicatorFIFOAlmostFullThreshold(int th_assert,
                                                          int th_negate)
{
  static const std::string MyFunc
    = MyName + "::SetL2FrameIndicatorAlmostFullThreshold";

  SetBufferAlmostFullThreshold(th_assert, th_negate,
                               TDC::laddr_l2fi_th_assert0,
                               TDC::laddr_l2fi_th_assert1,
                               TDC::laddr_l2fi_th_negate0,
                               TDC::laddr_l2fi_th_negate1,
                               MyFunc);

}

// -----------------------------------------------------------------------------
void
TDCController::SetMeasurementMode(int ch,
                                  int mode)
{
  static const std::string MyFunc
    = MyName + "::SetMeasurementMode";
  std::cout << "#D " << MyName << MyFunc << " " << ch << " " ;
  if (mode == TDC::kMeasurementTrailing)
    std::cout << " trailing edge" << std::endl;
  else if (mode == TDC::kMeasurementVeryHighResolution)
    std::cout << " very high resolution" << std::endl;
  else
    std::cout << " unknown parameter " << mode << std::endl;

  // get present setting
  int val = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_enable_ch + ch, 1);
  //  std::cout << "#D before = " << std::hex << val << std::dec << " (hex)" << std::endl;

  // lower 4 bits are kept. 
  val = (val & 0xf) | ((mode & 0x3) << 4);
  //  std::cout << "#D value  = " << std::hex << val << std::dec << " (hex)" << std::endl;
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_enable_ch + ch, val);

  val = FPGAModule::ReadModule_nCycle(TDC::mid, TDC::laddr_enable_ch + ch, 1);
  //  std::cout << "#D after  = " << std::hex << val << std::dec << " (hex)" << std::endl;
}

// -----------------------------------------------------------------------------
void
TDCController::SetReadOutFIFOAlmostFullThreshold(int th_assert,
                                                 int th_negate)
{
  static const std::string MyFunc
    = MyName + "::SetReadOutFIFOAlmostFullThreshold";

  SetBufferAlmostFullThreshold(th_assert, th_negate,
                               TDC::laddr_rofifo_th_assert0,
                               TDC::laddr_rofifo_th_assert1,
                               TDC::laddr_rofifo_th_negate0,
                               TDC::laddr_rofifo_th_negate1,
                               MyFunc);
}

// -----------------------------------------------------------------------------
void
TDCController::SetReferenceChannelBufferAlmostFullThreshold(int th_assert,
                                                            int th_negate)
{
  static const std::string MyFunc
    = MyName + "::SetReferenceChannelBufferAlmostFullThreshold";

  SetBufferAlmostFullThreshold(th_assert, th_negate,
                               TDC::laddr_trig_th_assert,
                               TDC::laddr_trig_th_assert,
                               TDC::laddr_trig_th_negate,
                               TDC::laddr_trig_th_negate,
                               MyFunc);
}

// -----------------------------------------------------------------------------
void
TDCController::SetRejectLatency(int val)
{
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_reject_latency0, val & 0xff);
  usleep(100000);
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_reject_latency1, (val>>8) & 0xff);
  usleep(100000);
}

// -----------------------------------------------------------------------------
void
TDCController::SetWindowOffset(int val)
{
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_window_offset0, val & 0xff);
  usleep(100000);
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_window_offset1, (val>>8) & 0xff);
  usleep(100000);
}

// -----------------------------------------------------------------------------
void
TDCController::SetSearchWindow(int val)
{
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_search_window0, val & 0xff);
  usleep(100000);
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_search_window1, (val>>8) & 0xff);
  usleep(100000);
}

// -----------------------------------------------------------------------------
void
TDCController::StartCalibration()
{
  static const std::string MyFunc = "::StartCalibration";
  std::cout << "#D " << MyName << MyFunc << std::endl;
  FPGAModule::WriteModule(TDC::mid, TDC::laddr_start_calibration, 1);
  for (int i=0; i<1000; ++i) {
    std::cout << ".";
    std::cout.flush();
    usleep(60);
  }
  std::cout << std::endl;
}
