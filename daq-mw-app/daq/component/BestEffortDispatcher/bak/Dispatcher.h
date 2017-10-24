// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "DaqComponentBase.h"

/*
////////// ROOT Include files //////////
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TApplication.h"
*/
using namespace RTC;

class Dispatcher
    : public DAQMW::DaqComponentBase
{
public:
    Dispatcher(RTC::Manager* manager);
    ~Dispatcher();

    // The initialize action (on CREATED->ALIVE transition)
    // former rtc_init_entry()
    virtual RTC::ReturnCode_t onInitialize();

    // The execution action that is invoked periodically
    // former rtc_active_do()
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
    TimedOctetSeq          m_in0_data;
    InPort<TimedOctetSeq> m_InPort0;
    
    TimedOctetSeq          m_in1_data;
    InPort<TimedOctetSeq> m_InPort1;
    
    TimedOctetSeq          m_in_data_sum;

    TimedOctetSeq          m_out0_data;
    OutPort<TimedOctetSeq> m_OutPort0;

    TimedOctetSeq          m_out1_data;
    OutPort<TimedOctetSeq> m_OutPort1;

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
    int set_data_OutPort0(unsigned int data_byte_size);
    int set_data_OutPort1(unsigned int data_byte_size);
    int reset_InPort0();
    int reset_InPort1();
    unsigned int read_InPort0();
    unsigned int read_InPort1();
    int write_OutPort0();
    int write_OutPort1();

    static const unsigned int SEND_BUFFER_SIZE = 4096;
    unsigned char m_data_sum[2*SEND_BUFFER_SIZE];
    //unsigned char m_data_0[SEND_BUFFER_SIZE];
    //unsigned char m_data_1[SEND_BUFFER_SIZE];
    //std::vector <unsigned char> m_data_sum;
    //std::vector <unsigned char> m_data_0;
    //std::vector <unsigned char> m_data_1;

    BufferStatus m_in0_status;
    BufferStatus m_in1_status;
    BufferStatus m_out0_status;
    BufferStatus m_out1_status;


    //TCanvas *m_canvas;
    //TH1I    *m_hist;


    unsigned int m_in0_timeout_counter;  //timeout counter for InPort0 reading
    unsigned int m_in1_timeout_counter;  //timeout counter for InPort1 reading
    unsigned int m_out0_timeout_counter;//timeout counter for OutPort0 writing
    unsigned int m_out1_timeout_counter;//timeout counter for OutPort1 writing

    unsigned int m_inport_recv_data_size_sum;
    unsigned int m_inport0_recv_data_size;
    unsigned int m_inport1_recv_data_size;
    
    bool m_debug;
};


extern "C"
{
    void DispatcherInit(RTC::Manager* manager);
};

#endif // DISPATCHER_H
