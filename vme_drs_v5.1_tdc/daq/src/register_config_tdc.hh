// -*- C++ -*-

#ifndef register_config_tdc_hh
#define register_config_tdc_hh

#include "RegisterMap.hh"

namespace register_config_tdc {

  // ______________________________________________________________________________
  static const unsigned short kMeasurementMode = TDC::kMeasurementTrailing;

  static const unsigned short kL1Delay       = 0x000;
  // static const unsigned short kRejectLatency = 0x060; // 2.048 usec
  // static const unsigned short kWindowOffset  = 0x030; // 1.024 usec
  // static const unsigned short kSearchWindow  = 0x030;

  // from 2017/10/19 12:43 (TNT)
  // LSB = 21.333 nsec
  // static const unsigned short kRejectLatency = 0x01A; // 554.66 ns
  // static const unsigned short kWindowOffset  = 0x010; // 341.3  ns
  // static const unsigned short kSearchWindow  = 0x00d; // 277    ns

  // from 2017/10/19 14:50
  static const unsigned short kRejectLatency = 0x014; 
  static const unsigned short kWindowOffset  = 0x006;
  static const unsigned short kSearchWindow  = 0x004;


  static const unsigned short kL1TriggerFIFOAlmostFullAssert = 30;
  static const unsigned short kL1TriggerFIFOAlmostFullNegate = 16;

  static const unsigned short kL1BufferAlmostFullAssert = 0xFA;
  static const unsigned short kL1BufferAlmostFullNegate = 0xE0;

  static const unsigned short kROFIFOAlmostFullAssert = 0xF0; // 240
  static const unsigned short kROFIFOAlmostFullNegate = 0x64; // 100

  static const unsigned short kL2FrameIndicatorFIFOAlmostFullAssert = 0xF0;
  static const unsigned short kL2FrameIndicatorFIFOAlmostFullNegate = 0xF0;

  static const unsigned short kL2BufferAlmostFullAssert = 0xFF0; // 4080
  static const unsigned short kL2BufferAlmostFullNegate = 0x800; // 2048
  
}



#endif
