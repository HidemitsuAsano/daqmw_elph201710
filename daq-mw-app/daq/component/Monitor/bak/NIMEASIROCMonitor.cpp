// -*- C++ -*-
/*!
 * @file NIMEASIROCMonitor.cpp
 * @brief
 * @date Jan. 20th, 2017
 * @author Hidemitsu Asano
 *
 */
#include "NIMEASIROCMonitor.h"

#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::HEADER_DATA_MISMATCH;
using DAQMW::FatalType::FOOTER_DATA_MISMATCH;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

// Module specification
// Change following items to suit your component's spec.
static const char* NIMEASIROCmonitor_spec[] =
{
    "implementation_id", "NIMEASIROCMonitor",
    "type_name",         "NIMEASIROCMonitor",
    "description",       "NIMEASIROCMonitor component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

NIMEASIROCMonitor::NIMEASIROCMonitor(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_InPort("NIMEASIROCmonitor_in",   m_in_data),
      m_in_status(BUF_SUCCESS),
      m_canvas(0),
      m_hist(0),
      m_bin(0),
      m_min(0),
      m_max(0),
      m_monitor_update_rate(30),
      m_event_byte_size(0),
      m_Nmodule(2),
      m_readoutch(128),// 64 * 2 modules
      //m_readoutch(64),// 64 * 1 modules
      m_isSaveFile(true),
      m_isSaveTree(true),
      m_tfile(NULL),
      m_tree(NULL),
      m_debug(false)
{
    // Registration: InPort/OutPort/Service

    // Set InPort buffers
    registerInPort ("NIMEASIROCmonitor_in",  m_InPort);

    init_command_port();
    init_state_table();
    set_comp_name("NIMEASIROCMONITOR");

}

NIMEASIROCMonitor::~NIMEASIROCMonitor()
{
}

RTC::ReturnCode_t NIMEASIROCMonitor::onInitialize()
{
    if (m_debug) {
        std::cerr << "NIMEASIROCMonitor::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t NIMEASIROCMonitor::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int NIMEASIROCMonitor::daq_dummy()
{
    if (m_canvas) {
        m_canvas->Update();
        // daq_dummy() will be invoked again after 10 msec.
        // This sleep reduces X servers' load.
        sleep(1);
    }

    return 0;
}

int NIMEASIROCMonitor::daq_configure()
{
    std::cerr << "*** NIMEASIROCMonitor::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);

    return 0;
}

int NIMEASIROCMonitor::parse_params(::NVList* list)
{

    std::cerr << "param list length:" << (*list).length() << std::endl;

    int len = (*list).length();
    for (int i = 0; i < len; i+=2) {
        std::string sname  = (std::string)(*list)[i].value;
        std::string svalue = (std::string)(*list)[i+1].value;

        std::cerr << "sname: " << sname << "  ";
        std::cerr << "value: " << svalue << std::endl;
        
        if (sname == "monitorUpdateRate") {
            if (m_debug) {
                std::cerr << "monitor update rate: " << svalue << std::endl;
            }
            char *offset;
            m_monitor_update_rate = (int)strtol(svalue.c_str(), &offset, 10);
        }
        // If you have more param in config.xml, write here
    }

    return 0;
}

int NIMEASIROCMonitor::daq_unconfigure()
{
    std::cerr << "*** NIMEASIROCMonitor::unconfigure" << std::endl;
    if (m_canvas) {
        delete m_canvas;
        m_canvas = 0;
    }

    if (m_hist) {
        delete m_hist;
        m_hist = 0;
    }
    return 0;
}

int NIMEASIROCMonitor::daq_start()
{
    std::cerr << "*** NIMEASIROCMonitor::start" << std::endl;

    m_in_status  = BUF_SUCCESS;

    //////////////// CANVAS FOR HISTOS ///////////////////
    if (m_canvas) {
        delete m_canvas;
        m_canvas = NULL;
    }
    m_canvas = new TCanvas("c1", "histos", 0, 0, 600, 400);
    m_canvas->Divide(2,1);
    /*
    ////////////////       HISTOS      ///////////////////
    if (m_hist) {
        delete m_hist;
        m_hist = NULL;
    }

    int m_hist_bin = 100;
    double m_hist_min = 0.0;
    double m_hist_max = 1000.0;

    gStyle->SetStatW(0.4);
    gStyle->SetStatH(0.2);
    gStyle->SetOptStat("em");

    m_hist = new TH1F("hist", "hist", m_hist_bin, m_hist_min, m_hist_max);
    m_hist->GetXaxis()->SetNdivisions(5);
    m_hist->GetYaxis()->SetNdivisions(4);
    m_hist->GetXaxis()->SetLabelSize(0.07);
    m_hist->GetYaxis()->SetLabelSize(0.06);
    */

    gStyle->SetOptStat("em");
    if(m_isSaveFile){
      std::ostringstream filename;
      time_t now = time(NULL);
      struct tm *pnow = localtime(&now);
      filename << "root/" << pnow->tm_year+1900 << pnow->tm_mon+1 << pnow->tm_mday;
      int runnumber = get_run_number();
      filename << "_run" << runnumber <<".root";
      m_tfile = new TFile(filename.str().c_str(),"RECREATE");
    }

    int nbin = 4096;
    for(int i =0; i<m_readoutch; i++){
      gPad->SetLogy();
      m_adcHigh[i] = new TH1I(Form("ADC_HIGH_%d", i),
          Form("ADC high gain %d", i),
          nbin, 0, 4096);

      gPad->SetLogy();
      m_adcLow[i] = new TH1I(Form("ADC_LOW_%d", i),
          Form("ADC low gain %d", i),
          nbin, 0, 4096);
      m_tdcLeading[i] = new TH1I(Form("TDC_LEADING_%d", i),
          Form("TDC leading %d", i),
          nbin, 0, 4096);
      m_tdcTrailing[i] = new TH1I(Form("TDC_TRAILING_%d", i),
          Form("TDC trailing %d", i),
          nbin, 0, 4096);
    }
    if(m_isSaveTree){
      m_tree = new TTree("evttree","evttree");
      //m_tree->Branch("evtnum",
    }



    return 0;
}

int NIMEASIROCMonitor::daq_stop()
{
    std::cerr << "*** NIMEASIROCMonitor::stop" << std::endl;

    //m_hist->Draw();
    //m_canvas->Update();

    reset_InPort();

    m_tfile->Write();
    m_tfile->Close();
    std::cout << "total sequence # " << get_sequence_num() << std::endl;

    return 0;
}

int NIMEASIROCMonitor::daq_pause()
{
    std::cerr << "*** NIMEASIROCMonitor::pause" << std::endl;

    return 0;
}

int NIMEASIROCMonitor::daq_resume()
{
    std::cerr << "*** NIMEASIROCMonitor::resume" << std::endl;

    return 0;
}

int NIMEASIROCMonitor::reset_InPort()
{
    int ret = true;
    while(ret == true) {
        ret = m_InPort.read();
    }

    return 0;
}

unsigned int NIMEASIROCMonitor::decode_data(const unsigned char* mydata)
{  
    //unsigned int netdata    = *(unsigned int*)&mydata[0];
    //m_NIMEASIROCData1.data = ntohl(netdata);
    unsigned char oneeventdata[4]={mydata[0],mydata[1],mydata[2],mydata[3]};
    unsigned int data32 = unpackBigEndian32(oneeventdata);
    unsigned int ret = Decode32bitWord(data32);
    /*
    if(isAdcHg(data32)){
      int ch = (data32 >> 13) & 0x3f;
      bool otr = ((data32 >> 12) & 0x01) != 0;
      int value = data32 & 0x0fff;
      if(!otr) {
        m_NIMEASIROCData1.Adchigh = value;
      }
    }else if(isAdcLg(data32)) {
      int ch = (data32 >> 13) & 0x3f;
      bool otr = ((data32 >> 12) & 0x01) != 0;
      int value = data32 & 0x0fff;
      if(!otr) {
        m_NIMEASIROCData1.Adclow = value;
      }
    }else if(isTdcLeading(data32)) {
      int ch = (data32 >> 13) & 0x3f;
      int value = data32 & 0x0fff;
      m_NIMEASIROCData1.TdcLeading = value;
    }else if(isTdcTrailing(data32)) {
      int ch = (data32 >> 13) & 0x3f;
      int value = data32 & 0x0fff;
      m_NIMEASIROCData1.TdcTrailing = value;
    //scaler info is not used so far
    }else if(isScaler(data32)) {
      int ch = (data32 >> 14) & 0x7f;
      int value = data32 & 0x3fff;
      m_NIMEASIROCData1.Scaler = value;
    }*/

    /*
    m_NIMEASIROCData.magic      = mydata[0];
    m_NIMEASIROCData.format_ver = mydata[1];
    m_NIMEASIROCData.module_num = mydata[2];
    m_NIMEASIROCData.reserved   = mydata[3];
    unsigned int netdata    = *(unsigned int*)&mydata[4];
    m_NIMEASIROCData.data       = ntohl(netdata);

    if (m_debug) {
        std::cerr << "magic: "      << std::hex << (int)m_NIMEASIROCData.magic      << std::endl;
        std::cerr << "format_ver: " << std::hex << (int)m_NIMEASIROCData.format_ver << std::endl;
        std::cerr << "module_num: " << std::hex << (int)m_NIMEASIROCData.module_num << std::endl;
        std::cerr << "reserved: "   << std::hex << (int)m_NIMEASIROCData.reserved   << std::endl;
        std::cerr << "data: "       << std::dec << (int)m_NIMEASIROCData.data       << std::endl;
    }
    */

    return ret;
}

unsigned int NIMEASIROCMonitor::decode_header(const unsigned char* mydata)
{
    unsigned char header[NIMEASIROC::headersize];
    for(int i=0; i<NIMEASIROC::headersize; i++){
        header[i] = mydata[i];
    }
    unsigned int header32 = unpackBigEndian32(header);
    unsigned int ret = Decode32bitWord(header32);

    return ret;
}


//fill data
int NIMEASIROCMonitor::fill_data(const unsigned char* mydata, const int size)
{  
    //H.Asano memo
    //ONE_EVENT_SIZE (=4bytes) ,defined in NIMEASIROCData.h
    //the unit of "size" is byte 
    /*
    for (int i = 0; i < size/(int)ONE_EVENT_SIZE; i++) {
      decode_data(mydata);
      float fdata = m_NIMEASIROCData.data/1000.0; // 1000 times value is received
      m_hist->Fill(fdata);
      mydata+=ONE_EVENT_SIZE;
    }*/

    unsigned int events = 0;
    m_NIMEASIROCData1.header = decode_header(mydata);
    
    if(m_debug){
      std::cout << __FUNCTION__ << " size from header " << 4*(m_NIMEASIROCData1.header & 0x0fff) << std::endl;  
      std::cout << __FUNCTION__ << " size " << size << std::endl;
      std::cout << __FUNCTION__ << " # of word " << size/(int)ONE_EVENT_SIZE << std::endl;
    }

    
    unsigned int size0  = 4*(m_NIMEASIROCData1.header & 0x0fff);
    for (unsigned int i = 0; i < (size0/(int)ONE_EVENT_SIZE); i++ ) {
      unsigned int onedata = decode_data(mydata+NIMEASIROC::headersize);//add offset of header
      
      if(isAdcHg(onedata)){
        int ch = (onedata >> 13) & 0x3f;
        bool otr = ((onedata >> 12) & 0x01) != 0;
        int value = onedata & 0x0fff;
        if(!otr) {
          m_adcHigh[ch]->Fill(value);
        }
      }else if(isAdcLg(onedata)) {
        int ch = (onedata >> 13) & 0x3f;
        bool otr = ((onedata >> 12) & 0x01) != 0;
        int value = onedata & 0x0fff;
        if(!otr) {
          m_adcLow[ch]->Fill(value);
        }
      }else if(isTdcLeading(onedata)) {
        int ch = (onedata >> 13) & 0x3f;
        int value = onedata & 0x0fff;
        m_tdcLeading[ch]->Fill(value);
      }else if(isTdcTrailing(onedata)) {
        int ch = (onedata >> 13) & 0x3f;
        int value = onedata & 0x0fff;
        m_tdcTrailing[ch]->Fill(value);
        //scaler info is not used so far
      }/*else if(isScaler(data32)) {
         int ch = (data32 >> 14) & 0x7f;
         int value = data32 & 0x3fff;
         m_NIMEASIROCData1.Scaler = value;
         }*/
      /*
         m_NIMEASIROCData.magic      = mydata[0];
         m_NIMEASIROCData.format_ver = mydata[1];
         m_NIMEASIROCData.module_num = mydata[2];
         m_NIMEASIROCData.reserved   = mydata[3];
         unsigned int netdata    = *(unsigned int*)&mydata[4];
         m_NIMEASIROCData.data       = ntohl(netdata);

         if (m_debug) {
         std::cerr << "magic: "      << std::hex << (int)m_NIMEASIROCData.magic      << std::endl;
         std::cerr << "format_ver: " << std::hex << (int)m_NIMEASIROCData.format_ver << std::endl;
         std::cerr << "module_num: " << std::hex << (int)m_NIMEASIROCData.module_num << std::endl;
         std::cerr << "reserved: "   << std::hex << (int)m_NIMEASIROCData.reserved   << std::endl;
         std::cerr << "data: "       << std::dec << (int)m_NIMEASIROCData.data       << std::endl;
         }
      */

      events++;
      mydata+=ONE_EVENT_SIZE;
    }
    
    unsigned int header1 = decode_header(mydata+NIMEASIROC::headersize);
    unsigned int size1  = 4*(header1 & 0x0fff);
    for (unsigned int i = 0; i < (size1/(int)ONE_EVENT_SIZE); i++ ) {
      unsigned int onedata = decode_data(mydata+2*NIMEASIROC::headersize);//add offset of header0 + header1
      
      if(isAdcHg(onedata)){
        int ch = ((onedata >> 13) & 0x3f)+64;
        bool otr = ((onedata >> 12) & 0x01) != 0;
        int value = onedata & 0x0fff;
        if(!otr) {
          m_adcHigh[ch]->Fill(value);
        }
      }else if(isAdcLg(onedata)) {
        int ch = ((onedata >> 13) & 0x3f)+64;
        bool otr = ((onedata >> 12) & 0x01) != 0;
        int value = onedata & 0x0fff;
        if(!otr) {
          m_adcLow[ch]->Fill(value);
        }
      }else if(isTdcLeading(onedata)) {
        int ch = ((onedata >> 13) & 0x3f)+64;
        int value = onedata & 0x0fff;
        m_tdcLeading[ch]->Fill(value);
      }else if(isTdcTrailing(onedata)) {
        int ch = ((onedata >> 13) & 0x3f)+64;
        int value = onedata & 0x0fff;
        m_tdcTrailing[ch]->Fill(value);
        //scaler info is not used so far
      }/*else if(isScaler(data32)) {
         int ch = ((data32 >> 14) & 0x7f)+64;
         int value = data32 & 0x3fff;
         m_NIMEASIROCData1.Scaler = value;
         }*/

      events++;
      mydata+=ONE_EVENT_SIZE;
    }


    return 0;
}

unsigned int NIMEASIROCMonitor::read_InPort()
{
    /////////////// read data from InPort Buffer ///////////////
    unsigned int recv_byte_size = 0;
    bool ret = m_InPort.read();

    //////////////////// check read status /////////////////////
    if (ret == false) { // false: TIMEOUT or FATAL
        m_in_status = check_inPort_status(m_InPort);
        if (m_in_status == BUF_TIMEOUT) { // Buffer empty.
            if (check_trans_lock()) {     // Check if stop command has come.
                set_trans_unlock();       // Transit to CONFIGURE state.
            }
        }
        else if (m_in_status == BUF_FATAL) { // Fatal error
            fatal_error_report(INPORT_ERROR);
        }
    }
    else {
        recv_byte_size = m_in_data.data.length();
    }

    if (m_debug) {
        std::cerr << "m_in_data.data.length():" << recv_byte_size
                  << std::endl;
    }

    return recv_byte_size;
}

//run event by event ? unless BestEfforDispacher is running ?
int NIMEASIROCMonitor::daq_run()
{
    if (m_debug) {
        std::cerr << "*** NIMEASIROCMonitor::run" << std::endl;
    }

    m_recv_data.clear();
    unsigned int recv_byte_size = read_InPort();
    if (recv_byte_size == 0) { // Timeout
        return 0;
    }

    //check_header_footer(m_in_data, recv_byte_size); // check header and footer
    m_event_byte_size = get_event_size(recv_byte_size);

    /////////////  Write component main logic here. /////////////
    //H. Asano memo
    //HEADER_BYTE_SIZE is actually 8 bytes, defined in /user/include/DaqComponentBase.h
    //after the header defined by DAQMW, the data sent by readerComp will be started.
    //The data is copied to the m_recv_data
    m_recv_data.resize(m_event_byte_size);
    memcpy(&m_recv_data[0], &m_in_data.data[HEADER_BYTE_SIZE], m_event_byte_size);
   
    //decode and fill histograms
    fill_data(&m_recv_data[0], m_event_byte_size);

    if (m_monitor_update_rate == 0) {
        m_monitor_update_rate = 1000;
    }

    unsigned long sequence_num = get_sequence_num();
    if (m_debug) {
        std::cerr << "*** NIMEASIROCMonitor::sequence # " << sequence_num << std::endl;
    }
  
    if ((sequence_num % m_monitor_update_rate) == 0) {
        //m_hist->Draw();
        m_canvas->cd(1);
        m_adcHigh[0]->Draw();
        gPad->SetLogy();
        m_canvas->cd(2);
        m_adcHigh[64]->Draw();
        gPad->SetLogy();
        m_canvas->Update();
        gPad->SetLogy();
    }
    /////////////////////////////////////////////////////////////
    inc_sequence_num();                      // increase sequence num.
    inc_total_data_size(m_event_byte_size);  // increase total data byte size

    return 0;
}


//convert 4 char arrays to a single 32 bit word
unsigned int NIMEASIROCMonitor::unpackBigEndian32(const unsigned char* array4byte)
{

    return ((array4byte[0] << 24) & 0xff000000) |
           ((array4byte[1] << 16) & 0x00ff0000) |
           ((array4byte[2] <<  8) & 0x0000ff00) |
           ((array4byte[3] <<  0) & 0x000000ff);
}



bool NIMEASIROCMonitor::isAdcHg(unsigned int data)
{
  return (data & 0x00680000) == 0x00000000;
}


bool NIMEASIROCMonitor::isAdcLg(unsigned int data)
{
  return (data & 0x00680000) == 0x00080000;
}


bool NIMEASIROCMonitor::isTdcLeading(unsigned int data)
{
  return (data & 0x00601000) == 0x00201000;
}


bool NIMEASIROCMonitor::isTdcTrailing(unsigned int data)
{
  return (data & 0x00601000) == 0x00200000;
}


bool NIMEASIROCMonitor::isScaler(unsigned int data)
{
  return (data & 0x00600000) == 0x00400000;
}


unsigned int NIMEASIROCMonitor::Decode32bitWord(unsigned int word32bit)
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


void NIMEASIROCMonitor::clearhists()
{
  for(int i = 0; i < 128; i++){
    delete m_adcHigh[i];    
    delete m_adcLow[i];     
    delete m_tdcLeading[i]; 
    delete m_tdcTrailing[i];

    m_adcHigh[i]=NULL;    
    m_adcLow[i]=NULL;     
    m_tdcLeading[i]=NULL; 
    m_tdcTrailing[i]=NULL;
  }
  for(int i = 0;i < 4; i++){
    delete m_2DcorrelationX[i];
    delete m_2DcorrelationU[i];
    delete m_2DcorrelationV[i];
    m_2DcorrelationX[i]=NULL;
    m_2DcorrelationU[i]=NULL; 
    m_2DcorrelationV[i]=NULL;
  }

}



extern "C"
{
    void NIMEASIROCMonitorInit(RTC::Manager* manager)
    {
        RTC::Properties profile(NIMEASIROCmonitor_spec);
        manager->registerFactory(profile,
                    RTC::Create<NIMEASIROCMonitor>,
                    RTC::Delete<NIMEASIROCMonitor>);
    }
};
