// -*- C++ -*-
/*!
 * @file NIMEASIROCReader2.cpp
 * @brief  This code is for NIM-EASICROC with Chikuma-san's firmware.
 *         Ishijima-san's firmware is not compatible with this code.
 * @date   Jan. 20th, 2017
 * @author Hidemitsu Asano
 *
 */

#include <cstdlib>
#include <string>


#include "SiTcpRbcp.hh"
#include "NIMEASIROCReader2.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

// Module specification
// Change following items to suit your component's spec.
static const char* nimeasiroceader_spec[] =
{
    "implementation_id", "NIMEASIROCReader2",
    "type_name",         "NIMEASIROCReader2",
    "description",       "NIMEASIROCReader2 component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};



NIMEASIROCReader2::NIMEASIROCReader2(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_OutPort("out1", m_out_data),
      m_sock(NULL),
      m_header(),
      m_data(),
      m_recv_byte_size(0),
      m_out_status(BUF_SUCCESS),
      m_rbcp(NULL),
      m_recvtimeout(10),
      m_isSendADC(true),
      m_isSendTDC(true),
      m_isSendScaler(false),//DO NOT turn on
      m_debug(false)
{
    // Registration: InPort/OutPort/Service

    // Set OutPort buffers
    registerOutPort("out1", m_OutPort);

    init_command_port();
    init_state_table();
    set_comp_name("NIMEASIROCReader2");
}

NIMEASIROCReader2::~NIMEASIROCReader2()
{
}

RTC::ReturnCode_t NIMEASIROCReader2::onInitialize()
{
    if (m_debug) {
        std::cerr << "NIMEASIROCReader2::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t NIMEASIROCReader2::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int NIMEASIROCReader2::daq_dummy()
{
    return 0;
}

int NIMEASIROCReader2::daq_configure()
{
    std::cerr << "*** NIMEASIROCReader2::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);
    
    std::string rubydir = "/home/daq1/work/ELPH_201710/daq-mw-app/daq/component/NIMEASIROCReader2/ruby/Controller.rb";
    std::string execmd  = rubydir + " " + m_srcAddr; 
    std::cout << std::endl;
    std::cout << "Initializing NIM-EASIROC......... "  << std::endl;
    std::cout << std::endl;
    
    int sysout = std::system(execmd.c_str());
    
    std::cout << std::endl;
    std::cout << __FILE__  << " L." << __LINE__ << " system command executed. " << sysout << std::endl;
    std::cout << execmd.c_str() << std::endl;
    std::cout << std::endl;
    if(sysout!=0){
      std::cerr << "***********************************" << std::endl;
      std::cerr << "NIM-EASIROC Initialization FAIL.... " << std::endl;
      std::cerr << "***********************************" << std::endl;
      return -1;
    }else{
      std::cout << "************************" << std::endl;
      std::cout << "NIM-EASIROC Initialized " << std::endl;
      std::cout << "************************" << std::endl;
    }

    //register configuration via SiTCP RBCP
    if (!m_rbcp) {
      std::cout << std::endl;
      std::cout << "register Remote Bus Control Porotocol (RBCP) : port " << SiTcpRbcp::kDefaultPort << std::endl;
      m_rbcp = new SiTcpRbcp(m_srcAddr, SiTcpRbcp::kDefaultPort);
    }
    
    try {
        // Create socket and connect to data server.
        m_sock = new DAQMW::Sock();
        m_sock->connect(m_srcAddr, m_srcPort);
    } catch (DAQMW::SockException& e) {
        std::cerr << "Sock Fatal Error : " << e.what() << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } catch (...) {
        std::cerr << "Sock Fatal Error : Unknown" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }
    //m_sock->setOptRecvTimeOut(3);
    /*
    try {
        // Create socket and connect to data server.
        m_sock = new DAQMW::Sock();
        m_sock->connect(m_srcAddr, m_srcPort);
    } catch (DAQMW::SockException& e) {
        std::cerr << "Sock Fatal Error : " << e.what() << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } catch (...) {
        std::cerr << "Sock Fatal Error : Unknown" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }*/
    //m_sock->setOptRecvTimeOut(1);
    // Check data port connections
    bool outport_conn = check_dataPort_connections( m_OutPort );
    if (!outport_conn) {
        std::cerr << "### NO Connection" << std::endl;
        fatal_error_report(DATAPATH_DISCONNECTED);
    }
     
    //read And Throw PreviousData 
    //clear FIFO before entering DAQ mode
    size_t thrownSize=0;
    int status = 0;
    unsigned char rs[1]={0};
    while(status != DAQMW::Sock::ERROR_TIMEOUT){
      status = m_sock->read(rs,1);
      thrownSize++;
    }
    std::cout << "ThrowPreviousDataSize: " << thrownSize << std::endl;

    //Go to DAQ mode from monitor mode
    DaqMode();
    std::cout << "set recv. time out " << m_recvtimeout << std::endl;

    m_sock->setOptRecvTimeOut(m_recvtimeout);
    m_out_status = BUF_SUCCESS;
    
    return 0;
}

int NIMEASIROCReader2::parse_params(::NVList* list)
{
    bool srcAddrSpecified = false;
    bool srcPortSpecified = false;

    std::cerr << "param list length:" << (*list).length() << std::endl;

    int len = (*list).length();
    for (int i = 0; i < len; i+=2) {
        std::string sname  = (std::string)(*list)[i].value;
        std::string svalue = (std::string)(*list)[i+1].value;

        std::cerr << "sname: " << sname << "  ";
        std::cerr << "value: " << svalue << std::endl;

        if ( sname == "srcAddr1" ) {
            srcAddrSpecified = true;
            if (m_debug) {
                std::cerr << "source addr: " << svalue << std::endl;
            }
            m_srcAddr = svalue;
        }
        if ( sname == "srcPort1" ) {
            srcPortSpecified = true;
            if (m_debug) {
                std::cerr << "source port: " << svalue << std::endl;
            }
            char* offset;
            m_srcPort = (int)strtol(svalue.c_str(), &offset, 10);
        }
    }

    if (!srcAddrSpecified) {
        std::cerr << "### ERROR:data source address not specified\n";
        fatal_error_report(USER_DEFINED_ERROR1, "NO SRC ADDRESS");
    }
    if (!srcPortSpecified) {
        std::cerr << "### ERROR:data source port not specified\n";
        fatal_error_report(USER_DEFINED_ERROR2, "NO SRC PORT");
    }
    
    return 0;
}

int NIMEASIROCReader2::daq_unconfigure()
{
    std::cerr << "*** NIMEASIROCReader2::unconfigure" << std::endl;
    //exit DAQ mode
    //MonitorMode();
    if(m_rbcp){
      delete m_rbcp;
      m_rbcp = NULL;
    }
    return 0;
}

int NIMEASIROCReader2::daq_start()
{

    //std::cerr << "*** NIMEASIROCReader2::start" << std::endl;

    return 0;
}

int NIMEASIROCReader2::daq_stop()
{
    std::cerr << "*** NIMEASIROCReader2::stop" << std::endl;
    
    //Go to monitor mode from DAQ mode
    std::cerr << __FILE__ << " L." << __LINE__ << "  exiting DAQ mode..."  << std::endl;
    
    //disable daq mode
    MonitorMode();

    if (m_sock) {
        m_sock->disconnect();
        delete m_sock;
        m_sock = NULL;
    }
    
    memset(m_header,0,sizeof(m_header));
    m_data.clear();
    
    // Finalize EASIROC
    // TODO : implement some function ?
    //finalize_device(); 
    std::cout << "total sequence # " << get_sequence_num() << std::endl;

    return 0;
}

int NIMEASIROCReader2::daq_pause()
{
    std::cerr << "*** NIMEASIROCReader2::pause" << std::endl;

    return 0;
}

int NIMEASIROCReader2::daq_resume()
{
    std::cerr << "*** NIMEASIROCReader2::resume" << std::endl;

    return 0;
}

int NIMEASIROCReader2::read_data_from_detectors()
{
    int received_data_size = 0;
     
    //How to read NIM-EASIROC data
    //1. receive Header (always 4 bytes)
    //2. get data size from header (changes event by event)
    //3. receive Data
    
    memset(m_header,0,sizeof(m_header));
    
    int status = m_sock->readAll(m_header,NIMEASIROC::headersize);

    if (status == DAQMW::Sock::ERROR_FATAL) {
        std::cerr << "### ERROR: m_sock->readAll() header" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }
    else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
        std::cerr << "### Timeout: m_sock->readAll() header" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }
    else {
        received_data_size = NIMEASIROC::headersize;
    }

    
    if(m_debug){
      std::cout << __FILE__ << " L. " << __LINE__ << " size of header " << sizeof(m_header) << std::endl;
    }
    //check header format and get data size
    unsigned int header32 = unpackBigEndian32(m_header);
    
    
    //unsigned int frame = header32 & 0x80808080;
    //bool isHeader = ((header32 >> 27) & 0x01) == 0x01;
    ////if(!isHeader){
    //if(frame != NIMEASIROC::normalframe){
    //  std::cerr << __FILE__ << " L." << __LINE__ << " Frame Error! " << std::endl;
    //  std::cerr << "header32 " << std::hex << header32 << std::dec << std::endl;
    //  return 0;
    //}
    
    /*
    //decoding header word
    unsigned int ret = ((header32 & 0x7f000000) >> 3) | 
                       ((header32 & 0x007f0000) >> 2) |
                       ((header32 & 0x00007f00) >> 1) |
                       ((header32 & 0x0000007f) >> 0);
    */

    //get the number of words
    unsigned int ret = Decode32bitWord(header32);
    size_t NWordData = ret & 0x0fff; 
    if(m_debug){
      std::cout << __FILE__ << " L." << __LINE__ << " data size " << NWordData << std::endl;
    }
    unsigned int dataSize = NWordData * sizeof(int);// bytes
    //if(!m_data.empty(){
    //  std::cerr << __FILE__ << " L." << __LINE__ << "m_data is not empty !!" << std::endl;
    //}
    if(m_debug){
      std::cout << __FILE__ << " L." << __LINE__ << " body data size " << dataSize << std::endl;
    }
    m_data.clear();
    m_data.resize(NIMEASIROC::headersize + dataSize);
    //std::copy(m_header.begin(),m_header.end(),back_inserter(m_data));
    for(int idata = 0; idata < NIMEASIROC::headersize; idata++){
      m_data.at(idata) = m_header[idata];
    }

    status = m_sock->readAll(&(m_data[NIMEASIROC::headersize]), dataSize);
    if (status == DAQMW::Sock::ERROR_FATAL) {
        std::cerr << "### ERROR: m_sock->readAll() body" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }
    else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
        std::cerr << "### Timeout: m_sock->readAll() body" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }
    else {
        //total data size = header size + data body size
        received_data_size += dataSize;
    }
    //TODO implement later
    /*
    if(!datacheck(m_data)) return 0; 
   */

    return received_data_size;
}

//set data going to Dispacher
int NIMEASIROCReader2::set_data(unsigned int data_byte_size)
{
    unsigned char header[8];
    unsigned char footer[8];

    set_header(&header[0], data_byte_size);
    set_footer(&footer[0]);

    ///set OutPort buffer length
    m_out_data.data.length(data_byte_size + HEADER_BYTE_SIZE + FOOTER_BYTE_SIZE);
    memcpy(&(m_out_data.data[0]), &header[0], HEADER_BYTE_SIZE);
    memcpy(&(m_out_data.data[HEADER_BYTE_SIZE]), &m_data[0], data_byte_size);
    memcpy(&(m_out_data.data[HEADER_BYTE_SIZE + data_byte_size]), &footer[0],
           FOOTER_BYTE_SIZE);

    return 0;
}

int NIMEASIROCReader2::write_OutPort()
{
    ////////////////// send data from OutPort  //////////////////
    //memo by H.Asano
    //The monitor module receives data from this OutPort.
    //The name of this OutPort defined in config.xml  must be same as the name of inPort for monitorComp
    bool ret = m_OutPort.write();

    //////////////////// check write status /////////////////////
    if (ret == false) {  // TIMEOUT or FATAL
        m_out_status  = check_outPort_status(m_OutPort);
        if (m_out_status == BUF_FATAL) {   // Fatal error
            fatal_error_report(OUTPORT_ERROR);
        }
        if (m_out_status == BUF_TIMEOUT) { // Timeout
            return -1;
        }
    }
    else {
        m_out_status = BUF_SUCCESS; // successfully done
    }

    return 0;
}

int NIMEASIROCReader2::daq_run()
{
    if (m_debug) {
        std::cerr << "*** NIMEASIROCReader2::run" << std::endl;
    }

    if (check_trans_lock()) {  // check if stop command has come
        set_trans_unlock();    // transit to CONFIGURED state
        return 0;
    }

    if (m_out_status == BUF_SUCCESS) {   // previous OutPort.write() successfully done
        int ret = read_data_from_detectors();
        if (ret > 0) {
            m_recv_byte_size = ret;
            set_data(m_recv_byte_size); // set data to OutPort Buffer
            if(m_debug){
              std::cout << __FILE__ << " L." << __LINE__ << " sent data size to OutPort " << m_recv_byte_size << std::endl;
            }
        }
    }

    if (write_OutPort() < 0) {
        ;     // Timeout. do nothing.
    }
    else {    // OutPort write successfully done
        inc_sequence_num();                     // increase sequence num.
        inc_total_data_size(m_recv_byte_size);  // increase total data byte size
        m_data.clear();
    }

    return 0;
}

void NIMEASIROCReader2::MonitorMode()
{
  std::cout << " l." << __LINE__ << "Enter Monitoring Mode .. " << std::endl;
  unsigned char data =0;
  if(m_isSendADC){
    //enable ADC info. 
    data |= NIMEASIROC::sendAdcBit;
  }
  if(m_isSendTDC){
    //enable TDC info.
    data |= NIMEASIROC::sendTdcBit;
  }
  if(m_isSendScaler){
    //enable scaler info.
    //should be disable 
    data |= NIMEASIROC::sendScalerBit;
  }
   
  std::cout << __FILE__ << " L." << __LINE__ << " write status register " <<  std::endl;
  std::cout << "Address: " << std::hex <<  NIMEASIROC::statusRegisterAddress << std::dec << "val. " <<  (int)data << std::endl;
  
  int datalength = 1;//byte
  m_rbcp->write(NIMEASIROC::statusRegisterAddress,&data,datalength);

  return;
}

void NIMEASIROCReader2::DaqMode()
{
  std::cerr << " l." << __LINE__ << " Enter DAQ mode"  << std::endl;
  unsigned char data =0;
  //enable DAQ mode
  data |= NIMEASIROC::daqModeBit;
  if(m_isSendADC){
    //enable ADC info. 
    data |= NIMEASIROC::sendAdcBit;
  }
  if(m_isSendTDC){
    //enable TDC info.
    data |= NIMEASIROC::sendTdcBit;
  }
  if(m_isSendScaler){
    //enable scaler info.
    //should be disable 
    data |= NIMEASIROC::sendScalerBit;
  }

  //if(m_debug){
    std::cout << __FILE__ << " l." << __LINE__ << " write status register " <<  std::endl;
    std::cout << "Address: " << std::hex << NIMEASIROC::statusRegisterAddress  << std::dec << " val.: " <<  (int)data << std::endl;
  //}
  int datalength = 1;//byte
  m_rbcp->write(NIMEASIROC::statusRegisterAddress,&data,datalength);
  
 // unsigned char triggermode = 0; 

  return;
}

//convert 4 char arrays to a single 32 bit word
unsigned int NIMEASIROCReader2::unpackBigEndian32(const unsigned char* array4byte)
{

    return ((array4byte[0] << 24) & 0xff000000) |
           ((array4byte[1] << 16) & 0x00ff0000) |
           ((array4byte[2] <<  8) & 0x0000ff00) |
           ((array4byte[3] <<  0) & 0x000000ff);
}



unsigned int NIMEASIROCReader2::Decode32bitWord(unsigned int word32bit)
{
  //check data format
  unsigned int frame = word32bit & 0x80808080;
  if(frame != NIMEASIROC::normalframe){
    std::cerr << __FILE__ << " L." << __LINE__ << " Frame Error! " << std::endl;
    std::cerr << "32 bit word: " << std::hex << word32bit << std::dec << std::endl;
    return 0;
  }

  return ((word32bit & 0x7f000000) >> 3) | 
         ((word32bit & 0x007f0000) >> 2) |
         ((word32bit & 0x00007f00) >> 1) |
         ((word32bit & 0x0000007f) >> 0);
}

//under construction. DO NOT USE
bool NIMEASIROCReader2::datacheck(std::vector <unsigned char> data)
{
  size_t NWordData = data.size()/sizeof(int);
  unsigned char dataoneblock[4];
  bool isAllOK=true;
  for(unsigned int i=0;i<NWordData;i++){
    dataoneblock[0] = data.at(i/4   );
    dataoneblock[1] = data.at(i/4 + 1);
    dataoneblock[2] = data.at(i/4 + 2);
    dataoneblock[3] = data.at(i/4 + 3);
    unsigned int data32 = unpackBigEndian32(dataoneblock);
    unsigned int ret = Decode32bitWord(data32);
    bool isOKword = (ret >> 27) ;
    isAllOK |= isOKword;
  }
  return isAllOK;
}



extern "C"
{
    void NIMEASIROCReader2Init(RTC::Manager* manager)
    {
        RTC::Properties profile(nimeasiroceader_spec);
        manager->registerFactory(profile,
                    RTC::Create<NIMEASIROCReader2>,
                    RTC::Delete<NIMEASIROCReader2>);
    }
};
