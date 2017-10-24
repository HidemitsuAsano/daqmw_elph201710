// -*- C++ -*-
/*!
 * @file  NIMEASIROCReader1.h
 * @brief 
 * @date Jan. 20th, 2017
 * @author Hidemitsu Asano
 * @email hidemitsu.asano@riken.jp
 */

#ifndef NIMEASIROCREADER_H
#define NIMEASIROCREADER_H

#include "DaqComponentBase.h"

#include <daqmw/Sock.h>

namespace NIMEASIROC{
  const unsigned int directControlAddress = 0x00000000;
  const unsigned int slowControl1Address = 0x00000003;
  const unsigned int readRegister1Address = 0x0000003C;
  const unsigned int slowControl2Address = 0x0000003D;
  const unsigned int readRegister2Address = 0x00000076;
  const unsigned int statusRegisterAddress = 0x00000077;
  const unsigned int selectableLogicAddress = 0x00000078;
  const unsigned int triggerWidthAddress = 0x00000088;
  const unsigned int timeWindowAddress = 0x00000100;
  const unsigned int pedestalSuppressionAddress = 0x00001000;
  const unsigned int hvControlAddress = 0x00010000;
  const unsigned int monitorAdcAddress = 0x00010010;
  const unsigned int readMadcAddress = 0x00010020;
  const unsigned int usrClkOutAddress = 0x00010030;
  const unsigned int triggerValuesAddress = 0x00010100;
  const unsigned int versionAddress = 0xF0000000;

  //RBCP
  const unsigned char daqModeBit = 0x01;
  const unsigned char sendAdcBit = 0x02;
  const unsigned char sendTdcBit = 0x04;
  const unsigned char sendScalerBit = 0x08;
  
  const int headersize = 4;//bytes. fixed value;
  const unsigned int normalframe     = 0x80000000;
  const unsigned int headercheckmask = 0x80808080;
}

class SiTcpRbcp;

using namespace RTC;

class NIMEASIROCReader1
    : public DAQMW::DaqComponentBase
{
public:
    NIMEASIROCReader1(RTC::Manager* manager);
    ~NIMEASIROCReader1();

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
    int read_data_from_detectors();
    int set_data(unsigned int data_byte_size);
    int write_OutPort();
      
    //utility function
    unsigned int unpackBigEndian32(const unsigned char* array4byte);
    unsigned int Decode32bitWord(unsigned int word32bit);
    bool datacheck(std::vector <unsigned char> data);

    //nim easiroc slow control
    void DaqMode();
    void MonitorMode();


    DAQMW::Sock* m_sock;               /// socket for data server
    

    unsigned char m_header[NIMEASIROC::headersize];
    std::vector <unsigned char> m_data;
    unsigned int  m_recv_byte_size;

    BufferStatus m_out_status;

    int m_srcPort;                        /// Port No. of data server
    std::string m_srcAddr;                /// IP addr. of data server

    SiTcpRbcp* m_rbcp;
    unsigned int m_recvtimeout;
    //nim easiroc data 
    size_t m_headersize;
    size_t m_datasize;
    bool m_isSendADC;
    bool m_isSendTDC;
    bool m_isSendScaler;
    
    bool m_debug;
};


extern "C"
{
    void NIMEASIROCReader1Init(RTC::Manager* manager);
};

#endif // NIMEASIROCREADER_H
