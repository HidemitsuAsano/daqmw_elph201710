// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include <vector>

#include "DaqComponentBase.h"

////////// ROOT Include files //////////
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TApplication.h"

#include "NIMEASIROCData.h"


using namespace RTC;

class THttpServer;

class Monitor
  : public DAQMW::DaqComponentBase
{
public:
  Monitor(RTC::Manager* manager);
  ~Monitor();

  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
  TimedOctetSeq          m_in_data;
  InPort<TimedOctetSeq>  m_InPort;

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
  int reset_InPort();

  unsigned int read_InPort();


  void parse_file(const std::string& fileName="");
  void update();

  BufferStatus m_in_status;
  int m_Verbosity;
  
  //DRS4
  unsigned int m_t0_ch;
  bool m_has_tdc_data;
  std::string m_conf_file_name;//not used so far

  //functions for NIM-EASIROC (NER)
  unsigned int NERdecode_header(const unsigned char* mydata);
  unsigned int NERdecode_data(const unsigned char* mydata);
  unsigned int NERunpackBigEndian32(const unsigned char* array4byte);
  bool NERisAdcHg(unsigned int data);
  bool NERisAdcLg(unsigned int data);
  bool NERisTdcLeading(unsigned int data);
  bool NERisTdcTrailing(unsigned int data);
  bool NERisScaler(unsigned int data);
  unsigned int NERDecode32bitWord(unsigned int word32bit);
  void clearhists();
  void resetHists();
  
  //make 2 data struct for 2 module operation
  //struct NIMEASIROCData m_NIMEASIROCData1;
  //struct NIMEASIROCData m_NIMEASIROCData2;
  //histograms for NIM-EASIROCs
  //implementation for QA of low level info.

  // static const int nNERch = 128;
  // TH1I* m_adcHigh[nNERch];
  // TH1I* m_TOT[nNERch];

  //TCanvas* m_cadchigh[2];
  //TCanvas* m_ctot[2]; 
  TCanvas* m_cadchigh[4];
  TCanvas* m_ctot[4]; 
  TCanvas *m_scaler;
  //TGraphErrors *gr;

  //web base monitoring
  std::string m_serverName;
  THttpServer* m_server;
  unsigned int m_update_cycle;

};


extern "C"
{
  void MonitorInit(RTC::Manager* manager);
};

#endif // DRS4QDCMONITOR_H
