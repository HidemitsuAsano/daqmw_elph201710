// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef DRS4QDCREADER2_H
#define DRS4QDCREADER2_H

#include <vector>
#include <string>

class SiTcpRbcp;

//#include "Sock.h"
#include <daqmw/Sock.h>

#include "DaqComponentBase.h"

using namespace RTC;

class Drs4QdcReader2
  : public DAQMW::DaqComponentBase
{
public:
  Drs4QdcReader2(RTC::Manager* manager);
  ~Drs4QdcReader2();

  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
  TimedOctetSeq          m_out_data;
  OutPort<TimedOctetSeq> m_OutPort;

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
  int read_adc();
  int read_data_from_detectors();
  int read_tdc();
  int set_data(unsigned int data_byte_size);
  int write_OutPort();

  void changeDaqMode(bool on=true);
  unsigned int getHeaderSize();
  bool isTdcDisabled();
  void resetDrs4Address();
  void resetFIFO();

  DAQMW::Sock* m_sock; // socket for frontend data
  std::vector<unsigned int> m_data;
  std::vector<unsigned int> m_header;
  unsigned int m_recv_byte_size;

  BufferStatus m_out_status;
  bool m_debug;

  int m_srcPort;
  std::string m_srcAddr;
  std::string m_cmd_daqreg;
  SiTcpRbcp* m_rbcp;
  //  bool m_use_ext_trigger;

  int m_headerSize; // DAQ header size (number of int words)
  bool m_hasBoardId; // flag to add board ID into data
  bool m_isTdcDisabled;
  unsigned int m_boardId;

  unsigned int m_maxEvent;
  std::string m_cmd_open_gate;
  std::string m_cmd_close_gate;

};


extern "C"
{
  void Drs4QdcReader2Init(RTC::Manager* manager);
};

#endif // DRS4QDCREADER2_H

