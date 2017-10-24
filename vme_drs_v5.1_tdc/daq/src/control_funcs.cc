#include <algorithm>

#include"control_funcs.hh"
#include"ParameterManager.hh"
#include"common_val.hh"

#include "register_config.hh"
#include "register_config_tdc.hh"

static const std::string kDefaultFileName("./register/LTC2656-12-default.txt");


//______________________________________________________________________________

// Module Initialize ------------------------------------------------
void
module_initialize(char* ip, rbcp_header *rbcpHeader, const std::string& file_name)
{
  std::string fname(file_name);
  if (fname.empty()) fname = kDefaultFileName;
  std::ifstream ifile(fname.c_str());
  if (ifile.fail()) {
    std::cerr << "#E input file doesn't exist. file name = " << fname << std::endl;
    std::exit(1);
  }
  
  dac_spi(ip, rbcpHeader, fname.c_str());
  adc_spi(ip, rbcpHeader);
  drs_init(ip, rbcpHeader);
  //  nimout(ip, rbcpHeader);
  system_reg(ip, rbcpHeader);
  daq_reg(ip, rbcpHeader);
}

// Interactive rbcp -------------------------------------------------
void
interactive_rbcp(char* ip, rbcp_header *rbcpHeader)
{
  CommandMan cMan(ip, udp_port, rbcpHeader);
  do{
    ++rbcpHeader->id;
  }while(cMan.parse_rbcp_command() > -1);
  
  return;
}

// dac_spi ----------------------------------------------------------
void
dac_spi(char* ip, rbcp_header *rbcpHeader, const char* filename)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  LTC2656_12_SPI dacMan(fModule);
  dacMan.Load(filename);

  return;
}

// adc_spi ----------------------------------------------------------
void
adc_spi(char* ip, rbcp_header *rbcpHeader)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  AD9637_SPI adcMan(fModule);
  adcMan.InitializeDevices();
  //  sleep(1);
  //  adcMan.SetTestMode(9);

  return;
}

// drs_spi ----------------------------------------------------------
void
drs_init(char* ip, rbcp_header *rbcpHeader)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  DRS4Controller drsMan(fModule);
  if(!drsMan.InitializeDevices()) std::exit(-1);

  return;
}

// lut_load ---------------------------------------------------------
void
lut_load(char* ip, rbcp_header *rbcpHeader, const char* filename)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  EventBuilder event(fModule);
  event.WriteOfsTable(filename);

  return;
}

// blcor1_load ------------------------------------------------------
void
blcor1_load(char* ip, rbcp_header *rbcpHeader)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  EventBuilder event(fModule);
  event.WriteBlCor1Table();

  return;
}

// blcor2_load ------------------------------------------------------
void
blcor2_load(char* ip, rbcp_header *rbcpHeader)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  EventBuilder event(fModule);
  event.WriteBlCor2Table();

  return;
}

// set_utput --------------------------------------------------------
void
nimout(char* ip, rbcp_header *rbcpHeader, int out1, int out2)
{
  if(out1 >= OMN::sizeOutSignal){
    std::cout << "#E : set_output Invalid register out1"
	      << out1 << std::endl;
    return;
  }

  if(out2 >= OMN::sizeOutSignal){
    std::cout << "#E : set_output Invalid register out2"
	      << out2 << std::endl;
    return;
  }

  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  fModule.WriteModule(OMN::mid, OMN::laddr_out1, out1);
  fModule.WriteModule(OMN::mid, OMN::laddr_out2, out2);
  std::cout << "#D : set output registers (out1, out2) = "
	    << out1 << ", " << out2
	    << std::endl;

  return;
}

// system_reg -------------------------------------------------------
void
system_reg(char* ip, rbcp_header *rbcpHeader)
{
  static const int fifo_we_delay = 28;

  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  fModule.WriteModule(EVB::mid, EVB::laddr_we_delay, fifo_we_delay);
  fModule.WriteModule(DAQ::mid, DAQ::laddr_EnBusy,
		      DAQ::reg_self | DAQ::reg_DRS | DAQ::reg_EVB | DAQ::reg_TDC
		      );
}

// daq_reg ----------------------------------------------------------
void
daq_reg(char* ip, rbcp_header *rbcpHeader)
{
  using namespace register_config;
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);

  unsigned int read_count = kReadCount;
  std::cout << "#D set read count " << read_count << std::endl;
  if (read_count%2 != 0) {
    std::cout << "#D read_count must be EVEN. "
              << read_count << " -> ";
    ++read_count;
    std::cout << read_count << std::endl;
  }
  if (read_count>1024) {
    std::cout << "#D read_count must be less than 1024. "
              << read_count << " -> 1024" << std::endl;
    read_count = 1024;
  }
  set_read_count(ip, rbcpHeader, read_count);

  // Set OR in LTM
  std::cout << "#D set LTM " << std::endl;
  fModule.WriteModule(LTM::mid, LTM::laddr_en_discri, kLTMEnableDiscri);

  // Set global trigger in MTM
  std::cout << "#D set MTM " << std::endl;
  fModule.WriteModule(MTM::mid, MTM::laddr_sel_trig, kMTMSelectTrigger);
    
  std::cout << "#D set delay " << std::endl;
  fModule.WriteModule(MTM::mid, MTM::laddr_delay, kMTMDelay);

  std::cout << "#D set clock frequency " << std::endl;
  set_clktrig_freq(ip, rbcpHeader, kMTMClockFrequency);

  // DAQ mode
  EventBuilder event_builder(fModule);
  event_builder.WriteRegister(EVB::laddr_control, 0);
  event_builder.WriteRegister(EVB::laddr_control, kEVBControl);
  //  unsigned short en_wf = 0;
  unsigned short en_wf = 0xffff;
  event_builder.WriteRegister(EVB::laddr_en_wf, kEVBEnableWaveForm);
  event_builder.WriteRegister(EVB::laddr_ddata, kEVBDoubleData);

  unsigned int size_event_buf = 2048;
  if (en_wf>0) size_event_buf /= (read_count+1);
  
  std::cout << "#D : multi-event buffer. n = "
            << size_event_buf << " events" << std::endl;
  
  if (en_wf>0) {
    if ((read_count+1)*size_event_buf > 2048) {
    std::cerr << "#E : daq_reg Invalid multi-event buffer size "
	      << size_event_buf << std::endl;
    std::cout << "#D : modified : "
              << size_event_buf << " -> ";
    size_event_buf = 2048/(read_count+1);
    //    std::cout << size_event_buf << std::endl;
    }
  } else if (size_event_buf > 2048) {
    size_event_buf = 2048;
  }
  event_builder.WriteRegister(EVB::laddr_nevent, size_event_buf);


  set_cascade_th(ip, rbcpHeader, kEVBCascadeThreshold1, kEVBCascadeThreshold2);

  noise_filter_th(ip, rbcpHeader);
  zero_suppress_th(ip, rbcpHeader);

  // busy setting
  fModule.WriteModule(DAQ::mid, DAQ::laddr_EnBusy, kDAQBusy);

  // NIM output
  nimout(ip, rbcpHeader, kOMNNimOut1, kOMNNimOut2);
  
  // TDC setting
  {
    using namespace register_config_tdc;
    
    TDCController tdcCon(ip, rbcpHeader, 0);
    // edge select
    for (int ich=0; ich<NofChModule; ++ich) {
      tdcCon.SetMeasurementMode(ich, kMeasurementMode);
    }

    tdcCon.SetL1Delay(kL1Delay);
    tdcCon.SetRejectLatency(kRejectLatency);
    tdcCon.SetWindowOffset(kWindowOffset); 
    tdcCon.SetSearchWindow(kSearchWindow);

    tdcCon.SetL1TriggerFIFOAlmostFullThreshold(kL1TriggerFIFOAlmostFullAssert,
                                               kL1TriggerFIFOAlmostFullNegate);

    tdcCon.SetL1BufferAlmostFullThreshold(kL1BufferAlmostFullAssert,
                                          kL1BufferAlmostFullNegate);

    tdcCon.SetReadOutFIFOAlmostFullThreshold(kROFIFOAlmostFullAssert,
                                             kROFIFOAlmostFullNegate);


    tdcCon.SetL2FrameIndicatorFIFOAlmostFullThreshold(kL2FrameIndicatorFIFOAlmostFullAssert,
                                                      kL2FrameIndicatorFIFOAlmostFullNegate);
    

    tdcCon.SetL2BufferAlmostFullThreshold(kL2BufferAlmostFullAssert,
                                          kL2BufferAlmostFullNegate);

    tdcCon.Disable(false);
    //tdcCon.Disable(true);
  }
  
  return;
}

// noise filter th --------------------------------------------------
void
noise_filter_th(char* ip, rbcp_header *rbcpHeader)
{
  std::vector<unsigned int> th_high(NofChModule);
  std::vector<unsigned int> th_low(NofChModule);
  for(int i = 0; i<NofChModule; ++i){
    th_high[i] = register_config::kNoiseFilterThresholdHigh[i];
    th_low[i]  = register_config::kNoiseFilterThresholdLow[i];
  }

  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  EventBuilder event_builder(fModule);
  event_builder.WriteNfTh(th_high, th_low);
}

// zero suppress th -------------------------------------------------
void
zero_suppress_th(char* ip, rbcp_header *rbcpHeader)
{
  std::vector<unsigned int> th_zs(NofChModule);
  for(int i = 0; i<NofChModule; ++i){
    th_zs[i] = register_config::kZeroSuppressThreshold[i];
  }

  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  EventBuilder event_builder(fModule);
  event_builder.WriteZsTh(th_zs);
}

// debug ------------------------------------------------------------
void
debug(char* ip, rbcp_header *rbcpHeader)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  fModule.WriteModule(BCT::mid, BCT::laddr_Reset, 0x1);

  return;
}

// set_read_count ---------------------------------------------------
void
set_read_count(char* ip, rbcp_header *rbcpHeader, int count)
{
  if(count%2 != 0){
    std::cerr << "#E : set_read_count Read count must be EVEN." 
	      << std::endl;
    return;
  }

  int drs_read_count   = count -1;
  int fifo_write_count = count/2;
  //  int fifo_write_count = count -1;

  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);  
  DRS4Controller drsMan(fModule);
  drsMan.WriteRegister(DRS::laddr_maxcount, 0, drs_read_count);

  EventBuilder evbMan(fModule);
  evbMan.WriteRegister(EVB::laddr_w_count, fifo_write_count);

  return;
}

// set_clktrig_freq -------------------------------------------------
void
set_clktrig_freq(char* ip, rbcp_header *rbcpHeader, unsigned int reg)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  for(int i = 0; i<4; ++i){
    unsigned int buf = (reg >> 8*i) & 0xff;
    fModule.WriteModule(MTM::mid, MTM::laddr_clk_freq+i, buf);
  }

  std::cout << std::hex << "#D : set_clktrig_freq is " << 
    fModule.ReadModule_nCycle(MTM::mid, MTM::laddr_clk_freq, 4)
	    << std::dec << std::endl;
}

// ----------------------------------------------------------------------
void
set_cascade_th(char* ip, rbcp_header *rbcpHeader,
               unsigned int th1, unsigned int th2)
{
  FPGAModule fModule(ip, udp_port, rbcpHeader, 0);
  static const unsigned int kshift = 10;
  unsigned int th = (th2 << kshift) + th1;
  std::cout << "#D : set_cascade_th " << th1 << " " << th2 << std::endl;
  fModule.WriteModule(EVB::mid, EVB::laddr_cascade, th);
}

// ----------------------------------------------------------------------
void
tdc_peek_lut(char* ip, rbcp_header *rbcpHeader, int ch, int nbins)
{
  if (ch < 0){
    std::cerr << "#E : tdc_peek_lut Invalid channel ID" << std::endl;
    return;
  }

  TDCController tdcMan(ip, rbcpHeader, 0);
  tdcMan.PeekLUT(ch, nbins);
}

// -----------------------------------------------------------------------------
void
tdc_start_calibration(char* ip, rbcp_header *rbcpHeader)
{
  TDCController tdcMan(ip, rbcpHeader, 0);
  tdcMan.StartCalibration();
}

