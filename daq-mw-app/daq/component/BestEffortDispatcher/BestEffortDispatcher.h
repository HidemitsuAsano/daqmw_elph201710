// -*- C++ -*-
/*!
 * @file BestEffortDispatcher.h
 * @brief
 * @date Oct.5th, 2017
 * @author Hidemitsu Asano
 * @email hidemitsu.asnao@riken.jp
 *
 */

#ifndef BESTEFFORTDISPATCHER_H
#define BESTEFFORTDISPATCHER_H

#include "DaqComponentBase.h"
#include "CAENVMEtypes.h"
#include "CAENv1718.h"
#include "CAENModule.h"

using namespace RTC;

class BestEffortDispatcher
  : public DAQMW::DaqComponentBase
{
public:
  BestEffortDispatcher(RTC::Manager* manager);
  ~BestEffortDispatcher();

  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
  //Inport for NIMEASIROC 1 
  TimedOctetSeq          m_in0_data;
  InPort<TimedOctetSeq>  m_InPort0;
  
  //Inport for NIMEASIROC 2
  TimedOctetSeq          m_in1_data;
  InPort<TimedOctetSeq> m_InPort1;

  //Inport for DRS4Qdc 1 
  TimedOctetSeq          m_in2_data;
  InPort<TimedOctetSeq>  m_InPort2;
  
  //Inport for DRS4Qdc 2
  TimedOctetSeq          m_in3_data;
  InPort<TimedOctetSeq> m_InPort3;

  //Inport for HulScaler 
  TimedOctetSeq          m_in4_data;
  InPort<TimedOctetSeq> m_InPort4;
  
  //built data 
  TimedOctetSeq          m_in_data_sum;

  //outport for logger
  TimedOctetSeq          m_out0_data;
  OutPort<TimedOctetSeq> m_OutPort;
  
  //outport for online monitor
  TimedOctetSeq          m_out1_data;
  OutPort<TimedOctetSeq> m_BestEffort_OutPort;
   

private:
  int daq_dummy();
  int daq_configure();
  int daq_unconfigure();
  int daq_start();
  int daq_run();
  int daq_stop();
  int daq_pause();
  int daq_resume();

  int parse_params(::NVList* list);
  int read_data_from_detectors();
  //set data for logger 
  int set_data_OutPort(unsigned int data_byte_size);
  //set data for Monitor
  int set_data_BestEffort_OutPort(unsigned int data_byte_size);
  int reset_InPort0();
  int reset_InPort1();
  int reset_InPort2();
  int reset_InPort3();
  int reset_InPort4();
  unsigned int read_InPort0();
  unsigned int read_InPort1();
  unsigned int read_InPort2();
  unsigned int read_InPort3();
  unsigned int read_InPort4();
  int write_OutPort();//DAQLogger 
  int write_BestEffort_OutPort();//Monitor

  // static const int SEND_BUFFER_SIZE = 4096;
  // unsigned char m_data[SEND_BUFFER_SIZE];
  static const int NINMODULE = 5;

  BufferStatus m_in0_status;
  BufferStatus m_in1_status;
  BufferStatus m_in2_status;
  BufferStatus m_in3_status;
  BufferStatus m_in4_status;

  BufferStatus m_out_status;
  BufferStatus m_besteffort_out_status;

  unsigned int m_in_tout_counts[NINMODULE];
  unsigned int m_out0_tout_counts;
  unsigned int m_out1_tout_counts;

  unsigned int m_inport_recv_data_size[NINMODULE];
  unsigned int m_inport_recv_data_size_sum;
  
  //CAEN bus controller
  CAENv1718* m_v1718;
  int32_t m_Handle;
  
  int m_Verbosity;
  
};


extern "C"
{
  void BestEffortDispatcherInit(RTC::Manager* manager);
};

#endif // BESTEFFORTDISPATCHER_H
