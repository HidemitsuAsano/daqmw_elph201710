// -*- C++ -*-
#ifndef register_config_hh
#define register_config_hh

#include "common_val.hh"
#include "RegisterMap.hh"

namespace register_config {
  //______________________________________________________________________________
  // DRS4 and EVB (event builder) read count
  // must be even number
  // min: 2
  // max: 1024 
  //______________________________________________________________________________
  // const unsigned int kReadCount = 1024;
  //static const unsigned int kReadCount = 1022;
  static const unsigned int kReadCount = 1024;


  
  //______________________________________________________________________________
  // Select OR in LTM (local trigger manager)
  // enable flag bits for local trigger input
  //______________________________________________________________________________
  static const unsigned short kLTMEnableDiscri = 0xffff;
  



  //______________________________________________________________________________
  // --------------------------------------------------
  // Select global trigger in MTM (master trigger manager)
  //______________________________________________________________________________
  static const unsigned int kMTMSelectTrigger = MTM::reg_L1ext;
  // = MTM::reg_L1local;
  // = MTM::reg_L1clk;
  // = MTM::reg_L1ext | MTM::reg_L2ext | MTM::reg_clrext | MTM::reg_enl2;
  // = MTM::reg_L1ext | MTM::reg_L2ext | MTM::reg_enl2;





  //______________________________________________________________________________
  // set MTM delay
  // (1 bit = 10 nsec)
  // RoI reading starts after the specified delay from the L1 trigger arrival.
  //______________________________________________________________________________
  // const unsigned int kDelay = 48;
  static const unsigned int kMTMDelay = 50;



  
  //______________________________________________________________________________
  // MTM freq
  // frequency of internal clock trigger
  //______________________________________________________________________________
  static const unsigned int kMTMClockFrequency = MTM::reg_freq_100hz;
  // const unsigned int kMTMFreq = MTM::reg_freq_10hz;
  // const unsigned int kMTMFreq = MTM::reg_freq_1hz;




  //______________________________________________________________________________
  // DAQ mode, EVB control
  //______________________________________________________________________________
  static const unsigned short kEVBControl  = EVB::c_reverse_window;
  // = EVB::c_reverse_window |
  //   EVB::c_noise_filter   |
  //   EVB::c_zs_wf          | 
  //   EVB::c_zs_QDC         |
  //   EVB::c_short_header;




  //______________________________________________________________________________
  // EVB enable waveform flag bits
  //______________________________________________________________________________
  static const unsigned short kEVBEnableWaveForm = 0xffff;




  //______________________________________________________________________________
  // double data flag
  // 1 = treat one sample point as 12 bit and pack 2 sample points into one 32-bit word
  // 0 = apply moving average filter to raw data, treat one sample point as 14 bit and pack 1 sample point into one 32-bit word
  //______________________________________________________________________________
  static const unsigned short kEVBDoubleData = 1; // 12 bit * 2 in 32-bit word
  // static const unsigned short kEVBDoubleData = 0; // 14 bit * 1 in 32-bit word


  
  //______________________________________________________________________________
  // cascade threshold
  //______________________________________________________________________________
  // static unsigned short kEVBCascadeThreshold1 = 767;
  // static unsigned short kEVBCascadeThreshold2 = 1023;
  static const unsigned short kEVBCascadeThreshold1 = 765;
  static const unsigned short kEVBCascadeThreshold2 = 1021;
  // static unsigned short kEVBCascadeThreshold1 = 763;
  // static unsigned short kEVBCascadeThreshold2 = 1019;



  //______________________________________________________________________________
  // noise (spike) filter threshold
  //______________________________________________________________________________
  static const unsigned short kNoiseFilterThresholdHigh[NofChModule]
  = {350, 350, 350, 350,
     350, 350, 350, 350,
     350, 350, 350, 350,
     350, 350, 350, 350};
  static const unsigned short kNoiseFilterThresholdLow[NofChModule]
  = {0, 0, 0, 0,
     0, 0, 0, 0,
     0, 0, 0, 0};

  //______________________________________________________________________________
  // zero suppress threshold
  //______________________________________________________________________________
  static const unsigned short kZeroSuppressThreshold[NofChModule]
  = {1000, 1000, 1000, 1000,
     1000, 1000, 1000, 1000,
     1000, 1000, 1000, 1000};
  

  //______________________________________________________________________________
  // busy setting
  //______________________________________________________________________________
  static const unsigned int kDAQBusy
  = DAQ::reg_self |
    DAQ::reg_DRS  |
    DAQ::reg_EVB  |
    DAQ::reg_TDC;
  

  //______________________________________________________________________________
  // NIM output 
  //______________________________________________________________________________
  static const unsigned short kOMNNimOut1 = OMN::reg_busy;
  static const unsigned short kOMNNimOut2 = OMN::reg_busy;

}



#endif
