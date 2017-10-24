// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <sstream>


// DRS4QDC control/daq library written by R. Honda
#include "RegisterMap.hh"
#include "common_val.hh"

#include "SiTcpRbcp.hh"
#include "getaddr.h"
#include "HexDump.hh"

#include "Drs4QdcReader1.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

const unsigned int kIdMagic      = 0x504d4f43; // 'COMP' (PMOC) in ascii code
const unsigned int kDrs4QdcMagic = 0xffff9dc4;
const unsigned int kTdcMagic     = 0x40434454; // 'TDC@' (@CDT) in ascii code

//______________________________________________________________________________
// mask/shift for FPGA module register address offset calculation
static const unsigned int module_id_mask  = 0xF;
static const unsigned int module_id_shift = 28U; // <<

static const unsigned int address_mask    = 0xFFF;
static const unsigned int address_shift   = 16U;  // <<

static const unsigned int exdata_mask     = 0xFFFF00;
static const unsigned int exdata_shift    = 8U;  // >>

static const unsigned int data_mask       = 0xFF;

static const int full_header_size  = 8;
static const int short_header_size = 4;

static const unsigned int kTdcNWordEventHeader = 6;

//______________________________________________________________________________
bool
checkTdcHeader(const std::vector<unsigned int>& header)
{
  bool ret = true;
  ret = (header.size() == kTdcNWordEventHeader);
  ret = (header[0] == kTdcMagic);
  ret = (((header[1]>>28) & 0xf) == 0x5);
  ret = (((header[2]>>28) & 0xf) == 0x6);
  ret = (((header[3]>>28) & 0xf) == 0x7);
  ret = (((header[4]>>28) & 0xf) == 0x7);
  ret = (((header[5]>>28) & 0xf) == 0xC) || (((header[5]>>28) & 0xf) == 0xD);
  return ret;
}

//______________________________________________________________________________
unsigned int getOffsetAddress(unsigned int module_id,
                              unsigned int local_address,
                              unsigned int write_data)
{
  unsigned int ret
    = ((module_id & module_id_mask) << module_id_shift)
    + ((local_address & address_mask) << address_shift)
    + ((write_data & exdata_mask) >> exdata_shift);
  std::cout << "#D address = " << std::hex << ret << std::dec << std::endl;
  return ret;
}

//______________________________________________________________________________
// Module specification
// Change following items to suit your component's spec.
static const char* drs4qdc1reader_spec[] =
  {
    "implementation_id", "Drs4QdcReader1",
    "type_name",         "Drs4QdcReader1",
    "description",       "Drs4QdcReader1 component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };

//______________________________________________________________________________
Drs4QdcReader1::Drs4QdcReader1(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_OutPort("out2", m_out_data),
    m_sock(nullptr),
    m_data(),
    m_header(), 
    m_recv_byte_size(0),
    m_out_status(BUF_SUCCESS),
    m_debug(false),
    m_srcPort(0),
    m_srcAddr(),
    m_cmd_daqreg(),
    m_rbcp(nullptr),
    //    m_use_ext_trigger(false), 
    m_headerSize(-1),
    m_hasBoardId(false),
    m_isTdcDisabled(false), 
    m_boardId(0),
    m_maxEvent(0),
    m_cmd_open_gate(),
    m_cmd_close_gate()
{
  // Registration: InPort/OutPort/Service

  // Set OutPort buffers
  registerOutPort("out2", m_OutPort);

  init_command_port();
  init_state_table();
  set_comp_name("Drs4QdcReader1");
}

//______________________________________________________________________________
Drs4QdcReader1::~Drs4QdcReader1()
{
  delete m_sock;
  m_sock = nullptr;
}

//______________________________________________________________________________
RTC::ReturnCode_t Drs4QdcReader1::onInitialize()
{
  if (m_debug) {
    std::cerr << "Drs4QdcReader1::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t Drs4QdcReader1::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_dummy()
{
  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_configure()
{
  std::cerr << "*** Drs4QdcReader1::configure" << std::endl;

  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // resolve board ID
  m_boardId = getaddr_ipv4(m_srcAddr.c_str());
  m_srcAddr = getaddr_ipv4_str(m_srcAddr.c_str());
  std::cout << "#D src = " << m_srcAddr << std::endl;
  std::cout << "#D board ID = " << std::hex << m_boardId << std::dec
            << " " << m_srcAddr << std::endl;

  // register configuration via SiTCP RBCP
  if (!m_rbcp) {
    m_rbcp = new SiTcpRbcp(m_srcAddr, SiTcpRbcp::kDefaultPort);
  }
  changeDaqMode(false);
  // call set_daqreg

  {
    std::string cmd_daqreg = m_cmd_daqreg + " " + m_srcAddr;
    std::cout << "#D do " << cmd_daqreg << std::endl;
    std::system(cmd_daqreg.c_str());
    std::cout << "#D done " << cmd_daqreg << std::endl;
  }
  
  

  isTdcDisabled();
  resetDrs4Address();
  getHeaderSize();
  resetFIFO();
  
  //  if (m_use_ext_trigger)
  changeDaqMode(true);

  try {
    // create socket and connect to frontend
    m_sock = new DAQMW::Sock;
    std::cout << "#D sockent connect: " << m_srcAddr << " " << m_srcPort << std::endl;
    m_sock->connect(m_srcAddr, m_srcPort);
  } catch (DAQMW::SockException& e) {
    std::cerr << "Sock Fatal Error: " << e.what() << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "TCP SOCKET CONNECT FATAL ERROR");
  } catch (...) {
    std::cerr << "Sock Fatal Error: Unknown" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "TCP SOCKET CONNECT FATAL ERROR UNKNOWN");
  }
  m_sock->setOptRecvTimeOut(10.0); // 10 seconds
  std::cout << "************************" << std::endl;
  std::cout << "Drs4QdcReader Initialized " << std::endl;
  std::cout << "************************" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::parse_params(::NVList* list)
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

    if (sname=="srcAddr2") {
      srcAddrSpecified = true;
      // remove white space at the beginning of the svalue    
      std::stringstream ss(svalue); 
      ss >> svalue;
      if (m_debug) {
        std::cerr << "source addr:" << svalue << std::endl;
      }
      m_srcAddr = svalue;
    }
    if (sname=="srcPort2") {
      srcPortSpecified = true;
      if (m_debug) {
        std::cerr << "source port:" << svalue << std::endl;
      }
      m_srcPort = std::stoul(svalue);
    }
    if (sname=="hasBoardId") {
      std::stringstream ss(svalue); 
      ss >> svalue;
      if ((svalue=="true") || (svalue=="TRUE") || (svalue=="1"))
        m_hasBoardId = true;
    }
    if (sname=="execSetDaqReg") {
      m_cmd_daqreg = svalue;
    }
    // if (sname=="ext_trigger") {
    //   std::stringstream ss(svalue); 
    //   ss >> svalue;
    //   if ((svalue=="true") || (svalue=="TRUE") || (svalue=="1"))
    //     m_use_ext_trigger = true;
    // }

    if ((sname=="max_event") || (sname=="maxEvent") || (sname=="MaxEvent")) {
      m_maxEvent = std::stoul(svalue);
    }
    if (sname=="open_gate") {
      m_cmd_open_gate = svalue;
    }
    if (sname=="close_gate") {
      m_cmd_close_gate = svalue;
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
  if (m_cmd_daqreg.empty()) {
    std::cerr << "### ERROR: DRS4 DAQ registers are not configured\n";
    fatal_error_report(USER_DEFINED_ERROR2, "INVALID DAQ REG");
  }


  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_unconfigure()
{
  std::cerr << "*** Drs4QdcReader1::unconfigure" << std::endl;


  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_start()
{
  std::cerr << "*** Drs4QdcReader1::start" << std::endl;
  /*
  try {
    // create socket and connect to frontend
    m_sock = new DAQMW::Sock;
    std::cout << "#D sockent connect: " << m_srcAddr << " " << m_srcPort << std::endl;
    m_sock->connect(m_srcAddr, m_srcPort);
  } catch (DAQMW::SockException& e) {
    std::cerr << "Sock Fatal Error: " << e.what() << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "TCP SOCKET CONNECT FATAL ERROR");
  } catch (...) {
    std::cerr << "Sock Fatal Error: Unknown" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "TCP SOCKET CONNECT FATAL ERROR UNKNOWN");
  }*/

  m_out_status = BUF_SUCCESS;


  // check data port connections
  bool is_connected = check_dataPort_connections(m_OutPort);
  if (!is_connected) {
    std::cerr << "### NO Connection to OutPort " << m_OutPort.getName() << std::endl;
    fatal_error_report(DATAPATH_DISCONNECTED);
  }

  //if (!m_use_ext_trigger)
  //changeDaqMode(true);

  if (!m_cmd_open_gate.empty()) {
    std::cout << " open external daq gate" << std::endl;
    std::system(m_cmd_open_gate.c_str());
  }
  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_stop()
{
  std::cerr << "*** Drs4QdcReader1::stop" << std::endl;
  changeDaqMode(false);
  resetFIFO();

  if (m_sock) {
    m_sock->disconnect();
    delete m_sock;
    m_sock = nullptr;
  }
  std::cout << "total sequence # " << get_sequence_num() << std::endl;

  resetDrs4Address();
  
  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_pause()
{
  std::cerr << "*** Drs4QdcReader1::pause" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_resume()
{
  std::cerr << "*** Drs4QdcReader1::resume" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::read_adc()
{
  int received_data_size = 0;
  //  std::vector<unsigned int> header(m_headerSize);

  if (m_header.empty()) {
    m_header.resize(m_headerSize);
    
    // read header
    int status = m_sock->readAll(reinterpret_cast<unsigned char*>(&(m_header[0])), sizeof(unsigned int)*m_headerSize);
    //  std::cout << "#D header read done" << std::endl;
    // for (auto x: header){
    //   std::cout << std::hex << x << " " << std::dec;
    // }
    // std::cout << std::endl;
  
    if (status == DAQMW::Sock::ERROR_FATAL) {
      std::cerr << "### ERROR: m_sock->readAll() adc header " << get_sequence_num() << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
      std::cerr << "### Timeout: m_sock->readAll() adc header " << get_sequence_num() << std::endl;
      // fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
      m_header.clear();
      return 0;
    } else if (status == DAQMW::Sock::ERROR_NOTSAMESIZE) {
      std::cerr << "### ERROR: m_sock->readAll() adc header not same size "
                << get_sequence_num()
                << " req = " << sizeof(unsigned int)*m_headerSize
                << ", ret = " << status
                << std::endl;
    } else {
      received_data_size += m_header.size() * sizeof(unsigned int);
    }
    
  }

  // header check
  if (m_header[0] != kDrs4QdcMagic) {
    std::cerr << "#E read_adc() invalid header " << get_sequence_num() << std::endl;
    std::for_each(m_header.begin(), m_header.end(), hddaq::HexDump());
    fatal_error_report(USER_DEFINED_ERROR1, "adc header magic");
  }

  // decode data size
  unsigned int n_word_data = m_header[1] & 0xffff;
  unsigned int sizeData    = sizeof(unsigned int)*n_word_data;
  //  std::cout << "#D N words to be read " << n_word_data << " (" << sizeData << " byte)" << std::endl;

  int hsize = 0;
  if (m_hasBoardId) {
    hsize = 2 + m_headerSize;
    // read body
  } else {
    hsize = m_headerSize;
  }
  //usleep(100);
  if (!m_header.empty() && m_data.empty()) {
    m_data.resize(hsize + n_word_data);  // number of int words
    m_data[0] = kIdMagic;
    m_data[1] = m_boardId;
    std::copy(m_header.begin(), m_header.end(), m_data.begin() + (hsize-m_headerSize));
    // read body
    int status = m_sock->readAll(reinterpret_cast<unsigned char*>(&(m_data[hsize])), sizeData);

  
    if (status == DAQMW::Sock::ERROR_FATAL) {
      std::cerr << "### ERROR: m_sock->readAll() adc body "
                << get_sequence_num() << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
      std::cerr << "### Timeout: m_sock->readAll() adc body" << std::endl;
      //fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
      m_data.clear();
      return 0;
    } else if (status == DAQMW::Sock::ERROR_NOTSAMESIZE) {
      std::cerr << "### ERROR: m_sock->readAll() adc body not same size "
                << get_sequence_num()
                << " req = " << sizeData
                << " ret = " << status
                << std::endl;
       std::cout << "adc data (all)" << std::endl;
       std::for_each(m_data.begin(), m_data.end(), hddaq::HexDump());

      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR NOT SAME SIZE");
    } else {

      // std::cout << "adc data (all)" << std::endl;
      // std::for_each(m_data.begin(), m_data.end(), hddaq::HexDump());
      
      received_data_size = m_data.size() * sizeof(unsigned int);
    }
    //  std::cout << "#D read body done " << received_data_size << " byte" << std::endl;
  }
  
  return received_data_size;
}

//______________________________________________________________________________
int Drs4QdcReader1::read_tdc()
{
  if (m_isTdcDisabled) {
    return 0;
  }
  // ---------- event header ----------
  // magic                       0x40434454
  // local L1 event counter      0x5XXXXXXX
  // local L2 event counter      0x6XXXXXXX
  // L1 trigger time stamp       0x7XXXXXXX
  // L1 trigger time stamp (2nd) 0x7XXXXXXX
  // frame size                  0xCXXXXXXX
  // frame size of last frame    0xDXXXXXXX
  
  // tdc                         0x3XXXXXXX

  // ---------- event trailer ----------
  // error                     0x9XXXXXXX
  // number of hits in 1 event 0xAXXXXXXX
  // event trailer             0xBA3A3A3A

  static const unsigned int
    sizeHeader = sizeof(unsigned int)*kTdcNWordEventHeader;

  std::vector<unsigned int> buffer(kTdcNWordEventHeader);
  
  for (int itry=0;;++itry) {
    int status = m_sock->readAll(reinterpret_cast<unsigned char*>(&(buffer[0])), sizeHeader);

    if (status == DAQMW::Sock::ERROR_FATAL) {
      std::cerr << "### ERROR: m_sock->readAll() tdc header "
                << "(nseq = " << get_sequence_num() 
                << ", i = " << itry << ")" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
      std::cerr << "### Timeout: m_sock->readAll() tdc header "
                << "(nseq = " << get_sequence_num() 
                << ", i = " << itry << ")" << std::endl;
      std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
    } else if (status == DAQMW::Sock::ERROR_NOTSAMESIZE) {
      std::cerr << "### ERROR: m_sock->readAll() tdc header not same size"
                << "(nseq = " << get_sequence_num() 
                << ", i = " << itry << ")"
                << " req-size = "  << sizeHeader << ", ret-size = " << status
                << std::endl;
      std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR NOT SAME SIZE");
    } else
      break;
  }

  // TDC header check
  if (!checkTdcHeader(buffer)) {
    std::cout << "#E read_tdc() invalid header "
              << get_sequence_num() << std::endl;
    std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
    fatal_error_report(USER_DEFINED_ERROR1, "tdc header magic");
  }

  
  bool isLastFrame = false;
  // read body ---------------------------------------------------------
  for (int itry=0;;++itry) {
    unsigned int data_type   = (buffer.back()>>28) & 0xf;
    if (data_type != 0xc && data_type != 0xd) {
      int n = buffer.size();
      std::cout << "#E @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
                << " read_tdc() invalid data type of frame size "
                << " itry = " << itry << std::endl;
      std::cout << "#D tdc (all) "
                << get_sequence_num() << std::endl;
      std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
      fatal_error_report(USER_DEFINED_ERROR1, "TDC INVALID DATA");
      //break;
    }
    unsigned int frame_id    = (buffer.back()>>16) & 0xfff;
    unsigned int n_word_data = buffer.back() & 0xfff;
    isLastFrame              = (data_type == 0xd);

    // std::cout << "#D frame_id = " << frame_id
    //           << ", n word = " << n_word_data << "(" << std::hex << n_word_data << std::dec << ")"
    //           << ", last = " << isLastFrame << std::endl;
    
    int widx = buffer.size();
    //    if (!isLastFrame && (frame_id == 0)) n_word_data += 1;
    if (!isLastFrame) n_word_data += 1;
    //    else n_word_data -= 1;
    if (frame_id==0)  n_word_data -= kTdcNWordEventHeader;
    else n_word_data -= 1;
    buffer.resize(widx + n_word_data);

    // std::cout << "#D frame_id = " << frame_id
    //           << ", n word = " << n_word_data << "(" << std::hex << n_word_data << std::dec << ")"
    //           << ", last = " << isLastFrame << std::endl;
    
    // std::cout << "#D n word to be read = " << n_word_data << std::endl;

    
    unsigned int sizeData = sizeof(unsigned int)*(n_word_data);
    //    std::cout << "#D n word =" << n_word_data << " to be read" << std::endl;
    for (;;) {
      int status = m_sock->readAll(reinterpret_cast<unsigned char*>(&buffer[widx]), sizeData);
      //      std::cout << "#D Event_Cycle_tdc (tmp) " << n_word_data << " " << ret << std::endl;
      if (status == DAQMW::Sock::ERROR_FATAL) {
        std::cerr << "### ERROR: m_sock->readAll() tdc body" << std::endl;
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
      } else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
        std::cerr << "### Timeout: m_sock->readAll() tdc body timeout "
                  << get_sequence_num()
                  << std::endl;
      } else if (status == DAQMW::Sock::ERROR_NOTSAMESIZE) {
        std::cerr << "### ERROR: m_sock->readAll() tdc body not same size "
                  << get_sequence_num()
                  << " req = " << sizeData
                  << ", ret = " << status
                  << std::endl;
        std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
        fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR NOT SAME SIZE");
      } else
        break;
    }

    // std::cout << "#D tdc read (all)" << std::endl;
    // std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
    if (isLastFrame) break;
  }
  //  sleep(1);

  // std::cout << "#D tdc read (all)" << std::endl;
  // std::for_each(buffer.begin(), buffer.end(), hddaq::HexDump());
  
  std::copy(buffer.begin(), buffer.end(), std::back_inserter(m_data));

  return (buffer.size() * sizeof(unsigned int));
  
}

//______________________________________________________________________________
int Drs4QdcReader1::set_data(unsigned int data_byte_size)
{
  //  std::cout << "run number = " << get_run_number() << std::endl;
  
  // std::cout << "#D set_data : seq = " << get_sequence_num() << std::endl;
  // std::cout << "#D byte = " << data_byte_size << " (" << (data_byte_size/sizeof(unsigned int))
  //           << ") , n word = " << m_data.size() << std::endl;
  // std::for_each(m_data.begin(), m_data.end(), hddaq::HexDump());
  
  unsigned char header[8];
  unsigned char footer[8];

  set_header(&header[0], data_byte_size);
  set_footer(&footer[0]);

  ///set OutPort buffer length
  m_out_data.data.length(data_byte_size + HEADER_BYTE_SIZE + FOOTER_BYTE_SIZE);
  memcpy(&(m_out_data.data[0]), &header[0], HEADER_BYTE_SIZE);
  memcpy(&(m_out_data.data[HEADER_BYTE_SIZE]), reinterpret_cast<unsigned char*>(&m_data[0]), data_byte_size);
  memcpy(&(m_out_data.data[HEADER_BYTE_SIZE + data_byte_size]), &footer[0],
         FOOTER_BYTE_SIZE);

  //  std::cout << "#D set_data() " << m_out_data.data.length() << " byte" << std::endl;
  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::write_OutPort()
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
      return -1;
    }
  }
  else {
    m_out_status = BUF_SUCCESS; // successfully done
  }
  //  std::cout << "#D write_OutPort() BUF_SUCCESS" << std::endl;
  return 0;
}

//______________________________________________________________________________
int Drs4QdcReader1::daq_run()
{
  if (m_debug) {
    std::cerr << "*** Drs4QdcReader1::run" << std::endl;
  }

  if (check_trans_lock()) {  // check if stop command has come
    set_trans_unlock();    // transit to CONFIGURED state
    return 0;
  }

  if (m_maxEvent>0) {
    if (m_maxEvent == get_sequence_num()) {
      std::string msg(" event number reaches maximum setting. ");
      std::cout << msg << std::endl;
      if (!m_cmd_close_gate.empty()) {
        std::cout << " close daq gate" << std::endl;
        std::system(m_cmd_close_gate.c_str());
      }
      fatal_error_report(USER_DEFINED_ERROR1, msg.c_str());
      return -1;
    }
  }

  
  if (m_out_status == BUF_SUCCESS) {   // previous OutPort.write() successfully done
    m_recv_byte_size  = read_adc();
    if (m_recv_byte_size == 0) {
      return 0;
    }
    m_recv_byte_size += read_tdc();
    if (m_recv_byte_size > 0) {
      set_data(m_recv_byte_size); // set data to OutPort Buffer
    }
  }

  if (write_OutPort() < 0) {
    ;     // Timeout. do nothing.
  }
  else {    // OutPort write successfully done
    inc_sequence_num();                     // increase sequence num.
    inc_total_data_size(m_recv_byte_size);  // increase total data byte size
    //std::cout << "L." << __LINE__ << " " <<  m_recv_byte_size <<  std::endl;
    m_header.clear();
    m_data.clear();

    //std::cout << get_sequence_num() << " events processed." << std::endl;
    int seqnum = get_sequence_num();
    if (seqnum%1000==0)
      std::cout << seqnum << " events processed." << std::endl;
  }

  return 0;
}

//______________________________________________________________________________
void
Drs4QdcReader1::changeDaqMode(bool on)
{
  unsigned char buf1;
  int len = 1;
  if (on) {
    buf1 = 1;
    std::cout << "Turn on DAQ mode" << std::endl;
  } else {
    buf1 = 0;
    std::cout << "Turn off DAQ mode" << std::endl;
  }

  m_rbcp->write(getOffsetAddress(DAQ::mid, DAQ::laddr_ExecDAQ, buf1), &buf1, len);
}
  
//______________________________________________________________________________
bool
Drs4QdcReader1::isTdcDisabled()
{
  unsigned char buf1 = 0;
  int len = 1;
  int status = m_rbcp->read(getOffsetAddress(TDC::mid, TDC::laddr_disable_tdc, 0), &buf1, len);
  m_isTdcDisabled = buf1 & 0x1;
  //  m_isTdcDisabled = true;
  return m_isTdcDisabled;
}

//______________________________________________________________________________
unsigned int
Drs4QdcReader1::getHeaderSize()
{
  unsigned char buf1 = 0;
  int len = 1;
  std::cout << "Get header size " << std::endl;
  m_rbcp->read(getOffsetAddress(EVB::mid, EVB::laddr_control, 0), &buf1, len);
  if (0 == (buf1 & EVB::c_short_header)) {
    m_headerSize = full_header_size;
    std::cout << " full header " << std::endl;
  }
  else {
    m_headerSize = short_header_size;
    std::cout << " short header " << std::endl;
  }
}
//______________________________________________________________________________
void
Drs4QdcReader1::resetDrs4Address()
{
  std::cout << "Reset DRS4 address" << std::endl;
  unsigned int drsReg = (DRS::addr_mux_ch8 << exdata_shift);
  unsigned char buf1 = drsReg & data_mask;
  int len = 1;
  m_rbcp->write(getOffsetAddress(DRS::mid, DRS::laddr_register+DRS::laddr_drsaddr, drsReg), &buf1, len);
}

//______________________________________________________________________________
void
Drs4QdcReader1::resetFIFO()
{
  // User reset
  std::cout << "Reset user FIFO" << std::endl;
  unsigned char buf1 = 0;
  int len = 1;
  m_rbcp->write(getOffsetAddress(EVB::mid, EVB::laddr_reset, 0),  &buf1, len);
}

//______________________________________________________________________________
extern "C"
{
  void Drs4QdcReader1Init(RTC::Manager* manager)
  {
    RTC::Properties profile(drs4qdc1reader_spec);
    manager->registerFactory(profile,
                             RTC::Create<Drs4QdcReader1>,
                             RTC::Delete<Drs4QdcReader1>);
  }
};
