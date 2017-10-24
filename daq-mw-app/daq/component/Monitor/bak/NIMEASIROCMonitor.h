// -*- C++ -*-
/*!
 * @file  NIMEASIROCMonitor.h
 * @brief
 * @date Jan. 20th, 2017
 * @author Hidemitsu Asano
 *
 */

#ifndef NIMEASIROCMONITOR_H
#define NIMEASIROCMONITOR_H

#include "DaqComponentBase.h"

//#include <arpa/inet.h> // for ntohl()


namespace NIMEASIROC{
  const int headersize = 4;//bytes. fixed value;
  const unsigned int normalframe     = 0x80000000;
  const unsigned int checkmask = 0x80808080;
  //const int nbindata = 4096;
}

////////// ROOT Include files //////////
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TApplication.h"

#include "NIMEASIROCData.h"

using namespace RTC;

class NIMEASIROCMonitor
    : public DAQMW::DaqComponentBase
{
public:
    NIMEASIROCMonitor(RTC::Manager* manager);
    ~NIMEASIROCMonitor();

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
    //int online_analyze();
    unsigned int decode_header(const unsigned char* mydata);
    unsigned int decode_data(const unsigned char* mydata);
    int fill_data(const unsigned char* mydata, const int size);
    void clearhists();
    
    //utility function
    unsigned int unpackBigEndian32(const unsigned char* array4byte);
    bool isAdcHg(unsigned int data);
    bool isAdcLg(unsigned int data);
    bool isTdcLeading(unsigned int data);
    bool isTdcTrailing(unsigned int data);
    bool isScaler(unsigned int data);
    unsigned int Decode32bitWord(unsigned int word32bit);

    BufferStatus m_in_status;

    ////////// ROOT Histogram //////////
    
    //H.Asano 
    //these are from the example code
    TCanvas *m_canvas;
    TH1F    *m_hist;
    int      m_bin;
    double   m_min;
    double   m_max;


    int      m_monitor_update_rate;
    
    //received nimeasiroc's header + body data from ReaderComp.
    std::vector <unsigned char> m_recv_data;
    unsigned int  m_event_byte_size;
    //make 2 data struct for 2 module operation
    struct NIMEASIROCData m_NIMEASIROCData1;
    struct NIMEASIROCData m_NIMEASIROCData2;
    

    //H.Asano
    //implementation for QA of low level info.
    TH1I* m_adcHigh[128];
    TH1I* m_adcLow[128];
    TH1I* m_tdcLeading[128];
    TH1I* m_tdcTrailing[128];
    
    //2D correlation plot for tracking
    TH2I* m_2DcorrelationX[4];
    TH2I* m_2DcorrelationU[4];
    TH2I* m_2DcorrelationV[4];
    //not used so far. (June. 9th, 2017)
    //TH1F* m_scaler[67*2];
    bool m_isSaveFile;  
    bool m_isSaveTree;  
    TFile* m_tfile; 
    TTree* m_tree;
    //data 
    /*
    int m_evtnum;
    int m_dataadcHigh[128];
    int m_dataadcLow[128];
    int m_datatdcLeading[128];
    int m_datatdcTrailing[128];
    */
    bool m_debug;
};


extern "C"
{
    void NIMEASIROCMonitorInit(RTC::Manager* manager);
};

#endif // NIMEASIROCMONITOR_H
