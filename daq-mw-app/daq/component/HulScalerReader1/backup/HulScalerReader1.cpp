// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include "HulScalerReader1.h"
#include "standalone/src/UDPRBCP.hh"
#include <iostream>
#include <iomanip> 

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

// Module specification
// Change following items to suit your component's spec.
static const char* hulscalerreader_spec[] =
{
    "implementation_id", "HulScalerReader1",
    "type_name",         "HulScalerReader1",
    "description",       "HulScalerReader1 component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

HulScalerReader1::HulScalerReader1(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_OutPort("out4", m_out_data),
      fModule(NULL),
      m_sock(NULL),
      m_recv_byte_size(0),
      m_data(),
      m_out_status(BUF_SUCCESS),
      m_debug(true)
{
    // Registration: InPort/OutPort/Service

    // Set OutPort buffers
    registerOutPort("out4", m_OutPort);

    init_command_port();
    init_state_table();
    set_comp_name("HulScalerReader1");
}

HulScalerReader1::~HulScalerReader1()
{
}

RTC::ReturnCode_t HulScalerReader1::onInitialize()
{
    if (m_debug) {
        std::cerr << "HulScalerReader1::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t HulScalerReader1::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int HulScalerReader1::daq_dummy()
{
    return 0;
}

int HulScalerReader1::daq_configure()
{
    std::cerr << "*** HulScalerReader1::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);
   
    if(m_sock == NULL){
      try {
        // Create socket and connect to data server.
        // Sock() is defined (DAQ-Middleware-HOME)/src/lib/SiTCP/CPP/Sock/Sock.h
        m_sock = new DAQMW::Sock();
        std::cout << "conecting socket ... " << std::endl;
        m_sock->connect(m_srcAddr, m_srcPort);
      } catch (DAQMW::SockException& e) {
        std::cerr << "Sock Fatal Error : " << e.what() << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
      } catch (...) {
        std::cerr << "Sock Fatal Error : Unknown" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
      }
    }

    // set parameters of socket to be same as ConnectSocket() in daq_func.cc
    // written by Honda.
    // set/reset receive/read timeout
    // The method sets or resets receive/read timeout. 
    // If the specified value is 0, it resets the timeout. 
    // Otherwise, it sets the timeout value in seconds.
    // if SUCCESS returns, success. Oterwise, fatal error will be thrown.
    //m_sock->setOptRecvTimeOut(3);
    m_sock->setOptRecvTimeOut(10);

    //set/reset Nagle algorithm
    //* The method sets or resets Nagle algorithm. If bool is true, it sets.
    //* if SUCCESS returns, success. Oterwise, fatal error will be thrown.
    m_sock->setOptNoDelay(false);
    //finish parameter setting for socket, those must be set after connecting socket
    
    
    rbcp_header rbcpHeader;
    rbcpHeader.type = UDPRBCP::rbcp_ver_;
    rbcpHeader.id   = 0;
    m_board_ip = new char[m_srcAddr.length() + 1];
    strcpy(m_board_ip, m_srcAddr.c_str());
    std::cout << "Board IP " ;
    for(unsigned int ic = 0 ; ic<strlen(m_board_ip) ; ic++){
      std::cout << m_board_ip[ic] ; 
    }
    std::cout << std::endl;
    
    //FPGAModule is called from the library in ($PWD)/standalone/src/libFPGAModule.so
    //board_ip is char* type , udp_port, &rbcpHeader, interactive mode=0  
    if(fModule) delete fModule;
    FPGAModule *fModule = new FPGAModule(m_board_ip, udp_port, &rbcpHeader, 0);
    
    int ret=0;
    //module ID, Local Address, register
    ret = fModule->WriteModule(DCT::mid, DCT::laddr_evb_reset, 1);//reset Event Builder
    std::cerr << "L." << __LINE__ << "Write couter reset  " << ret << std::endl;
    //enable input port
    //LSB     : Fixed Sigin U <- DRS4 trigger out
    //2nd bit : Fixed Sigin D <- NIM-ECL converter
    //3rd bit : Mezzanine U   <- not used
    //4th bit : Mezzanine D   <- not used
    ret = fModule->WriteModule(SCR::mid, SCR::laddr_enable_block, 0x3);//set block enable
    std::cerr << "L." << __LINE__ << " set block enable 0x3 :" << ret << std::endl;
    std::cerr << "Read back: " << std::hex << fModule->ReadModule(SCR::mid, SCR::laddr_enable_block,1) << std::endl;
    
    //enable hardware coutern reset by NIM input
    //LSB     : Fixed Sigin U <- not used
    //2nd bit : Fixed Sigin D <- not used
    //3rd bit : Mezzanine U   <- not used
    //4th bit : Mezzanine D   <- not used
    ret = fModule->WriteModule(SCR::mid, SCR::laddr_enable_hdrst, 0x0);//set hard reset
    std::cerr << "L." << __LINE__ << " set harware reset bit 0x0: " << ret << std::endl;
    std::cerr << "Read back : " << std::hex << fModule->ReadModule(SCR::mid, SCR::laddr_enable_hdrst,1) << std::endl;
    //SETTING Trigger Manager (TRM)
    
    
    //not to use TRM
    //unsigned int sel_trig = 0;
    

    fModule->WriteModule(IOM::mid, IOM::laddr_extL1, IOM::reg_i_nimin1);
    ////do not use extL2
    fModule->WriteModule(IOM::mid, IOM::laddr_extL2, IOM::reg_i_nimin1);
    std::cerr << "L." << __LINE__ << " set L2 trigger bit " << IOM::reg_i_nimin1 << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(TRM::mid, IOM::laddr_extL2,1) << std::endl;
    //do not use extClr
    fModule->WriteModule(IOM::mid, IOM::laddr_extCCRst, IOM::reg_i_nimin3);
    std::cerr << "L." << __LINE__ << " set ext CCRst bit " << IOM::reg_i_nimin3 << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(IOM::mid, IOM::laddr_extCCRst,1) << std::endl;
    
    fModule->WriteModule(IOM::mid, IOM::laddr_extSpillGate, IOM::reg_i_nimin2);
    std::cerr << "L." << __LINE__ << " set extSpillGate bit " << IOM::reg_i_nimin2  << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(IOM::mid, IOM::laddr_extSpillGate,1) << std::endl;
    

    // Check data port connections
    //std::cerr << "check_dataPort_connections( m_OutPort ); is tempolarily disabled" << std::endl;
    bool outport_conn = check_dataPort_connections( m_OutPort );
    if (!outport_conn) {
      std::cerr << "### NO Connection" << std::endl;
      fatal_error_report(DATAPATH_DISCONNECTED);
    }
    
    //read And Throw unused Data 
    //clear FIFO before daq starting
    size_t thrownSize=0;
    int status = 0;
    unsigned char rs[1]={0};
    //while(status != DAQMW::Sock::ERROR_TIMEOUT){
    while( (m_sock->read(rs,1)) != DAQMW::Sock::ERROR_TIMEOUT){
      status = m_sock->read(rs,1);
      thrownSize++;
    }
    std::cout << "ThrownDataSize: " << thrownSize << std::endl;
    
    /*
    rbcp_header rbcpHeader;
    rbcpHeader.type = UDPRBCP::rbcp_ver_;
    rbcpHeader.id   = 0;
    if(fModule) delete fModule;
    FPGAModule *fModule = new FPGAModule(m_board_ip, udp_port, &rbcpHeader, 0);
    */
    m_out_status = BUF_SUCCESS;
    //using NIMIN as L1 trigger
    unsigned int sel_trig = TRM::reg_L1Ext;
    fModule->WriteModule(TRM::mid, TRM::laddr_sel_trig,  sel_trig);
    std::cerr << "L." << __LINE__ << " set L1 trigger bit " << sel_trig << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(TRM::mid, TRM::laddr_sel_trig,1) << std::endl;
    
    ret = fModule->WriteModule(SCR::mid, SCR::laddr_counter_reset,  1);
    std::cerr << "L." << __LINE__ << " set counter reset " << "  "  << ret << std::endl;
    //enable DAQ gate
    ret = fModule->WriteModule(DCT::mid, DCT::laddr_gate,  1);
    std::cerr << "L." << __LINE__ << " set DAQ gate 1 " << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(DCT::mid, DCT::laddr_gate,1) << std::endl;
    std::cout << "enable DAQ gate" << std::endl;


    std::cout << "************************" << std::endl;
    std::cout << "HUL-Scaler Initialized  " << std::endl;
    std::cout << "************************" << std::endl;

    

    return 0;
}

int HulScalerReader1::parse_params(::NVList* list)
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

        if ( sname == "srcAddr4" ) {
            srcAddrSpecified = true;
            if (m_debug) {
                std::cerr << "source addr: " << svalue << std::endl;
            }
            m_srcAddr = svalue;
        }
        if ( sname == "srcPort4" ) {
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

int HulScalerReader1::daq_unconfigure()
{
    std::cerr << "*** HulScalerReader1::unconfigure" << std::endl;
    if(fModule) delete fModule;
    return 0;
}

int HulScalerReader1::daq_start()
{   
    /*
    rbcp_header rbcpHeader;
    rbcpHeader.type = UDPRBCP::rbcp_ver_;
    rbcpHeader.id   = 0;
    if(fModule) delete fModule;
    FPGAModule *fModule = new FPGAModule(m_board_ip, udp_port, &rbcpHeader, 0);
    m_out_status = BUF_SUCCESS;
    int ret = fModule->WriteModule(SCR::mid, SCR::laddr_counter_reset,  1);
    std::cerr << "L." << __LINE__ << " set counter reset " << "  "  << ret << std::endl;
    //enable DAQ gate
    ret = fModule->WriteModule(DCT::mid, DCT::laddr_gate,  1);
    std::cerr << "L." << __LINE__ << " set DAQ gate 1 " << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(DCT::mid, DCT::laddr_gate,1) << std::endl;
    std::cout << "enable DAQ gate" << std::endl;
    */
    
    //std::cerr << "*** HulScalerReader1::start" << std::endl;

     
    
    return 0;
}

int HulScalerReader1::daq_stop()
{
    std::cerr << "*** HulScalerReader1::stop" << std::endl;
    //disable DAQ gate
    rbcp_header rbcpHeader;
    rbcpHeader.type = UDPRBCP::rbcp_ver_;
    rbcpHeader.id   = 0;
    m_board_ip = new char[m_srcAddr.length() + 1];
    strcpy(m_board_ip, m_srcAddr.c_str());
    std::cout << "Board IP " ;
    for(unsigned int ic = 0 ; ic<strlen(m_board_ip) ; ic++){
      std::cout << m_board_ip[ic] ; 
    }
    std::cout << std::endl;
    
    //FPGAModule is called from the library in ($PWD)/standalone/src/libFPGAModule.so
    //board_ip is char* type , udp_port, &rbcpHeader, interactive mode=0  
    if(fModule) delete fModule;
    FPGAModule *fModule = new FPGAModule(m_board_ip, udp_port, &rbcpHeader, 0);
    int ret = fModule->WriteModule(DCT::mid, DCT::laddr_gate,  0);
    std::cerr << "L." << __LINE__ << " set DAQ gate 0 " << "  "  << ret << std::endl;
    std::cerr << "Read back " << fModule->ReadModule(DCT::mid, DCT::laddr_gate,1) << std::endl;
    std::cout << "close DAQ gate" << std::endl;

    
    std::cout << "clearing buffer " << std::endl;
    //read And Throw unused Data 
    //clear FIFO after daq stop
    size_t thrownSize=0;
    //int status = 0;
    unsigned char rs[1]={0};
    while( (m_sock->read(rs,1)) != DAQMW::Sock::ERROR_TIMEOUT){
      //status = m_sock->read(rs,1);
      thrownSize++;
    }
    std::cerr << std::dec << "ThrownDataSize: " << thrownSize << std::endl;
    
    if (m_sock) {
        m_sock->disconnect();
        delete m_sock;
        m_sock = NULL;
    }
    std::cerr << std::dec << "total sequence # " << get_sequence_num() << std::endl;

    if(fModule){
      delete fModule;
      fModule = NULL;
    }
    delete [] m_board_ip;
    return 0;
}

int HulScalerReader1::daq_pause()
{
    std::cerr << "*** HulScalerReader1::pause" << std::endl;

    return 0;
}

int HulScalerReader1::daq_resume()
{
    std::cerr << "*** HulScalerReader1::resume" << std::endl;

    return 0;
}

int HulScalerReader1::read_data_from_detectors()
{
    if(m_debug) std::cerr << "read data " <<  std::endl;
    int received_data_size = 0;
    
    //int data[NofData];
    //read header at first, there will be a size of data body.
    static const unsigned int sizeofHeader = NofHead*sizeof(unsigned int);
    int status = m_sock->readAll((unsigned char*)m_data, sizeofHeader);
    //int status = m_sock->readAll((unsigned char*)data, sizeofHeader);
    if (status == DAQMW::Sock::ERROR_FATAL) {
        std::cerr << "### ERROR: m_sock->readAll" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
        std::cerr << "### Timeout: m_sock->readAll" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }else {
        received_data_size += sizeofHeader;
    }
   
    if(m_debug) std::cerr << "header size " << received_data_size << std::endl;
    unsigned int n_word_data = m_data[1] & 0x3ff;
    //unsigned int n_word_data = data[1] & 0x3ff;
    unsigned int sizeData    = n_word_data*sizeof(unsigned int);
    if(n_word_data == 0) return received_data_size;

    //read data body
    status = m_sock->readAll((unsigned char*)(m_data + NofHead),sizeData);
    //status = m_sock->readAll((unsigned char*)(data + NofHead),sizeData);
    if (status == DAQMW::Sock::ERROR_FATAL) {
        std::cerr << "### ERROR: m_sock->readAll" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
        std::cerr << "### Timeout: m_sock->readAll" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }else {
        received_data_size += sizeData;
    }

    if(m_debug){ 
      std::cerr << "data size " << received_data_size << std::endl;

        //std::printf("\033[2J");
        //for(int i = 0; i<n_word_data; ++i){
        //  std::printf("%8x ", m_data[i]);
        //  if((i+1)%8 == 0) std::printf("\n");
       // }// for(i)
      for(unsigned int i = 0; i< (n_word_data+NofHead); ++i){
        std::cout << std::hex << std::setw(8) << m_data[i] << " " ;
        //std::cout << std::hex << std::setw(8) << data[i] << " " ;
        if((i+1)%8 == 0) std::cout << std::endl;
      }
        //std::printf("\n");
    }
    //memcpy(&(m_data[0]),&data[0],received_data_size/sizeof(int));
    return received_data_size;
}

int HulScalerReader1::set_data(unsigned int data_byte_size)
{
    unsigned char header[8];
    unsigned char footer[8];

    set_header(&header[0], data_byte_size);
    set_footer(&footer[0]);

    ///set OutPort buffer length
    m_out_data.data.length(data_byte_size + HEADER_BYTE_SIZE + FOOTER_BYTE_SIZE);
    memcpy(&(m_out_data.data[0]), &header[0], HEADER_BYTE_SIZE);
    memcpy(&(m_out_data.data[HEADER_BYTE_SIZE]),reinterpret_cast<unsigned char*>(&m_data[0]), data_byte_size);
    memcpy(&(m_out_data.data[HEADER_BYTE_SIZE + data_byte_size]), &footer[0],
           FOOTER_BYTE_SIZE);

    return 0;
}


int HulScalerReader1::write_OutPort()
{
    if(m_debug){
      std::cerr << "writting port" << std::endl;
    }
    ////////////////// send data from OutPort  //////////////////
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

int HulScalerReader1::daq_run()
{
    if (m_debug) {
        std::cerr << "*** HulScalerReader1::run" << std::endl;
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
        }
    }else{
       if(m_debug) std::cout << "out port " << m_out_status << std::endl;
    }

    if( (write_OutPort() < 0) ) {
      ;     // Timeout. do nothing.
    }
    else {    // OutPort write successfully done
        inc_sequence_num();                     // increase sequence num.
        inc_total_data_size(m_recv_byte_size);  // increase total data byte size
    }

    return 0;
}


extern "C"
{
    void HulScalerReader1Init(RTC::Manager* manager)
    {
        RTC::Properties profile(hulscalerreader_spec);
        manager->registerFactory(profile,
                    RTC::Create<HulScalerReader1>,
                    RTC::Delete<HulScalerReader1>);
    }
};
