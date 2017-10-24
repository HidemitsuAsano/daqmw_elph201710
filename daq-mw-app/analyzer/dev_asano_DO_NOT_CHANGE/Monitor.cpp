// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include <THttpServer.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <sstream>


#include "Monitor.h"


#include "datadrs.hh"
#include "Drs4QdcDecoder.hh"
#include "WaveformDrawer.hh"
#include "Drs4IntegralAna.hh"
#include "Drs4TdcDrawer.hh"
#include "RegisterMap.hh"
#include "common_val.hh"

#include "getaddr.h"
#include "HexDump.hh"

//______________________________________________________________________________
using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::HEADER_DATA_MISMATCH;
using DAQMW::FatalType::FOOTER_DATA_MISMATCH;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

WaveformDrawer wfdraw1,wfdraw2;
Drs4IntegralAna    drs4iana1,drs4iana2;
Drs4TdcDrawer      drs4tdcdraw1,drs4tdcdraw2;


dataDrs* drs4cont1 = nullptr;
dataDrs* drs4cont2 = nullptr;
std::vector<double> peakXList(NofChModule*Nboards);
std::vector<double> baselineList(NofChModule*Nboards);
std::vector<double> amplitudeList(NofChModule*Nboards);
std::vector<double> integralList(NofChModule*Nboards);

//______________________________________________________________________________
void
clear(dataDrs& c)
{
  c.nword_header   = 0;
  c.nword_body     = 0;
  c.global_tag     = 0;
  c.local_tag      = 0;
  c.tic_count      = 0;
  c.fl_double_data = 0;
  
  for (auto& e : c.wsr) e = 0;
  for (auto& e : c.cellnum) e = 0;
  for (auto& e : c.data_wf) e.clear();
  for (auto& e : c.data_qdc) e = 0;
  for (auto& e : c.data_tdc) e.clear();
  for (auto& e : c.data_dt)  e.clear();
  for (auto& e : c.data_tdc_2nd) e.clear();
  for (auto& e : c.data_tdc_2nd) e.clear();//why twice?
  for (auto& e : c.data_width) e.clear();

  c.l1_tdc  = 0;
  c.l1_tdc1 = 0;

}

//______________________________________________________________________________
// Module specification
// Change following items to suit your component's spec.
static const char* monitor_spec[] =
  {
    "implementation_id", "Monitor",
    "type_name",         "Monitor",
    "description",       "Monitor component",
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
Monitor::Monitor(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_InPort("in0",   m_in_data),
    m_in_status(BUF_SUCCESS),
    m_Verbosity(0),
    m_t0_ch(0xffff), 
    m_has_tdc_data(false),
    m_conf_file_name(),
    //m_serverName("http:8888"), 
    m_serverName("http:8888?thrds=8"), 
    m_server(nullptr),
    m_update_cycle(1)
{
  // Registration: InPort/OutPort/Service

  // Set InPort buffers
  registerInPort ("in0",  m_InPort);

  init_command_port();
  init_state_table();
  set_comp_name("MONITOR");

  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetFrameFillColor(0);
  //  gStyle->SetCanvasColor(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetTitleAlign(22);
  gStyle->SetTitleX(0.5);
  gStyle->SetTitleY(0.95);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  Int_t fontid = 132;

  gStyle->SetStatFont(fontid);
  gStyle->SetLabelFont(fontid, "XYZ");
  gStyle->SetLabelFont(fontid, "");
  gStyle->SetTitleFont(fontid, "XYZ");
  gStyle->SetTitleFont(fontid, "");
  gStyle->SetTitleOffset(1.2, "XYZ");
  gStyle->SetTextFont(fontid);
  gStyle->SetFuncWidth(2);
  gStyle->SetLegendBorderSize(0);

  gStyle->SetOptStat("ouirmen");
  gStyle->SetOptFit(true);
  
  gStyle->SetPadGridX(true);
  gStyle->SetPadGridY(true);

  
}

//______________________________________________________________________________
Monitor::~Monitor()
{
  if (drs4cont1) delete drs4cont1; drs4cont1 = nullptr;
  if (drs4cont2) delete drs4cont2; drs4cont2 = nullptr;
}

//______________________________________________________________________________
RTC::ReturnCode_t Monitor::onInitialize()
{
  if (m_Verbosity) {
    std::cerr << "Monitor::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t Monitor::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int Monitor::daq_dummy()
{
  update();
  gSystem->Sleep(10);
  
  return 0;
}

//______________________________________________________________________________
int Monitor::daq_configure()
{
  std::cerr << "*** Monitor::configure" << std::endl;

  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);


  if (!m_server) {
    m_server = new THttpServer(m_serverName.c_str());
  }

  if (!drs4cont1) drs4cont1 = new dataDrs;
  if (!drs4cont2) drs4cont2 = new dataDrs;
  
  //Drs4Qdc 1
  int ncalled = wfdraw1.initialize(m_server);
  if(m_Verbosity){
    std::cout << "wfdraw1 init  " << ncalled << std::endl;
  }

  drs4iana1.initialize(m_server);
  drs4tdcdraw1.initialize(m_server);
  drs4tdcdraw1.setT0Channel(m_t0_ch);
  //Drs4Qdc 2
  ncalled = wfdraw2.initialize(m_server);
  if(m_Verbosity){
    std::cout << "wfdraw2 init " << ncalled << std::endl;
  }
  drs4iana2.initialize(m_server);
  drs4tdcdraw2.initialize(m_server);
  drs4tdcdraw2.setT0Channel(m_t0_ch);
  
  update();
  int nbin = 4096;
  const int ncan =2 ;
  for(int ic=0;ic<ncan;ic++){
    if(m_cadchigh[ic]) continue;
    m_cadchigh[ic] = new TCanvas(Form("cadchigh%d",ic),Form("ADC high gain%d",ic));
    m_cadchigh[ic]->Divide(8,4);
  }
  for(int ic=0;ic<ncan;ic++){
    if(m_ctot[ic]) continue;
    m_ctot[ic] = new TCanvas(Form("ctot%d",ic),Form("TOT%d",ic));
    m_ctot[ic]->Divide(8,4);
  }
  for(int i =0; i<nNERch; i++){
    int ican = -1;
    if(i< 32){
      m_cadchigh[0]->cd(i+1);
    }else if(32<= i && i < 64){
      m_cadchigh[1]->cd(i+1-32);
    }else if(64<= i && i < 96){
      m_cadchigh[2]->cd(i+1-64);
    }else if(96<= i && i < 128){
      m_cadchigh[3]->cd(i+1-96);
    }
  
    gPad->SetLogy();
    m_adcHigh[i] = new TH1I(Form("ADC_HIGH_%d", i),
        Form("ADC high gain %d", i),
        nbin, 0, 4096);
    char sname[256];
    m_adcHigh[i]->Draw();
    sprintf(sname,"/nimeasiroc/adc",i);
    m_server->Register(sname, m_adcHigh[i]) ;

    gPad->SetLogy();
    m_TOT[i] = new TH1I(Form("TOT_%d", i),
        Form("TOT %d", i),
        nbin, 0, 4096);
     m_TOT[i]->Draw();
    sprintf(sname,"/nimeasiroc/tdc",i);
    m_server->Register(sname, m_TOT[i]) ;
  }

  return 0;
}

//______________________________________________________________________________
int Monitor::parse_params(::NVList* list)
{

  std::cerr << "param list length:" << (*list).length() << std::endl;

  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;

    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;

    if (sname=="server") {
      m_serverName = svalue;
    }
    if (sname=="update_cycle") {
      m_update_cycle = std::stoul(svalue);
    }
    if (sname=="hasTdc" || sname=="hasTDC") {
      if (svalue=="true" || svalue=="TRUE" || svalue=="yes" || svalue=="YES") {
        m_has_tdc_data = true;
      }
    }
    if (sname=="t0" || sname=="T0") {
      m_t0_ch = std::stoul(svalue);
    }
    if (sname=="confFile") {
      m_conf_file_name = svalue;
    }
  }


  return 0;
}

//______________________________________________________________________________
int Monitor::daq_unconfigure()
{
  std::cerr << "*** Monitor::unconfigure" << std::endl;

  update();
  clearhists();
  
  return 0;
}

//______________________________________________________________________________
int Monitor::daq_start()
{
  std::cerr << "*** Monitor::start" << std::endl;

  m_in_status  = BUF_SUCCESS;
  wfdraw1.reset();
  drs4tdcdraw1.reset();
  drs4iana1.reset();
  wfdraw2.reset();
  drs4tdcdraw2.reset();
  drs4iana2.reset();
  
  update();

  return 0;
}

//______________________________________________________________________________
int Monitor::daq_stop()
{
  std::cerr << "*** Monitor::stop" << std::endl;
  update();
    
  reset_InPort();


  return 0;
}

//______________________________________________________________________________
int Monitor::daq_pause()
{
  std::cerr << "*** Monitor::pause" << std::endl;
  update();
    
  return 0;
}

//______________________________________________________________________________
int Monitor::daq_resume()
{
  std::cerr << "*** Monitor::resume" << std::endl;
  update();
    
  return 0;
}

//______________________________________________________________________________
int Monitor::reset_InPort()
{
  int ret = true;
  while(ret == true) {
    ret = m_InPort.read();
  }

  return 0;
}

//______________________________________________________________________________
unsigned int Monitor::read_InPort()
{
  if (check_trans_lock()) set_trans_unlock();

    
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

  if (m_Verbosity) {
    std::cerr << "m_in_data.data.length():" << recv_byte_size
              << std::endl;
  }

  return recv_byte_size;
}

//______________________________________________________________________________
int Monitor::daq_run()
{
  if (m_Verbosity) {
    std::cerr << "*** Monitor::run" << std::endl;
  }

  unsigned int recv_byte_size = read_InPort();
  if (recv_byte_size == 0) { // Timeout
    return 0;
  }

  // temporarily comment out to avoid footer mismatch
  //  check_header_footer(m_in_data, recv_byte_size); // check header and footer
   
  //get data size after subtracting header and footer
  unsigned int event_byte_size = get_event_size(recv_byte_size);
  if(m_Verbosity){
    std::cerr << event_byte_size << std::endl;
  }
 
  /////////////  Write component main logic here. /////////////
  //H. Asano memo
  //HEADER_BYTE_SIZE is 8 bytes, defined in /user/include/DaqComponentBase.h
  //after the header defined by DAQMW, the data sent by readerComp will be started.
  //data structure
  //1. DAQ-MW header (8 bytes, fixed length) 
  //2. NIMEASIROC1 (variable length)
  //3. NIMEASIROC2 (variable length)
  //4. Drs4QDC1 (variable length)
  //5. Drs4QDC2 (variable length)
  //6. Hul-Scaler (12 (header) + 256 (body) = 268 bytes, fixed length) 
  //7. DAQ-MW footer (8 bytes,fixed length)
  //
  
  std::vector <unsigned char> recv_data;
  recv_data.resize(event_byte_size);
  
  //cut off DAQ-MW's header
  //Note "event_byte_size" does not include DAQ-MW's header and footer size
  memcpy(&recv_data[0], &m_in_data.data[HEADER_BYTE_SIZE], event_byte_size);
  const unsigned char* NERdata = &recv_data[0];
  struct FiberHit sfiber[nNERch];
  if(m_Verbosity>3){
    //std::cerr << "all data:  " << get_sequence_num() << std::endl;
    //std::for_each(((unsigned int)recv_data.begin(), (unsigned int)recv_data.end(), hddaq::HexDump());
  }
  
  //number of Nim-EasiRoc (NER)
  const int nNERmodule = 2;
  size_t totalNERdata = 2*4;//2 headers * 4 bytes
  for (int iNER =0;iNER<nNERmodule;iNER++){
    unsigned int NERheader = NERdecode_header(NERdata);
    unsigned int datalen  = 4*(NERheader & 0x0fff);
    totalNERdata += datalen;
    NERdata +=NIMEASIROC::headersize;
    for (unsigned int i = 0; i < (datalen/4); i++ ) {//1data = 4 byte
      //if(m_Verbosity){
      //  std::cout << "L." << __LINE__ << "mod. " << iNER << " ich " << i << std::endl;
      //}
      unsigned int onedata = NERdecode_data(NERdata);

      int ch = (onedata >> 13) & 0x3f;
      if(iNER%2==1) ch+=64;
      //if(m_Verbosity){
      //  std::cout << "L." << __LINE__ << "mod. " << iNER << " ich " << ch << " idata " << i << std::endl;
      //}
      if(NERisAdcHg(onedata)){
        bool otr = ((onedata >> 12) & 0x01) != 0;
        int value = onedata & 0x0fff;
        if(!otr) {
          m_adcHigh[ch]->Fill(value);
        }
      }else if(NERisAdcLg(onedata)) {
        bool otr = ((onedata >> 12) & 0x01) != 0;
        int value = onedata & 0x0fff;
        if(!otr) {
          //m_adcLow[ch]->Fill(value);
        }
      }else if(NERisTdcLeading(onedata)) {
        int value = onedata & 0x0fff;
        sfiber[ch].tdcleading = value;
        //m_tdcLeading[ch]->Fill(value);
      }else if(NERisTdcTrailing(onedata)) {
        int value = onedata & 0x0fff;
        sfiber[ch].tdctrailing = value;
        //m_tdcTrailing[ch]->Fill(value);
        //scaler info is not used so far
      }/*else if(isScaler(data32)) {
         int ch = (data32 >> 14) & 0x7f;
         int value = data32 & 0x3fff;
         m_NIMEASIROCData1.Scaler = value;
         }*/
      NERdata+=4;//1 data = 4byte
    }//
  }
  
  for(int ich=0;ich<nNERch;ich++){
    int tot = sfiber[ich].tdcleading - sfiber[ich].tdctrailing;
    m_TOT[ich]->Fill(tot);
  }
  
  //remaining Data : DRS4Qdc1,2 HULScaler, DAQMW's footer, NER's data is subtracted here
  //Note "event_byte_size" does not include DAQ-MW's header and footer size
  std::vector<unsigned int> recv_data2((event_byte_size-totalNERdata)/sizeof(unsigned int));
  memcpy(reinterpret_cast<unsigned char*>(&recv_data2[0]),
         &m_in_data.data[HEADER_BYTE_SIZE+totalNERdata],
         (event_byte_size-totalNERdata) );
  
  if(m_Verbosity>1){
    std::cerr << "DRS4 1,2 and HulScaler:  " << get_sequence_num() << std::endl;
    std::for_each(recv_data2.begin(), recv_data2.end(), hddaq::HexDump());
  }
  
  Drs4QdcDecoder Drs4Qdcdecode;

  clear(*drs4cont1);
  clear(*drs4cont2);
  for (auto& e : peakXList)     e = 0;
  for (auto& e : amplitudeList) e = 0;
  for (auto& e : integralList)  e = 0;
  for (auto& e : baselineList)  e = 0;
  if(m_Verbosity){
    std::cerr << "start drs4 1 " << std::endl;
  }

  //std::vector<unsigned int>::const_iterator drs4itr = recv_data2.begin();
  auto drs4dataitr = recv_data2.begin();
  unsigned int nword_drs4_1 = Drs4Qdcdecode(drs4dataitr, *drs4cont1, m_has_tdc_data);
  if(m_Verbosity){
    std::cerr << "nword drs4_1 " << nword_drs4_1 << std::endl;
  }
  drs4tdcdraw1.fill(*drs4cont1);
  drs4iana1.fill(*drs4cont1);

  if(m_Verbosity){
    std::cerr << "wfa drs4 1 " << std::endl;
  }
  if ((get_sequence_num() % m_update_cycle) == 0) {
    std::vector<double> peakX(NofChModule);
    std::vector<double> baseline(NofChModule);
    for (int i=0; i<NofChModule; ++i) {
      peakXList[i]     = drs4iana1.getPeakX(i);
      baselineList[i]  = drs4iana1.getBaseLine(i);
      amplitudeList[i] = drs4iana1.getAmplitude(i);
      integralList[i]  = drs4iana1.getIntegral(i);
    }
    wfdraw1.fill(*drs4cont1, peakXList, baselineList);
    update();
  }
  if(m_Verbosity){
    std::cerr <<  "go to DRS4_2 " << std::endl;
  }
  /////////////////////////////////////////////////////////////
  //move iterator
  
  drs4dataitr += nword_drs4_1; 
  
  unsigned int nword_drs4_2 = Drs4Qdcdecode(drs4dataitr, *drs4cont2, m_has_tdc_data);
  if(m_Verbosity){
    std::cerr <<  "nword_drs4_2 " << nword_drs4_2 << std::endl;
    std::cerr <<  "go to DRS4_2 tdc" << std::endl;
  }
  
  
  drs4tdcdraw2.fill(*drs4cont2);
  if(m_Verbosity){
    std::cerr <<  "go to DRS4_2 iana" << std::endl;
  }
  drs4iana2.fill(*drs4cont2);

  if ((get_sequence_num() % m_update_cycle) == 0) {
    if(m_Verbosity){
      std::cerr <<  "go to DRS4_2 wfdraw" << std::endl;
    }
    std::vector<double> peakX(NofChModule);
    std::vector<double> baseline(NofChModule);
    for (int i=0; i<NofChModule; ++i) {
      peakXList[i]     = drs4iana2.getPeakX(i);
      baselineList[i]  = drs4iana2.getBaseLine(i);
      amplitudeList[i] = drs4iana2.getAmplitude(i);
      integralList[i]  = drs4iana2.getIntegral(i);
    }
    wfdraw2.fill(*drs4cont2, peakXList, baselineList);
    update();
  }
  

  if(m_Verbosity>10){
     std::cerr << "event byte size " << event_byte_size << std::endl;
     std::cerr << "NER data size   " << totalNERdata << std::endl;
     std::cerr << "DRS4 1 nword " << nword_drs4_1 << std::endl;
     std::cerr << "DRS4 1 data size " << nword_drs4_1*4 << std::endl;
     std::cerr << "DRS4 2 nword " << nword_drs4_2 << std::endl;
     std::cerr << "DRS4 2 data size (cal) " << event_byte_size - totalNERdata - nword_drs4_1*4 - 268 << std::endl;
     std::cerr << "HUL              " << 268 << std::endl;
     size_t size  = recv_data2.size();
     for(unsigned int i = 0; i< 67; ++i){
       
       std::cout << std::hex << std::setw(8) <<
       recv_data2.at(i+size-67)
       << " " ;
       if((i+1)%8 == 0) std::cout << std::endl;
     }
  }

  size_t size  = recv_data2.size();
  
  int seqnum = get_sequence_num();
  if (seqnum%100==0){
    for(unsigned int i = 0; i< 67; ++i){
      std::cout << std::hex << std::setw(8) <<
        recv_data2.at(i+size-67)  << " " ;
      if((i+1)%8 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
  }
  
  //HUL scaler
  static const unsigned int magic    = 0xffff4ca1;
  static const unsigned int magic_head     = 0xf;
  static const unsigned int magic_rvm      = 0xf9;
  static const unsigned int magic_block1   = 0x8;
  static const unsigned int magic_block2   = 0x9;
  static const unsigned int magic_block3   = 0xA;
  static const unsigned int magic_block4   = 0xB;

  static const int shift_data_head = 28;
  static const int mask_data_head  = 0xf;

  static const int mask_nword= 0x7ff;

  static const int mask_scaler = 0xfffffff;
  
  //if(!m_scaler) m_cscaler = new TCanvas("Scaler","Scaler");
  //m_scaler->cd();
  //gr = new TGraphErrors



  inc_sequence_num();                       // increase sequence num.
  inc_total_data_size(event_byte_size);     // increase total data byte size

  return 0;
}

//______________________________________________________________________________
void
Monitor::parse_file(const std::string& fileName)
{
  std::cout << "#D parse() file = " << fileName << std::endl;
  if (fileName.empty()) return;
  
  std::ifstream ifile(fileName.c_str());
  if (ifile.fail()) {
    std::cout << " error file open " << fileName << std::endl;
    return;
  }
  
  while (ifile) {
    std::string line;
    std::getline(ifile, line);
    if (line.empty()) continue;
    if (line.find("#")!=std::string::npos) continue;
    std::istringstream iss(line);
    Int_t ch;
    std::string name;
    std::string title;
    Int_t start;
    Int_t end;
    Int_t threshold;
    iss >> ch >> name >> title >> start >> end >> threshold;

    std::cout << ch << " "
              << start << " "
              << end << " "
              << threshold
              << std::endl;
  }
}


//______________________________________________________________________________
void
Monitor::update()
{
  gSystem->ProcessEvents();
}


unsigned int Monitor::NERdecode_header(const unsigned char* mydata)
{
    unsigned char header[NIMEASIROC::headersize];
    for(int i=0; i<NIMEASIROC::headersize; i++){
        header[i] = mydata[i];
    }
    unsigned int header32 = NERunpackBigEndian32(header);
    unsigned int ret = NERDecode32bitWord(header32);

    return ret;
}

unsigned int Monitor::NERdecode_data(const unsigned char* mydata)
{  
    //unsigned int netdata    = *(unsigned int*)&mydata[0];
    //m_NIMEASIROCData1.data = ntohl(netdata);
    unsigned char oneeventdata[4]={mydata[0],mydata[1],mydata[2],mydata[3]};
    unsigned int data32 = NERunpackBigEndian32(oneeventdata);
    unsigned int ret = NERDecode32bitWord(data32);

    return ret;
}



//convert 4 char arrays to a single 32 bit word
unsigned int Monitor::NERunpackBigEndian32(const unsigned char* array4byte)
{

    return ((array4byte[0] << 24) & 0xff000000) |
           ((array4byte[1] << 16) & 0x00ff0000) |
           ((array4byte[2] <<  8) & 0x0000ff00) |
           ((array4byte[3] <<  0) & 0x000000ff);
}



bool Monitor::NERisAdcHg(unsigned int data)
{
  return (data & 0x00680000) == 0x00000000;
}


bool Monitor::NERisAdcLg(unsigned int data)
{
  return (data & 0x00680000) == 0x00080000;
}


bool Monitor::NERisTdcLeading(unsigned int data)
{
  return (data & 0x00601000) == 0x00201000;
}


bool Monitor::NERisTdcTrailing(unsigned int data)
{
  return (data & 0x00601000) == 0x00200000;
}


bool Monitor::NERisScaler(unsigned int data)
{
  return (data & 0x00600000) == 0x00400000;
}


unsigned int Monitor::NERDecode32bitWord(unsigned int word32bit)
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

void Monitor::clearhists()
{
  for(int i = 0; i < nNERch; i++){
    delete m_adcHigh[i];    
    //delete m_adcLow[i];     
    delete m_TOT[i]; 

    m_adcHigh[i]=NULL;    
    //m_adcLow[i]=NULL;     
    m_TOT[i]=NULL; 
  }
}



//______________________________________________________________________________
extern "C"
{
  void MonitorInit(RTC::Manager* manager)
  {
    RTC::Properties profile(monitor_spec);
    manager->registerFactory(profile,
                             RTC::Create<Monitor>,
                             RTC::Delete<Monitor>);
  }
};


