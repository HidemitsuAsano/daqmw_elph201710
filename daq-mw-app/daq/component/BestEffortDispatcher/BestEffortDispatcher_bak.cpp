// -*- C++ -*-
/*!
 * @file BestEffortDispatcher.h
 * @brief This class works as merger, event builder and dispatcher
 * @date
 * @author Hidemitsu Asano
 * @email hidemitsu.asano@riken.jp 
 *
 */

#include "BestEffortDispatcher.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

// Module specification
// Change following items to suit your component's spec.
static const char* besteffortdispatcher_spec[] =
  {
    "implementation_id", "BestEffortDispatcher",
    "type_name",         "BestEffortDispatcher",
    "description",       "Dispatcher component with best-effort OutPort",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };

//______________________________________________________________________________
BestEffortDispatcher::BestEffortDispatcher(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_InPort0("in0", m_in0_data),//NIMEASIROC1
    m_InPort1("in1", m_in1_data),//NIMEASIROC2
    m_InPort2("in2", m_in2_data),//DRS4QDC1
    m_InPort3("in3", m_in3_data),//DRS4QDC2
    m_InPort4("in4", m_in4_data),//HULSCALER
    m_OutPort("out0", m_out0_data),//for DAQlogger, standard type
    m_BestEffort_OutPort("out1", m_out1_data),//for Monitor, best effort type
    m_in0_status(BUF_SUCCESS),
    m_in1_status(BUF_SUCCESS),
    m_in2_status(BUF_SUCCESS),
    m_in3_status(BUF_SUCCESS),
    m_in4_status(BUF_SUCCESS),
    m_out_status(BUF_SUCCESS),
    m_besteffort_out_status(BUF_SUCCESS),
    m_in_tout_counts(),
    m_out0_tout_counts(0),
    m_out1_tout_counts(0),
    m_inport_recv_data_size(),
    m_inport_recv_data_size_sum(0),
    //m_v1718(0),
    //m_Handle(0),
    m_Verbosity(1)
{
  // Registration: InPort/OutPort/Service

  // Set OutPort buffers
  registerInPort("in0", m_InPort0);
  registerInPort("in1", m_InPort1);
  registerInPort("in2", m_InPort2);
  registerInPort("in3", m_InPort3);
  registerInPort("in4", m_InPort4);
  
  registerOutPort("out0", m_OutPort);//To DAQLogger 
  registerOutPort("out1", m_BestEffort_OutPort);//To Monitor

  init_command_port();
  init_state_table();
  set_comp_name("BestEffortDispatcher");
}

//______________________________________________________________________________
BestEffortDispatcher::~BestEffortDispatcher()
{
}

//______________________________________________________________________________
RTC::ReturnCode_t BestEffortDispatcher::onInitialize()
{
  if (m_Verbosity) {
    std::cerr << "BestEffortDispatcher::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t BestEffortDispatcher::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_dummy()
{
  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_configure()
{
  std::cerr << "*** BestEffortDispatcher::configure" << std::endl;
  
  std::cerr << "reset Inports" << std::endl;
  reset_InPort0();
  reset_InPort1();
  reset_InPort2();
  reset_InPort3();
  reset_InPort4();
  
  // Create VME anad //
  //base address of v1718 is hard-coded here. if you change base address via
  //rotary switch of v1718, modify it.
  //BdType , Link, base address (0x00), pointer for handling 
  //if(!m_v1718){
  //  m_v1718 = new CAENv1718(cvV1718,0,0,&m_Handle);
  //  m_v1718->Initialize();
  //}
  //m_v1718->SystemReset();

  //generate latch from output line 1 (AS)
  //m_v1718->SetBusy(1);
  //m_v1718->OnePulse();
  //m_v1718->Sto
  
  //std::cerr << "CAEN v1718 is initialized" << std::endl;
  
  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::parse_params(::NVList* list)
{
  std::cerr << "param list length:" << (*list).length() << std::endl;

  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;

    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;
  }

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_unconfigure()
{
  std::cerr << "*** BestEffortDispatcher::unconfigure" << std::endl;
  
  //m_v1718->OnePulse();
  //m_v1718->SystemReset();
  //delete m_v1718;
  //m_v1718 = 0;

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_start()
{
  std::cerr << "*** BestEffortDispatcher::start" << std::endl;
  m_in0_status   = BUF_SUCCESS;
  m_in1_status   = BUF_SUCCESS;
  m_in2_status   = BUF_SUCCESS;
  m_in3_status   = BUF_SUCCESS;
  m_in4_status   = BUF_SUCCESS;

  m_out_status = BUF_SUCCESS;
  m_besteffort_out_status = BUF_SUCCESS;
  //clear latch veto from output 1
  //m_v1718->SetBusy(0);
  //clear trigger veto
  //m_v1718->OnePulse();
  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_stop()
{
  //generate latch veto from output 1
  //m_v1718->SetBusy(1);

  std::cerr << "*** BestEffortDispatcher::stop" << std::endl;
  //reset_InPort0();
  //reset_InPort1();
  //reset_InPort2();
  //reset_InPort3();
  //reset_InPort4();
  //std::cerr << "*** reset InPorts.. " << std::endl;
  std::cout << "Dispather::total sequence # " << get_sequence_num() << std::endl;


  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_pause()
{
  std::cerr << "*** BestEffortDispatcher::pause" << std::endl;

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_resume()
{
  std::cerr << "*** BestEffortDispatcher::resume" << std::endl;

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::read_data_from_detectors()
{
  int received_data_size = 0;
  /// write your logic here
  return received_data_size;
}

//______________________________________________________________________________
int BestEffortDispatcher::set_data_OutPort(unsigned int data_byte_size)
{
  ///set OutPort buffer length
  m_out0_data.data.length(data_byte_size);
  memcpy(&(m_out0_data.data[0]), &m_in_data_sum.data[0], data_byte_size);
  return 0;
}


//______________________________________________________________________________
int BestEffortDispatcher::set_data_BestEffort_OutPort(unsigned int data_byte_size)
{

  // modify footer
  // unsigned char footer[8];
  // set_footer(&footer[8]);

  m_out1_data.data.length(data_byte_size);
  // memcpy(&(m_out1_data.data[0]), &(m_in_data.data[0]), data_byte_size-FOOTER_BYTE_SIZE);
  // memcpy(&(m_out1_data.data[data_byte_size-FOOTER_BYTE_SIZE]), &footer[0], FOOTER_BYTE_SIZE);
  memcpy(&(m_out1_data.data[0]), &(m_in_data_sum.data[0]), data_byte_size);

  return 0;
}

//______________________________________________________________________________

//READ data from NIMEASRIOC1
unsigned int BestEffortDispatcher::read_InPort0()
{
  if (check_trans_lock()) set_trans_unlock();
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort0.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in0_status = check_inPort_status(m_InPort0);
    if (m_in0_status == BUF_TIMEOUT) { // Buffer empty.
      m_in_tout_counts[0]++;
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in0_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else { // success
    m_in_tout_counts[0] = 0;
    recv_byte_size = m_in0_data.data.length();
    m_in0_status = BUF_SUCCESS;
  }
  if (m_Verbosity) {
    std::cerr << "m_in0_data.data.length():" << recv_byte_size
      << std::endl;
  }
  //here recv_byte_size includes header and footersize
  return recv_byte_size;

}

//READ data from NIMEASIROC2
unsigned int BestEffortDispatcher::read_InPort1()
{
  if (check_trans_lock()) set_trans_unlock();
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort1.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in1_status = check_inPort_status(m_InPort1);
    if (m_in1_status == BUF_TIMEOUT) { // Buffer empty.
      m_in_tout_counts[1]++;
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in1_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else { // success
    m_in_tout_counts[1] = 0;
    recv_byte_size = m_in1_data.data.length();
    m_in1_status = BUF_SUCCESS;
  }
  if (m_Verbosity) {
    std::cerr << "m_in1_data.data.length():" << recv_byte_size
      << std::endl;
  }

  return recv_byte_size;

}


//READ data from Drs4Qdc1
unsigned int BestEffortDispatcher::read_InPort2()
{
  if (check_trans_lock()) set_trans_unlock();
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort2.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in2_status = check_inPort_status(m_InPort2);
    if (m_in2_status == BUF_TIMEOUT) { // Buffer empty.
      m_in_tout_counts[2]++;
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in2_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else {
    m_in_tout_counts[2] = 0;
    recv_byte_size = m_in2_data.data.length();
    m_in2_status = BUF_SUCCESS;
  }
  if (m_Verbosity) {
    std::cerr << "m_in2_data.data.length():" << recv_byte_size
              << std::endl;
  }

  return recv_byte_size;
}

//READ data from Drs4Qdc2
unsigned int BestEffortDispatcher::read_InPort3()
{
  if (check_trans_lock()) set_trans_unlock();
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort3.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in3_status = check_inPort_status(m_InPort3);
    if (m_in3_status == BUF_TIMEOUT) { // Buffer empty.
      m_in_tout_counts[3]++;
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in3_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else {
    m_in_tout_counts[3] = 0;
    recv_byte_size = m_in3_data.data.length();
    m_in3_status = BUF_SUCCESS;
  }
  if (m_Verbosity) {
    std::cerr << "m_in3_data.data.length():" << recv_byte_size
              << std::endl;
  }

  return recv_byte_size;
}


//READ data from HulScaler
unsigned int BestEffortDispatcher::read_InPort4()
{
  if (check_trans_lock()) set_trans_unlock();
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort4.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in4_status = check_inPort_status(m_InPort4);
    if (m_in4_status == BUF_TIMEOUT) { // Buffer empty.
      m_in_tout_counts[4]++;
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in4_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else {
    m_in_tout_counts[4] = 0;
    recv_byte_size = m_in4_data.data.length();
    m_in4_status = BUF_SUCCESS;
  }
  if (m_Verbosity) {
    std::cerr << "m_in4_data.data.length():" << recv_byte_size
              << std::endl;
  }

  return recv_byte_size;
}



//______________________________________________________________________________
//send data  to outport 0 (=logger component)
int BestEffortDispatcher::write_OutPort()
{
  ////////////////// send data from OutPort  //////////////////
  bool ret = m_OutPort.write();

  //////////////////// check write status /////////////////////
  if (ret == false) {  // TIMEOUT or FATAL
    m_out_status  = check_outPort_status(m_OutPort);
    if (m_out_status == BUF_FATAL) {   // Fatal error
      fatal_error_report(OUTPORT_ERROR);
    }
    if (m_out_status == BUF_TIMEOUT) { // Timeout
      m_out0_tout_counts++;
      return -1;
    }
  }
  m_out0_tout_counts = 0;
  return 0; // successfully done
}


//______________________________________________________________________________
//out port for monitor with best effort type
int BestEffortDispatcher::write_BestEffort_OutPort()
{
  ////////////////// send data from OutPort  //////////////////
  bool ret = m_BestEffort_OutPort.write();

  //////////////////// check write status /////////////////////
  if (ret == false) {  // TIMEOUT or FATAL
    m_besteffort_out_status  = check_outPort_status(m_BestEffort_OutPort);
    if (m_besteffort_out_status == BUF_FATAL) {   // Fatal error
      fatal_error_report(OUTPORT_ERROR);
    }
    if (m_besteffort_out_status == BUF_TIMEOUT) { // Timeout
      m_out1_tout_counts++;
      return -1;
    }
  }
  m_out1_tout_counts = 0;
  return 0; // successfully done
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_run()
{
  if (m_Verbosity) {
    std::cerr << "*** BestEffortDispatcher::run" << std::endl;
  }
  
  if(m_Verbosity>=10){
    std::cerr << "out1_tout_counts:" << m_out1_tout_counts << "  bestEff_tout_counts:" << m_out1_tout_counts << std::endl;
  }

  //In best effort type, you don't need see bestoffort out port
  //Here, only timeout of out port for logger is checked
  if (m_out_status != BUF_TIMEOUT) {
    m_inport_recv_data_size[0] = read_InPort0();
    m_inport_recv_data_size[1] = read_InPort1();
    m_inport_recv_data_size[2] = read_InPort2();
    m_inport_recv_data_size[3] = read_InPort3();
    m_inport_recv_data_size[4] = read_InPort4();

    if ((m_inport_recv_data_size[0] == 0)
     || (m_inport_recv_data_size[1] == 0)  
     || (m_inport_recv_data_size[2] == 0)  
     || (m_inport_recv_data_size[3] == 0)  
     || (m_inport_recv_data_size[4] == 0)  
    ) { // TIMEOUT or buffer empty at least 1 module
      if (m_Verbosity>10) {
        int seqnum = get_sequence_num();
        std::cerr << "L. " << __LINE__ << " TIMEOUT: event sequence:" << seqnum << std::endl;  
        for(int ip = 0; ip<5;ip++){
          if(m_inport_recv_data_size[ip] == 0){
            std::cerr << " inport" << ip <<" timeout" << std::endl;
          }
        }
      }
      return 0;
    }else {
      if(m_Verbosity){
        std::cerr << __LINE__ << " start_build"  << std::endl;
      }
      //check header and footer 
      //They does not work.
      //check_header_footer(m_in0_data, m_inport_recv_data_size[0]);
      //check_header_footer(m_in1_data, m_inport_recv_data_size[1]);
      //check_header_footer(m_in2_data, m_inport_recv_data_size[2]);
      //check_header_footer(m_in3_data, m_inport_recv_data_size[3]);
      //check_header_footer(m_in4_data, m_inport_recv_data_size[4]);
      

      //event building 
      //take sum of inport0-4
      //total data size (before event building) =
      // header0 + data_body0 + footer0 + header1 + data_body1 + footer1
      // header2 + data_body2 + footer2 + header3 + data_body3 + footer3
      // header4 + data_body4 + footer4 
      //
      //total data size (after event building) = 
      // header + data_body0 + data_body1 
      //        + data_body2 + data_body3 
      //        + data_body4 + footer4 
      // 
      m_inport_recv_data_size_sum = 
      m_inport_recv_data_size[0] + 
      m_inport_recv_data_size[1] +
      m_inport_recv_data_size[2] +
      m_inport_recv_data_size[3] +
      m_inport_recv_data_size[4];
      if(m_Verbosity){
        std::cerr << __LINE__ << "  " << m_inport_recv_data_size_sum << std::endl;
      }
      unsigned int  send_data_size = m_inport_recv_data_size_sum - 4*HEADER_BYTE_SIZE -4*FOOTER_BYTE_SIZE;
      unsigned char newheader[HEADER_BYTE_SIZE];
      unsigned char newfooter[FOOTER_BYTE_SIZE];
      //IF data size should include header and footer size, if not, remove HEADER_BYTE_SIZE and FOOTER_BYTE_SIZE
      set_header(&newheader[0],send_data_size);
      set_footer(&newfooter[0]);

      //we don't check file size
      //if(send_data_size > SEND_BUFFER_SIZE){
      //  std::cerr << "buffer overflow ! sum of size " << send_data_size << std::endl;
      //}
      
      //set Outport buffer length
      m_in_data_sum.data.length(send_data_size);
      
      unsigned int sumdatapoint=0;
      //set new header with corrected data size
      memcpy(&m_in_data_sum.data[sumdatapoint],&newheader[0],HEADER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << " copy header "  << std::endl;
      }
      sumdatapoint += HEADER_BYTE_SIZE;
      //set data from inport 0 (NIMEASIROC1)
      //memcpy(&m_in_data_sum.data[sumdatapoint],&m_in0_data.data[HEADER_BYTE_SIZE],m_inport0_recv_data_size-FOOTER_BYTE_SIZE);
      memcpy(&m_in_data_sum.data[sumdatapoint],&m_in0_data.data[HEADER_BYTE_SIZE],m_inport_recv_data_size[0]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << " copy in 0 " << sumdatapoint << std::endl;
      }
      //set data from inport 1 (NIMEASIROC2)
      sumdatapoint += m_inport_recv_data_size[0]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE;
      memcpy(&m_in_data_sum.data[sumdatapoint],&m_in1_data.data[HEADER_BYTE_SIZE],m_inport_recv_data_size[1]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << " copy in 1 " << sumdatapoint << std::endl;
      }
      //set data from inport 2 (Drs4Qdc1)
      sumdatapoint += m_inport_recv_data_size[1]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE;
      memcpy(&m_in_data_sum.data[sumdatapoint],&m_in2_data.data[HEADER_BYTE_SIZE],m_inport_recv_data_size[2]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << " copy in 2 " << sumdatapoint << std::endl;
      }
      
      //set data from inport 3 (Drs4Qdc2)
      sumdatapoint += m_inport_recv_data_size[2]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE;
      memcpy(&m_in_data_sum.data[sumdatapoint],&m_in3_data.data[HEADER_BYTE_SIZE],m_inport_recv_data_size[3]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << " copy in 3 " << sumdatapoint << std::endl;
      }
      //set data from inport 4 (HulScaler)
      sumdatapoint += m_inport_recv_data_size[3]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE;
      memcpy(&m_in_data_sum.data[sumdatapoint],&m_in4_data.data[HEADER_BYTE_SIZE],m_inport_recv_data_size[4]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << " copy in 4 " << sumdatapoint << std::endl;
      }
      
      //set new footer
      sumdatapoint += m_inport_recv_data_size[4]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE;
      memcpy(&m_in_data_sum.data[sumdatapoint],&newfooter[0],FOOTER_BYTE_SIZE);
      if(m_Verbosity){
        std::cerr << __LINE__ << "recv size   " << m_inport_recv_data_size_sum << std::endl;
        std::cerr << "NER   " <<    
        m_inport_recv_data_size[0]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE+
        m_inport_recv_data_size[1]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE<< std::endl;
        std::cerr << "DRS4_1 "  << m_inport_recv_data_size[2]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE << std::endl;
        std::cerr << "DRS4_1 check function "  << get_event_size(m_inport_recv_data_size[2])   << std::endl;
        std::cerr << "DRS4_2 "  << m_inport_recv_data_size[3]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE << std::endl;
        std::cerr << "HUL   "  << m_inport_recv_data_size[4]-HEADER_BYTE_SIZE-FOOTER_BYTE_SIZE << std::endl;
        std::cerr << "send_data_size " <<  send_data_size  << std::endl;
        std::cerr << "set footer " <<  sumdatapoint << std::endl;
        std::cerr << "send_data_size  - FOOTER_BYTE_SIZE " <<  send_data_size - FOOTER_BYTE_SIZE << std::endl;
      }
      
      set_data_OutPort(send_data_size);
      set_data_BestEffort_OutPort(send_data_size);
      //m_v1718->OnePulse();
    }//if not time out
  }
  
  if ( (m_in0_status != BUF_TIMEOUT)
   &&  (m_in1_status != BUF_TIMEOUT)
   &&  (m_in2_status != BUF_TIMEOUT)
   &&  (m_in3_status != BUF_TIMEOUT)
   &&  (m_in4_status != BUF_TIMEOUT)
  ) {
    if (write_OutPort() < 0) { // TIMEOUT
      ; // do nothing
      if(m_Verbosity){
        std::cerr << "L." << __LINE__ << "write_outport0 timeout " << std::endl;
      }
    }
    else {
      m_out_status = BUF_SUCCESS;
    }
  }
  
  //If Outport for logger is OK, then send data to the best effort outport (monitor)
  if ( (m_in0_status != BUF_TIMEOUT)  
    && (m_in1_status != BUF_TIMEOUT) 
    && (m_in2_status != BUF_TIMEOUT) 
    && (m_in3_status != BUF_TIMEOUT) 
    && (m_in4_status != BUF_TIMEOUT) 
    && (m_out_status != BUF_TIMEOUT)) {
    if (write_BestEffort_OutPort() < 0) { // TIMEOUT
      ; // do nothing
      if(m_Verbosity){
        std::cerr << "L." << __LINE__ << "write_outport0 timeout " << std::endl;
      }
    }
    else {
      m_besteffort_out_status = BUF_SUCCESS;
    }
  }
  
  //check all inport and outport for logger status
  //do not check best effort type outport
  if ( (m_in0_status != BUF_TIMEOUT)  
    && (m_in1_status != BUF_TIMEOUT) 
    && (m_in2_status != BUF_TIMEOUT) 
    && (m_in3_status != BUF_TIMEOUT) 
    && (m_in4_status != BUF_TIMEOUT) 
    && (m_out_status != BUF_TIMEOUT)) {
      
    inc_sequence_num();                    // increase sequence num.
    //unsigned int event_data_size = get_event_size(m_inport_recv_data_size_sum);
    unsigned int  send_data_size = m_inport_recv_data_size_sum - 4*HEADER_BYTE_SIZE -4*FOOTER_BYTE_SIZE;
    
    inc_total_data_size(send_data_size);  // increase total data byte size

    int seqnum = get_sequence_num();
    if (seqnum%1000 == 0)
      std::cout << seqnum << " events processed. " << std::endl;
  }
  //then, veto clear

  return 0;

}


int BestEffortDispatcher::reset_InPort0()
{
    TimedOctetSeq dummy_data;

    while (m_InPort0.isEmpty() == false) {
        m_InPort1 >> dummy_data;
    }
    //std::cerr << "*** Dispatcher::InPort flushed\n";
    return 0;
}

int BestEffortDispatcher::reset_InPort1()
{
    TimedOctetSeq dummy_data;

    while (m_InPort1.isEmpty() == false) {
        m_InPort1 >> dummy_data;
    }
    //std::cerr << "*** Dispatcher::InPort flushed\n";
    return 0;
}

int BestEffortDispatcher::reset_InPort2()
{
    TimedOctetSeq dummy_data;

    while (m_InPort2.isEmpty() == false) {
        m_InPort2 >> dummy_data;
    }
    //std::cerr << "*** Dispatcher::InPort flushed\n";
    return 0;
}

int BestEffortDispatcher::reset_InPort3()
{
    TimedOctetSeq dummy_data;

    while (m_InPort3.isEmpty() == false) {
        m_InPort3 >> dummy_data;
    }
    //std::cerr << "*** Dispatcher::InPort flushed\n";
    return 0;
}


int BestEffortDispatcher::reset_InPort4()
{
    TimedOctetSeq dummy_data;

    while (m_InPort4.isEmpty() == false) {
        m_InPort4 >> dummy_data;
    }
    //std::cerr << "*** Dispatcher::InPort flushed\n";
    return 0;
}


//______________________________________________________________________________
extern "C"
{
  void BestEffortDispatcherInit(RTC::Manager* manager)
  {
    RTC::Properties profile(besteffortdispatcher_spec);
    manager->registerFactory(profile,
                             RTC::Create<BestEffortDispatcher>,
                             RTC::Delete<BestEffortDispatcher>);
  }
};
