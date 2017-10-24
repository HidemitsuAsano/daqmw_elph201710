// -*- C++ -*-
/*!
 * @file HulScalerReader1
 * @brief This code is to read HUL scaler by DAQ-middleware as reader component.
 * This is transplanted from Dr.Honda's code which is made for standalone operation (http://openit.kek.jp/project/HUL)
 * @date Oct. 3rd, 2017
 * @author Hidemitsu Asano
 * @e-mail hidemitsu.asano@riken.jp
 */

#ifndef HulScalerReader1_H
#define HulScalerReader1_H

#include "DaqComponentBase.h"

#include <daqmw/Sock.h>
#include <vector>

#include <iostream>
#include <cstdio>


#include "standalone/src/RegisterMap.hh"
#include "standalone/src/network.hh"
#include "standalone/src/UDPRBCP.hh"
#include "standalone/src/CommandMan.hh"
#include "standalone/src/FPGAModule.hh"
#include "standalone/src/rbcp.h"
//#include "standalone/src/daq_funcs.hh"
#include <errno.h>

class SiTcpRbcp;
class FPGAModule;

using namespace RTC;
using namespace HUL_Scaler;

static const int NofHead = 3;
static const int NofBody = 256;
static const int NofData = NofHead + NofBody;

class HulScalerReader1
    : public DAQMW::DaqComponentBase
{

public:
  typedef std::vector<unsigned char> dType;
  typedef dType::const_iterator      dcItr;

public:
    HulScalerReader1(RTC::Manager* manager);
    ~HulScalerReader1();

    // The initialize action (on CREATED->ALIVE transition)
    // former rtc_init_entry()
    virtual RTC::ReturnCode_t onInitialize();

    // The execution action that is invoked periodically
    // former rtc_active_do()
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);
    
private:
    TimedOctetSeq          m_out_data;
    OutPort<TimedOctetSeq> m_OutPort;
    FPGAModule *fModule;
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
    int set_data(unsigned int data_byte_size);
    int write_OutPort();
    
    //uses NIMOUT1 as I/O manager
    //probably not used
    //int ConnectSocket(const char* ip);
    //int receive(int sock, char* data_buf, unsigned int length);
    //int Event_Cycle(int sock, unsigned int* buffer);

    DAQMW::Sock* m_sock;               /// socket for data server
    int m_lsock;
    unsigned int  m_recv_byte_size;
    //std::vector<unsigned int> m_data;
    unsigned int  m_data[NofData];
    //unsigned int  m_data[];
    BufferStatus m_out_status;

    int m_srcPort;                        /// Port No. of data server
    std::string m_srcAddr;                /// IP addr. of data server
    char*  m_board_ip;
    bool m_debug;
    
};


extern "C"
{
    void HulScalerReader1Init(RTC::Manager* manager);
};

#endif // HULScalerREADER_H
