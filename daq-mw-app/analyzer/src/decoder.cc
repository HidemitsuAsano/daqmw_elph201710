#include<ios>
#include<iostream>
#include<cstdlib>
#include<cmath>
#include<algorithm>
#include<functional>
#include <iomanip>

#include"decoder.hh"
#include"datadrs.hh"
#include"HexDump.hh"

#include"TFile.h"
#include"TTree.h"
#include"TString.h"

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>

#define BLCOR 0
#define VER2  0

static const std::string MyName = "DrsDecoder";

const double kTimeUnit      = 1.0/960e6/1e-9; // nsec
const double kAmplitudeUnit = 0.445; // mV
const double kChargeUnit    = kTimeUnit*kAmplitudeUnit/50; // pC
const double kTDCTimeUnit   = 1.0;

//Peak serch
const int fIStart = 200;
const int fIEnd   = 450;

//Base line
const int BaseParam = 50;

//Integral
const int RangeLow  = 10;
const int RangeHigh = 10;

const char XUV[3][2] = {"X","U","V"};
const int sigmathre[4] = {20,20,20,20};//adc threshold for each easiroc (0-3), unit is 1 sigma of pedestal

struct FiberHit{
  int layer;
  int fiber;
  int easiroc;
  int type;
  double pos;
  int adchigh;
  bool otradchigh;
  int adclow;
  bool otradclow;
  int tdcleading;
  int tdctrailing;

  FiberHit(){
    layer = -1;
    fiber = -1;
    easiroc = -1;
    type = -1;
    pos = -99.;
    adchigh = -1;
    otradchigh = false;
    adclow = -1;
    otradclow = false;
    tdcleading = -1; 
    tdctrailing = -1;
  }
  void clear(){
    layer = -1;
    fiber = -1;
    easiroc = -1;
    type = -1;
    pos = -99.;
    adchigh = -1;
    otradchigh = false;
    adclow = -1;
    otradclow = false;
    tdcleading = -1; 
    tdctrailing = -1;
  }

};

//______________________________________________________________________________
unsigned int
decodeTdcWord(unsigned int v,
              unsigned int& ct,
              unsigned int& ft)
{
  std::cout << "decodeTdcWord" << std::endl;
  unsigned int data_type = (v >> DrsDecoder::ktdc_data_type_shift) & DrsDecoder::ktdc_data_type_mask;
  ct = (v >> DrsDecoder::ktdc_coarse_time_shift) & DrsDecoder::ktdc_coarse_time_mask;
  ft = (v >> DrsDecoder::ktdc_fine_time_shift) & DrsDecoder::ktdc_fine_time_mask;
  return data_type;
}

//______________________________________________________________________________
unsigned int
decodeTdcWord(unsigned int v,
              unsigned int& ch,
              unsigned int& ct,
              unsigned int& ft)
{
  ch = (v >> DrsDecoder::ktdc_ch_id_shift) & DrsDecoder::ktdc_ch_id_mask;
  return decodeTdcWord(v, ct, ft);
}

//______________________________________________________________________________
unsigned int
getTDC(unsigned int traw)
{
  unsigned int ret = traw & 0x00ffffff;
  return ret;

  // following calculations are performed in FPGA
  unsigned int c1 = (ret>>11);
  c1 = c1 & 0x1fff;
  unsigned int c2 = (ret>>10);
  c2 = c2 & 0x1;
  c2 = 2 -c2;
  unsigned int ft = (ret & 0x3ff);
  ret = (c1<<11) - (c2<<10) - ft;
  return ret & 0x00ffffff;

}

//______________________________________________________________________________
unsigned int
calc_dt(unsigned int traw0,
        unsigned int traw1)
{
  unsigned int t0 = getTDC(traw0);
  unsigned int t1 = getTDC(traw1);

  return (t0 - t1) & 0x00ffffff;
}


// Open ----------------------------------------------------------------------
bool
DrsDecoder::Open(const char* name)
{
  static const std::string MyFunc = "::Open ";

  {// local scope
    std::ifstream ifs_org(name, std::ios::binary);

    if(!ifs_org.is_open()){
      std::cerr << MyName << MyFunc
                << "File open error " << name << std::endl;
      return false;
    }
  }

  // add gzip compressor to filter chain
  //m_ifs.push(boost::iostreams::gzip_decompressor());
  m_ifs.push(boost::iostreams::file_source(name, std::ios::binary));
  m_ifs.auto_close();
  
  // Initialize process
  flag.reset();
  flag.set(Good);
  flag.set(FirstData);
  
  return true;
}

// getNextEvent --------------------------------------------------------------
bool
DrsDecoder::getNextEvent()
{
  static const std::string MyFunc = "::getNextEvent ";
  
  // EOF ?
  if(m_ifs.eof()){
    std::cout << "#D : " << MyName << MyFunc
	      << "End Of File" << std::endl;
    return false;
  }
  if(NreadDRS%2==0){
    std::cout << "start DAQMW HEADER" << std::endl;
    const unsigned int DAQMW_HEADERSIZE = 8 ; //bytes
    const unsigned int DAQMW_FOOTERSIZE = 8 ;
    char buf[DAQMW_HEADERSIZE];
    m_ifs.read(buf,DAQMW_HEADERSIZE);
    //GO TO NIMEASIROC 
    std::cout << "start NIM-EASIROC" << std::endl;
    std::cout << "NDRS " << NreadDRS  << std::endl;

    struct FiberHit sfiber[128];
    for(int imod = 0 ;imod<2;imod++){
      char headerByte[4];
      m_ifs.read(headerByte, 4);
      //for(int i =0 ;i<4;i++){
      //  std::cout  << (int)headerByte[i] << std::endl;
      //}

      unsigned int header32 = getBigEndian32(headerByte);
      unsigned int header = Decode32bitWord(header32);
      bool isHeader = ((header >> 27) & 0x01) == 0x01;
      if(!isHeader) {
        std::cerr << imod << std::endl;
        std::cerr << "Frame Error of header data" << std::endl;
        fprintf(stderr, "    %08X\n", header);
        //for(int i =0 ;i<8;i++){
        //  std::cout  << buf[i] << std::endl;
        //}
        std::exit(1);
      }
      size_t dataSize = header & 0x0fff;
      //std::cout << "datasize " << dataSize << std::endl;
      unsigned int scalerValues[69];
      char* dataBytes = new char[dataSize * 4];
      m_ifs.read(dataBytes, dataSize * 4);  
      for(size_t i = 0; i < dataSize; ++i) {
        unsigned int data32 = getBigEndian32(dataBytes + 4 * i);
        unsigned int data = Decode32bitWord(data32); 
        if(!data){
          std::cout << "Invalid data event " <<  std::endl;
          std::cout << "data size " << dataSize << std::endl;
          break;
        }
        //int ch_test = (data >> 13) & 0x3f;
        //cout << "event " << events << " ch " << ch_test << endl;
        int ch = (data >> 13) & 0x3f;
        if(imod%2==1) ch += 64; 
        int type = getXUV(ch);
        int layer = getlayer(ch);
        int fiber = getfiber(ch); 
        int easiroc = geteasiroc(ch);
        sfiber[ch].type = type;
        sfiber[ch].layer = layer;
        sfiber[ch].fiber = fiber;
        sfiber[ch].easiroc = easiroc;

        if(isAdcHg(data)) {
          bool otr = ((data >> 12) & 0x01) != 0;
          int value = data & 0x0fff;
          float pos = fiber/2.0 - 6.0;
          sfiber[ch].otradchigh = otr;
          sfiber[ch].adchigh = value;
          sfiber[ch].pos = pos;
          if(!otr) {
            //hit ?
            //if(value > thre[ch]){
            //cout << "HIT layer " << layer << "type" << type << " ch " << ch << " fiber "<< fiber << endl;
            //hitprofile[layer][type]->Fill(pos);
            //hitprofile_ch[layer][type]->Fill(fiber);
            // if(layer==0){
            //   hitpos[1][0][type][multi[layer][type]]=pos;//pat. 1 L0-L1
            //   hitpos[2][0][type][multi[layer][type]]=pos;//pat. 2 L0-L2
            //   hitpos[3][0][type][multi[layer][type]]=pos;//pat. 3 L0-L3
            //   fiberarr[1][0][type][multi[layer][type]]=fiber;//pat. 1 L0-L1
            //   fiberarr[2][0][type][multi[layer][type]]=fiber;//pat. 2 L0-L2
            //  fiberarr[3][0][type][multi[layer][type]]=fiber;//pat. 3 L0-L3
            // }else{
            //   hitpos[layer][1][type][multi[layer][type]]=pos;
            //   fiberarr[layer][1][type][multi[layer][type]]=fiber;
            // }
            // multi[layer][type]++;
            // if(multi[layer][type] > maxbuf) cout << "too many events" << endl;
            // }
            //adcHigh[ch]->Fill(value);
          }
        }else if(isAdcLg(data)) {
          bool otr = ((data >> 12) & 0x01) != 0;
          int value = data & 0x0fff;
          sfiber[ch].otradclow = otr;
          sfiber[ch].adclow = value;
          if(!otr) {
            //adcLow[ch]->Fill(value);
          }
        }else if(isTdcLeading(data)) {
          int value = data & 0x0fff;
          sfiber[ch].tdcleading = value;
          //tdcLeading[ch]->Fill(value);
        }else if(isTdcTrailing(data)) {
          int value = data & 0x0fff;
          sfiber[ch].tdctrailing = value;
          //tdcTrailing[ch]->Fill(value);
        }else if(isScaler(data)) {
          int value = data & 0x3fff;
          scalerValues[ch] = value;
          //cout << "event:"<<events<<"/scalerValues["<<ch<<"]:"<<scalerValues[ch] << endl; 
          if(ch == 68) {
            //int scalerValuesArrayIndex = events % 100;
            //memcpy(scalerValuesArray[scalerValuesArrayIndex], scalerValues,
            //    sizeof(scalerValues));
          }
        }else {
          int ch = (data >> 13) & 0x3f;
          int value = data & 0x0fff;
          std::cout << "adchg:"  << (data & 0x00680000);
          std::cout << "adclg:"  << (data & 0x00680000);
          std::cout << "tdcl:"   << (data & 0x00601000);
          std::cout << "tdct:"   << (data & 0x00601000);
          std::cout << "scaler:" << (data & 0x00600000);
          std::cout << "data:" << data << std::endl; 
          std::cout << "ch:" << ch << " value:" << value << std::endl;
          std::cerr << "Unknown data type" << std::endl;
        }

        //store ADC vs ToT correlation
        // if((sfiber[ch].tdctrailing != -1) && (sfiber[ch].tdcleading !=-1)){
        double tot =  sfiber[ch].tdcleading - sfiber[ch].tdctrailing;
        //tdcToT[ch]->Fill(tot);
        //adcToTcorr[ch]->Fill(sfiber[ch].adchigh,tot);
        // }
      }//end of loop of dataSize 
      delete[] dataBytes;
    }
  }//if first DRS4
  
  // read component header
  bufType compHeader;
  read(compHeader, 2);
  // std::cout << std::hex << compHeader[0] << " " << compHeader[1] << std::dec << std::endl;

  // read base header
  if (compHeader[0]!=0x504d4f43) {
    head_buffer_.resize(2);
    head_buffer_[0] = compHeader[0];
    head_buffer_[1] = compHeader[1];
    compHeader.resize(sizeBaseHeader-2);
    read(compHeader, compHeader.size());
    std::copy(compHeader.begin(), compHeader.end(), std::back_inserter(head_buffer_));
  } else {
    read(head_buffer_, (int)sizeBaseHeader);
  }

  if(flag[EoF]) return false;

  int n_word_body   = head_buffer_[i_datasize] & data_size_mask;
  if(flag[FirstData]){
    int n_word_header = (head_buffer_[i_datasize] >> header_size_shift) & header_size_mask;
    if(n_word_header == sizeBaseHeader + sizeExHeader){
      flag.set(ExHead);
    }// if(n_work)

    flag.reset(FirstData);
  }//if(flag)

  // read drs header
  if(flag[ExHead]) read(exhead_buffer_, (int)sizeExHeader);

  // read body
  read(body_buffer_, n_word_body);

  if (has_tdc_data)
  { // TDC data read header
    tdc_buffer_.resize(sizeTdcHeader, 0);
    read(tdc_buffer_, static_cast<int>(tdc_buffer_.size()));
    //std::cout << "#D TDC header " << std::endl;
    //std::for_each(tdc_buffer_.begin(), tdc_buffer_.end(), hddaq::HexDump());


    for (;;) {
      //unsigned int d32 = tdc_header.back();
      unsigned int d32 = tdc_buffer_[i_frame_size];
      unsigned int data_type = (d32 >> ktdc_data_type_shift) & ktdc_data_type_mask;
      int seq_id       = (d32 >> ktdc_num_frame_shift) & ktdc_num_frame_mask;
      int tdc_nword    = (d32 >> ktdc_frame_size_shift) & ktdc_frame_size_mask;
      // std::cout << "#D TDC num frae = " << seq_id
      //           << ", n word = " << tdc_nword
      //           << " data type = " << std::hex << data_type << std::dec
      //           << std::endl;

      bool isLastFrame =  (data_type == ktdc_type_last_frame_size);
      //        std::cout << "#D isLastFrame = " << isLastFrame << std::endl;

      if (seq_id==0) tdc_nword -= sizeTdcHeader;
      if (!isLastFrame) ++tdc_nword;

      //std::cout << "#D n word (mod) = " << tdc_nword << std::endl;

      bufType tdcBufTmp(tdc_nword);

      read(tdcBufTmp, tdc_nword);
      //        std::for_each(tdcBufTmp.begin(), tdcBufTmp.end(), hddaq::HexDump());
      tdc_buffer_.insert(tdc_buffer_.end(), tdcBufTmp.begin(), tdcBufTmp.end());
      //std::for_each(tdc_buffer_.begin(), tdc_buffer_.end(), hddaq::HexDump());
      if (isLastFrame) break;
    }
  }
  NreadDRS++;

  if((NreadDRS-1)%2 == 1){
    //read HUL and DAQMW footer
    //std::cout << "start HUL " << std::endl;
    for(unsigned int i = 0; i< 67; ++i){
      int Huldata;
      m_ifs.read((char*)&Huldata, 4);
      std::cout << std::hex << std::setw(8) << Huldata << 
         " " ;
      if((i+1)%8 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
    char footer[8];
    m_ifs.read(footer,8);
    //for(int i=0;i<8;i++){
    //  std::cout << (int) footer[i] << std::endl;
    //}
  }

  return true;
}

// decode --------------------------------------------------------------------
bool
DrsDecoder::decode(dataDrs& cont)
{
  static const std::string MyFunc = "::decode ";

  int is2nd = (NreadDRS-1)%2;
  if(!is2nd){
    for(int i = 0; i<NofChModule*NofChModule; ++i){
      //int is2nd = (NreadDRS-1)%2;
      //int ich = i + is2nd*NofChModule;
      cont.data_wf[i].clear();
    }
  }

  // decode base header
  cont.nword_header = (head_buffer_[i_datasize] >> header_size_shift) & header_size_mask;
  cont.nword_body     =  head_buffer_[i_datasize] & data_size_mask;
  cont.global_tag     = (head_buffer_[i_tag] >> gtag_shift) & gtag_mask;
  cont.local_tag      =  head_buffer_[i_tag] & ltag_mask;
  cont.tic_count      =  head_buffer_[i_tic] & tic_mask;
  cont.fl_double_data = (head_buffer_[i_tic] >> datastr_shift ) & datastr_mask;

  std::cout << "Line " << __LINE__ << std::endl;
  std::cout << "#D " << std::hex << cont.nword_header
            << " " << cont.nword_body
            << " " << cont.local_tag
            << std::endl;

#if BLCOR
  bool fl_overflow = false;
  if(cont.tic_overflow) fl_overflow = true;
  setThisEvent(cont.tic_count, cont.tic_overflow);
#endif

  // decode ex header
  if(flag[ExHead]){
    for(unsigned int i = 0; i<sizeExHeader; ++i){
      //int is2nd = (NreadDRS-1)%2;
      //int ich = i + is2nd*NofChModule;
      cont.wsr[i]     = (exhead_buffer_[i] >> wsr_shift) & wsr_mask;
      cont.cellnum[i] =  exhead_buffer_[i] & cellnum_mask;
    }// for(i)
  }// if(flag)

#if BLCOR
  int current_cellnum[NofChModule] = {cont.cellnum[0], cont.cellnum[0],
				      cont.cellnum[0], cont.cellnum[0],
				      cont.cellnum[1], cont.cellnum[1],
				      cont.cellnum[1], cont.cellnum[1],
				      cont.cellnum[2], cont.cellnum[2],
				      cont.cellnum[2], cont.cellnum[2],
				      cont.cellnum[3], cont.cellnum[3],
				      cont.cellnum[3], cont.cellnum[3]};
#endif

  // decode body
  std::cout <<  std::dec <<  __LINE__ << std::endl;
  for(int i = 0; i<cont.nword_body; ++i){
    unsigned int data_type = (body_buffer_[i] >> type_shift) & type_mask;
    unsigned int ch        = (body_buffer_[i] >> ch_shift) & ch_mask;
    int is2nd = (NreadDRS-1)%2;
    if(is2nd) ch += NofChModule;

    if(data_type == type_wf){
      if(cont.fl_double_data){
	double data_high = (double)((body_buffer_[i] >> wf_high_shift) & wf_mask_12bit);
	double data_low  = (double)(body_buffer_[i] & wf_mask_12bit);

	cont.data_wf[ch].push_back(data_low);
	cont.data_wf[ch].push_back(data_high);
      }else{
	unsigned int data  = (body_buffer_[i] & wf_mask_14bit);
	unsigned int extra_bits  = data & 0x3; // extra 2bits
	double wf_data = (double)(data >> 2) + extra_bits*0.25;
	cont.data_wf[ch].push_back(wf_data);
	//	if((ch == 0 || ch == 15 ) && cont.data_wf[ch].size()>511) std::cout << "ch:" << ch << " " << cont.data_wf[ch].size() << std::endl;
      }
	
#if BLCOR
      int drs4_id = (int)ch/4;
      data_low  -= getCorBL(drs4_id, current_cellnum[ch]++);
      if(current_cellnum[ch] == 1024) current_cellnum[ch] = 0;
      data_high -= getCorBL(drs4_id, current_cellnum[ch]++);
      if(current_cellnum[ch] == 1024) current_cellnum[ch] = 0;
#endif

    }else if(data_type == type_qdc){
      unsigned int data_qdc = body_buffer_[i] & qdc_mask;
      cont.data_qdc[ch] = data_qdc;
    }else{
      std::cout << "#E : " << MyName << MyFunc
		<< "Unknown data type " << std::hex << data_type 
		<< std::dec << std::endl;
      std::cout << "D ith = " << i
                << " nword header = " << cont.nword_header 
                << "\n nword body = " << cont.nword_body
                << "\n global tag = " << cont.global_tag
                << "\n local tag = "  << cont.local_tag
                << "\n tic count = " << cont.tic_count
                << "\n fl_double = " << cont.fl_double_data
                << std::endl;
      std::cout << "i " << i << std::endl;
      std::cout << "NDRS " << NreadDRS << std::endl;
      std::for_each(body_buffer_.begin(),   body_buffer_.end(),   hddaq::HexDump());
      return false;
    }// if(data_type)
  }// for(i)

#if BLCOR
  doneThisEvent();
#endif

  return true;
}

//______________________________________________________________________________
bool
DrsDecoder::decodeTDC(dataDrs& cont)
{
  if (!has_tdc_data) return true;

  std::cout << "#D decodeTDC" << std::endl;

  int is2nd = (NreadDRS-1)%2;
  if(!is2nd){
    for (unsigned int i=0; i<cont.data_tdc.size(); ++i) {

      //int is2nd = (NreadDRS-1)%2;
      //int ich = i + is2nd*NofChModule;
      // cont.data_ct[i].clear();
      // cont.data_ft[i].clear();
      cont.data_tdc[i].clear();
      cont.data_dt[i].clear();

      // cont.data_ct_2nd[i].clear();
      // cont.data_ft_2nd[i].clear();
      cont.data_tdc_2nd[i].clear();
      cont.data_dt_2nd[i].clear();

      cont.data_width[i].clear();
    }
  }

  int n = tdc_buffer_.size();
  unsigned int l1_traw  = tdc_buffer_[i_l1_tdc];
  unsigned int l1_t1raw = tdc_buffer_[i_l1_tdc_2nd];
  {
    unsigned int ct;
    unsigned int ft;
    decodeTdcWord(l1_traw, ct, ft);
    // cont.l1_ct = ct;
    // cont.l1_ft = ft;
    cont.l1_tdc = getTDC(l1_traw);
  }
  {
    unsigned int ct;
    unsigned int ft;
    decodeTdcWord(l1_t1raw, ct, ft);
    // cont.l1_ct1 = ct;
    // cont.l1_ft1 = ft;
    cont.l1_tdc1 = getTDC(l1_t1raw);
  }

  for (int i=sizeTdcHeader; i<n; ++i) {
    //    std::cout << "#D i = " << i  << std::endl;

    unsigned int v = tdc_buffer_[i];
    unsigned int ch;
    unsigned int ct;
    unsigned int ft;
    unsigned int data_type = decodeTdcWord(v, ch, ct, ft);
    int is2nd = (NreadDRS-1)%2;
    if(is2nd) ch += NofChModule;
    unsigned int tdc = getTDC(v);
    unsigned int dt = calc_dt(l1_traw, v);
    if (data_type == ktdc_type_leading_edge){
       std::cout << "data_type = " << std::hex << data_type << std::dec
                 << ", ch = " << ch
                 << ", ct = " << ct
                 << ", ft = " << ft
                 << std::endl;
      // cont.data_ct[ch].push_back(ct);
      // cont.data_ft[ch].push_back(ft);
      std::cout << ch << std::endl;
      cont.data_tdc[ch].push_back(tdc);
      std::cout << ch << std::endl;
      cont.data_dt[ch].push_back(dt);
    } else if ((data_type == ktdc_type_trailing_edge) || (data_type == ktdc_type_2nd_LE)){
      // cont.data_ct_2nd[ch].push_back(ct);
      // cont.data_ft_2nd[ch].push_back(ft);
      cont.data_tdc_2nd[ch].push_back(tdc);
      cont.data_dt_2nd[ch].push_back(dt);

      unsigned int width = (tdc - cont.data_tdc[ch].back()) & 0xffffff;
      cont.data_width[ch].push_back(width);
    }
  }
  return true;

}

bool
DrsDecoder::decodeADC(dataDrs& cont)
{
  std::cout << "cont size   " ;
  std::cout << cont.data_adc.size() << std::endl;
  
  bool is2nd = (NreadDRS-1)%2;
  if(!is2nd){
    for (unsigned int i=0; i<cont.data_adc.size(); ++i) {
      cont.data_adc[i].clear();
      cont.data_bl[i].clear();
      cont.data_amp[i].clear();
      cont.data_peakx[i].clear();
    }
  }

  std::cout << __LINE__ << std::endl;
  for(int i = 0; i<NofChModule; ++i){
    int ich = i;
    if(is2nd) ich =+ NofChModule;
    int fPeakX  = -9999;
    int fPeakY  = -9999;
    double fBaseLine  = 0.0;
    double fAmplitude = 0.0;
    double fIntegral  = 0.0;
	
    //Serch peak 
    std::cout << __LINE__ << std::endl;
    for (int j=fIStart; j<fIEnd; ++j){
      if (fPeakY < cont.data_wf[ich][j]){
	fPeakY = cont.data_wf[ich][j];
	fPeakX = j;
      }
    }

    //Calc Bese Line  
    int range1 = 1;
    int range2 = fPeakX - BaseParam;
    double sum=0.0;
    int    num=0;
    std::cout << __LINE__ << std::endl;
    for (int j=range1; j<range2; ++j) {
      if (TMath::Abs(cont.data_wf[ich][j+1]-cont.data_wf[ich][j]) < 10) {
	++num;
	sum += cont.data_wf[ich][j];
      }
    }
    fBaseLine = sum/num;
    fAmplitude = fPeakY - fBaseLine;
	
    //Integral
    int n = cont.data_wf[ich].size();
    int imin = fPeakX - RangeLow;
    if (imin<0) imin = 0;
    int imax = fPeakX + RangeHigh;
    if (imax>n) imax = n;
    
     std::cout << "#D Integral() "
                << " peak x = " << fPeakX
                << " imin = " << imin
                << " imax = " << imax
                << std::endl;
    std::cout << __LINE__ << std::endl;
    
    for (int j=imin; j<imax; ++j) {
      fIntegral += cont.data_wf[ich][j]-fBaseLine;
    }
    std::cout << __LINE__ << "  " << ich << std::endl;

    //Fill QDC information
    double Integral = fIntegral * kChargeUnit;
    double BaseLine = fBaseLine * kAmplitudeUnit;
    double Amplitude = fAmplitude * kAmplitudeUnit;
    double PeakX = fPeakX * kTimeUnit;
    
    cont.data_adc[ich].push_back(Integral);
    cont.data_bl[ich].push_back(BaseLine);
    cont.data_amp[ich].push_back(Amplitude);
    cont.data_peakx[ich].push_back(PeakX);
    std::cout << __LINE__ << "  " << ich << std::endl;
  }

  return true;
}




// read ----------------------------------------------------------------------
int 
DrsDecoder::read(bufType& buf, int nword)
{
  static const std::string MyFunc = "::read ";

  buf.clear();
  buf.resize(nword);

  m_ifs.read((char*)&buf[0], nword*sizeof(unsigned int));
  int n_read_word = m_ifs.gcount()/(int)sizeof(unsigned int);
  if(n_read_word != nword){
    // std::cerr << "#E : " << MyName << MyFunc
    //           << "End of file" << std::endl;
    std::cout << "#D hogee " << n_read_word << " " << nword << std::endl;
    flag.set(EoF);
  }
  
  return n_read_word;  
}

// Constructor ---------------------------------------------------------------
DrsDecoder::DrsDecoder()//:
  //head_buffer_(sizeBaseHeader),
  //exhead_buffer_(sizeExHeader),
  //body_buffer_(1),
  //NreadDRS(0)
{
  head_buffer_[0].resize(sizeBaseHeader);
  exhead_buffer_[0].resize(sizeExHeader);
  body_buffer_[0].resize(1);
  NreadDRS=0;
#if BLCOR
  resetTicCont();
#endif
}

// Destructor ----------------------------------------------------------------
DrsDecoder::~DrsDecoder()
{
  if (!m_ifs.empty()) m_ifs.reset();
}

// ---------------------------------------------------------------------------
// base line restorer
// ---------------------------------------------------------------------------
// resetTicCont --------------------------------------------------------------
void
DrsDecoder::resetTicCont()
{
  for(int i = 0; i<NofDrs*NofBoards; ++i){
    contTic[i].clear();
    contTic[i].resize(NofCell*NofBoards);
    for(int cell = 0; cell<NofCell*NofBoards; ++cell){
      contTic[i][cell].fl_read       = false;
      contTic[i][cell].time_interval = 0;
      contTic[i][cell].fl_overflow   = true;
    }// for(cell)
  }// for(i)

  return;
}// resetTicCont

// setThisEvent --------------------------------------------------------------
void
DrsDecoder::setThisEvent(int tic, bool of)
{
  for(int i = 0; i<NofDrs*2; ++i){
    for(int cell = 0; cell<NofCell*2; ++cell){
      if(of){
	contTic[i][cell].time_interval = max_tic;
	contTic[i][cell].fl_overflow   = true;	
      }else{
	if(!contTic[i][cell].fl_overflow){
	  contTic[i][cell].time_interval += tic;
	  if(contTic[i][cell].time_interval > max_tic){
	    contTic[i][cell].time_interval = max_tic;
	    contTic[i][cell].fl_overflow   = true;
	  }
	}// if(fl_overflow)
      }// if(of)
    }// for(cell)
  }// for(i)

  return;
}// setThisEvent

// getCorBL ------------------------------------------------------------------
double
DrsDecoder::getCorBL(int drs, int cellnum)
{
  // set fl_read
  contTic[drs][cellnum].fl_read = true;

  // no correction becase of over flow
  if(contTic[drs][cellnum].fl_overflow) return 0;
  
  // correction
  static const double pa =  0.475703;
  static const double pb = -10.8766;
  static const double pc =  355.101;
  static const double baseline 
    = pa*log(13000.)*log(13000.) + pb*log(13000.) + pc;
  
#if VER2
  double xmean;
  int ix = (int)contTic[drs][cellnum].time_interval;
  if((ix >> 14) != 0) ix = 10000;
  for(int i = 0; i<2; ++i){
    double multi = pow(128., i);
    for(int j = 1; j<128; ++j){
      int xmin = j*multi;
      int xmax = (j+1)*multi;

      if(xmin < ix && ix <= xmax){
	xmean= (xmin+xmax)/2.;
	break;
      }// if

    }//for(j)
  }//for(i)
  double x = log(xmean);
  double cor = (pa*x*x + pb*x + pc - baseline);
  cor += 0.5;
  cor  = (int)cor;
#else
  double x = log(contTic[drs][cellnum].time_interval);
  double cor = (pa*x*x + pb*x + pc - baseline);
#endif

  return cor;
}

// doneThisEvent -------------------------------------------------------------
void
DrsDecoder::doneThisEvent()
{
  for(int i = 0; i<NofDrs*2; ++i){
    for(int cell = 0; cell<NofCell*2; ++cell){
      if(contTic[i][cell].fl_read){
	contTic[i][cell].fl_read       = false;
	contTic[i][cell].time_interval = 0;
	contTic[i][cell].fl_overflow   = false;
      }// if(fl_read)
    }// for(cell)
  }// for(i)
  return;
}

//const char XUV[3][2] = {"X","U","V"};
//const int sigmathre[4] = {20,20,20,20};//adc threshold for each easiroc (0-3), unit is 1 sigma of pedestal


int DrsDecoder::getXUV(int ch=-1)
{
  if(64<= ch) ch -= 64;

  if(0 <= ch && ch <= 7){
    return 1;   //U (8 fibers)
  }else if(8  <= ch && ch <= 17){
    return 1;  //U (10 fibers)
  }else if(18 <= ch && ch <= 23){
    return 0;  //X (6 fibers)
  }else if(24 <= ch && ch <= 25){
    return 1;  //U (2 fibers)
  }else if(26 <= ch && ch <= 31){
    return 0; //X (6 fibers)
  }else if(32 <= ch && ch <= 37){
    return 0; //X (6 fibers)
  }else if(38 <= ch && ch <= 39){
    return 2; //V (2 fibers)
  }else if(40 <= ch && ch <= 45){
    return 0; //X (6 fibers)
  }else if(46 <= ch && ch <= 63){
    return 2; //V (18 fibers)
  }else{ 
    std::cout << "invalid ch" << ch << std::endl;
    return -1;
  }
}

//return sublayer  0 or 1
int DrsDecoder::getsublayer(int ch=-1)
{
  if(64<= ch) ch -= 64;

  int type = getXUV(ch);
  int sublayer = -1;
  if(type == 1){//U
    if(0 <= ch && ch <= 7) sublayer = 0; //L0 U0-U8 
    else if(8  <= ch && ch <= 15) sublayer = 1;//L1 U0-U7
    else if(16 <= ch && ch <= 17) sublayer = 0;//L0 U8-U9
    else if(24 <= ch && ch <= 25) sublayer = 1;//L1 U8-U9
  }else if(type == 0){//X
    if(18 <= ch && ch <= 23) sublayer = 0; //L0 X0-X5
    else if(26 <= ch && ch <= 31) sublayer = 1;//L1 X0-X5
    else if(32 <= ch && ch <= 37) sublayer = 0;//L0 X6-X11
    else if(40 <= ch && ch <= 45) sublayer = 1;//L1 X6-X11                               
  }else if(type == 2){//V
    if(38 <= ch && ch <= 39) sublayer = 0 ; //L0 V0-V1
    else if(46 <= ch && ch <= 47) sublayer = 1 ;//L1 V0-V1
    else if(48 <= ch && ch <= 55) sublayer = 0 ;//L0 V2-V9 
    else if(56 <= ch && ch <= 63) sublayer = 1 ; //L1 V2-V9
  }else{
    std::cout << "invalid ch" << ch << std::endl;
  }
  
  return sublayer;
}

//return layer0 -3
int DrsDecoder::getlayer(int ch=-1)
{
  bool is64over = false;
  if(64<= ch){
    ch -= 64;
    is64over = true;
  }

  int type = getXUV(ch);
  int layer = -1;
  if(type==1){//U
    if(0 <= ch && ch <= 7) layer = 0; //L0 U0-U8 
    else if(8 <= ch && ch <= 15) layer = 1;//L1 U0-U7
    else if(16<= ch && ch <= 17) layer = 0;//L0 U8-U9
    else if(24 <= ch && ch <= 25) layer = 1;//L1 U8-U9
  }else if(type == 0){//X
    if(18<= ch && ch <= 23) layer = 0; //L0 X0-X5
    else if(26<= ch && ch <= 31) layer = 1;//L1 X0-X5
    else if(32<= ch && ch <= 37) layer = 0;//L0 X6-X11
    else if(40<= ch && ch <= 45) layer = 1;//L1 X6-X11                               
  }else if(type == 2){//V
    if(38<= ch && ch <= 39) layer = 0 ; //L0 V0-V1
    else if(46<= ch && ch <= 47) layer = 1 ;//L1 V0-V1
    else if(48<= ch && ch <= 55) layer = 0 ;//L0 V2-V9 
    else if(56<= ch && ch <= 63) layer = 1 ; //L1 V2-V9
  }else{
    std::cout << "invalid ch" << ch << std::endl;
  }
  if(is64over) layer += 2; 
  return layer;
}

int DrsDecoder::getfiber(int ch=-1)
{
  if(64<= ch) ch = ch - 64;
  int type = getXUV(ch);
  int fiber = -1;
  //U type
  if(type==1){//U
    if(0 <= ch && ch <= 7) fiber = ch; // L0 U0-U8 
    else if(8 <= ch && ch <= 15) fiber = ch - 8;//L1 U0-U7
    else if(16<= ch && ch <= 17) fiber = ch - 8;//L0 U8-U9
    else if(24 <= ch && ch <= 25) fiber = ch - 16;//L1 U8-U9
  }else if(type == 0){//X
    if(18<= ch && ch <= 23) fiber = ch - 18; //L0 X0-X5
    else if(26<= ch && ch <= 31) fiber = ch - 26;//L1 X0-X5
    else if(32<= ch && ch <= 37) fiber = ch - 26;//L0 X6-X11
    else if(40<= ch && ch <= 45) fiber = ch - 34;//L1 X6-X11                               
  }else if(type == 2){//V
    if(38<= ch && ch <= 39) fiber = ch - 38 ; //L0 V0-V1
    else if(46<= ch && ch <= 47) fiber = ch - 46;//L1 V0-V1
    else if(48<= ch && ch <= 55) fiber = ch - 46;//L0 V2-V9 
    else if(56<= ch && ch <= 63) fiber = ch - 54; //L1 V2-V9
  }else{
    std::cout << "invalid ch" << ch << std::endl;
  }
   
  return fiber;
}

//get easiroc chip #0-3 
int DrsDecoder::geteasiroc(int ch=-1)
{
  int easiroc = -1;
  if(0 <= ch && ch <= 31) easiroc = 0;
  else if(32<= ch && ch <= 63) easiroc = 1;
  else if(64<= ch && ch <= 95) easiroc = 2;
  else if(96<= ch && ch <= 127) easiroc = 3;
  else std::cout << "invalid ch" << ch << std::endl;
  return easiroc;
}


unsigned int DrsDecoder::getBigEndian32(const char* b=NULL)
{
    //std::cout << "size of b " << sizeof(b) << std::endl;
    return ((b[0] << 24) & 0xff000000) |
           ((b[1] << 16) & 0x00ff0000) |
           ((b[2] <<  8) & 0x0000ff00) |
           ((b[3] <<  0) & 0x000000ff);
}

unsigned int DrsDecoder::Decode32bitWord(unsigned int word32bit=0)
{
  //check data format
  unsigned int frame = word32bit & 0x80808080;
  if(frame != 0x80000000){
    std::cerr << __FILE__ << " L." << __LINE__ << " Frame Error! " << std::endl;
    std::cerr << "32 bit word: " << std::hex << word32bit << std::dec << std::endl;
    return 0;
  }

  return ((word32bit & 0x7f000000) >> 3) | 
         ((word32bit & 0x007f0000) >> 2) |
         ((word32bit & 0x00007f00) >> 1) |
         ((word32bit & 0x0000007f) >> 0);
}


//ADC High Gain
bool DrsDecoder::isAdcHg(unsigned int data=0)
{
    return (data & 0x00680000) == 0x00000000;
}

//ADC Low Gain
bool DrsDecoder::isAdcLg(unsigned int data=0)
{
    return (data & 0x00680000) == 0x00080000;
}

//TDC Leading
bool DrsDecoder::isTdcLeading(unsigned int data=0)
{
    return (data & 0x00601000) == 0x00201000;
}

//TDC Trailing
bool DrsDecoder::isTdcTrailing(unsigned int data=0)
{
    return (data & 0x00601000) == 0x00200000;
}

//not impletented yet 
bool DrsDecoder::isScaler(unsigned int data=0)
{
    return (data & 0x00600000) == 0x00400000;
}
