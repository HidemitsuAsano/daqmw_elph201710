#include<ios>
#include<iostream>
#include<cstdlib>
#include<cmath>
#include<algorithm>
#include<functional>

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

// const double kTimeUnit      = 1.0;
// const double kAmplitudeUnit = 1.0;
// const double kChargeUnit    = 1.0;
// const double kTDCTimeUnit   = 1.0;

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

//______________________________________________________________________________
unsigned int
decodeTdcWord(unsigned int v,
              unsigned int& ct,
              unsigned int& ft)
{
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
  m_ifs.push(boost::iostreams::gzip_decompressor());
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
  
  return true;
}

// decode --------------------------------------------------------------------
bool
DrsDecoder::decode(dataDrs& cont)
{
  static const std::string MyFunc = "::decode ";

  // clear vector
  for(int i = 0; i<NofChModule; ++i){
    cont.data_wf[i].clear();
  }

  // decode base header
  cont.nword_header = (head_buffer_[i_datasize] >> header_size_shift) & header_size_mask;
  cont.nword_body     =  head_buffer_[i_datasize] & data_size_mask;
  cont.global_tag     = (head_buffer_[i_tag] >> gtag_shift) & gtag_mask;
  cont.local_tag      =  head_buffer_[i_tag] & ltag_mask;
  cont.tic_count      =  head_buffer_[i_tic] & tic_mask;
  cont.fl_double_data = (head_buffer_[i_tic] >> datastr_shift ) & datastr_mask;

  // std::cout << "#D " << std::hex << cont.nword_header
  //           << " " << cont.nword_body
  //           << " " << cont.local_tag
  //           << std::endl;

#if BLCOR
  bool fl_overflow = false;
  if(cont.tic_overflow) fl_overflow = true;
  setThisEvent(cont.tic_count, cont.tic_overflow);
#endif

  // decode ex header
  if(flag[ExHead]){
    for(unsigned int i = 0; i<sizeExHeader; ++i){
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
  for(int i = 0; i<cont.nword_body; ++i){
    unsigned int data_type = (body_buffer_[i] >> type_shift) & type_mask;
    unsigned int ch        = (body_buffer_[i] >> ch_shift) & ch_mask;
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
      }//Peak serch
	
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

  //  std::cout << "#D decodeTDC" << std::endl;
  for (unsigned int i=0; i<cont.data_tdc.size(); ++i) {
    cont.data_tdc[i].clear();
    cont.data_dt[i].clear();

    cont.data_tdc_2nd[i].clear();
    cont.data_dt_2nd[i].clear();

    cont.data_width[i].clear();
  }

  int n = tdc_buffer_.size();
  unsigned int l1_traw  = tdc_buffer_[i_l1_tdc];
  unsigned int l1_t1raw = tdc_buffer_[i_l1_tdc_2nd];
  {
    unsigned int ct;
    unsigned int ft;
    decodeTdcWord(l1_traw, ct, ft);
    cont.l1_tdc = getTDC(l1_traw);
  }
  {
    unsigned int ct;
    unsigned int ft;
    decodeTdcWord(l1_t1raw, ct, ft);
    cont.l1_tdc1 = getTDC(l1_t1raw);
  }

  for (int i=sizeTdcHeader; i<n; ++i) {
    //    std::cout << "#D i = " << i  << std::endl;

    unsigned int v = tdc_buffer_[i];
    unsigned int ch;
    unsigned int ct;
    unsigned int ft;
    unsigned int data_type = decodeTdcWord(v, ch, ct, ft);
    unsigned int tdc = getTDC(v);
    unsigned int dt = calc_dt(l1_traw, v);
    if (data_type == ktdc_type_leading_edge){
      // std::cout << "data_type = " << std::hex << data_type << std::dec
      //           << ", ch = " << ch
      //           << ", ct = " << ct
      //           << ", ft = " << ft
      //           << std::endl;
      cont.data_tdc[ch].push_back(tdc*kTDCTimeUnit);
      cont.data_dt[ch].push_back(dt*kTDCTimeUnit);
    } else if ((data_type == ktdc_type_trailing_edge) || (data_type == ktdc_type_2nd_LE)){
      cont.data_tdc_2nd[ch].push_back(tdc*kTDCTimeUnit);
      cont.data_dt_2nd[ch].push_back(dt*kTDCTimeUnit);

      unsigned int width = (tdc - cont.data_tdc[ch].back()) & 0xffffff;
      cont.data_width[ch].push_back(width*kTDCTimeUnit);
    }
  }
  return true;

}

//______________________________________________________________________________
bool
DrsDecoder::decodeADC(dataDrs& cont)
{
  for (unsigned int i=0; i<cont.data_adc.size(); ++i) {
    cont.data_adc[i].clear();
    cont.data_bl[i].clear();
    cont.data_amp[i].clear();
    cont.data_peakx[i].clear();
  }

  for(int i = 0; i<NofChModule; ++i){

    int fPeakX  = -9999;
    int fPeakY  = -9999;
    double fBaseLine  = 0.0;
    double fAmplitude = 0.0;
    double fIntegral  = 0.0;
	
    //Serch peak 
    for (int j=fIStart; j<fIEnd; ++j){
      if (fPeakY < cont.data_wf[i][j]){
	fPeakY = cont.data_wf[i][j];
	fPeakX = j;
      }
    }

    //Calc Bese Line  
    int range1 = 1;
    int range2 = fPeakX - BaseParam;
    double sum=0.0;
    int    num=0;
    for (int j=range1; j<range2; ++j) {
      if (TMath::Abs(cont.data_wf[i][j+1]-cont.data_wf[i][j]) < 10) {
	++num;
	sum += cont.data_wf[i][j];
      }
    }
    fBaseLine = sum/num;
    fAmplitude = fPeakY - fBaseLine;
	
    //Integral
    int n = cont.data_wf[i].size();
    int imin = fPeakX - RangeLow;
    if (imin<0) imin = 0;
    int imax = fPeakX + RangeHigh;
    if (imax>n) imax = n;
    
    // std::cout << "#D Integral() "
    //            << " peak x = " << fPeakX
    //            << " imin = " << imin
    //            << " imax = " << imax
    //            << std::endl;
    
    for (int j=imin; j<imax; ++j) {
      fIntegral += cont.data_wf[i][j]-fBaseLine;
    }
    
    //Fill QDC information
    double Integral = fIntegral * kChargeUnit;
    double BaseLine = fBaseLine * kAmplitudeUnit;
    double Amplitude = fAmplitude * kAmplitudeUnit;
    double PeakX = fPeakX * kTimeUnit;
    
    cont.data_adc[i].push_back(Integral);
    cont.data_bl[i].push_back(BaseLine);
    cont.data_amp[i].push_back(Amplitude);
    cont.data_peakx[i].push_back(PeakX);
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
    //std::cout << "#D hogee " << n_read_word << " " << nword << std::endl;
    flag.set(EoF);
  }
  
  return n_read_word;  
}

// Constructor ---------------------------------------------------------------
DrsDecoder::DrsDecoder():
  head_buffer_(sizeBaseHeader),
  exhead_buffer_(sizeExHeader),
  body_buffer_(1)
{
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
  for(int i = 0; i<NofDrs; ++i){
    contTic[i].clear();
    contTic[i].resize(NofCell);

    for(int cell = 0; cell<NofCell; ++cell){
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
  for(int i = 0; i<NofDrs; ++i){
    for(int cell = 0; cell<NofCell; ++cell){
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
  for(int i = 0; i<NofDrs; ++i){
    for(int cell = 0; cell<NofCell; ++cell){
      if(contTic[i][cell].fl_read){
	contTic[i][cell].fl_read       = false;
	contTic[i][cell].time_interval = 0;
	contTic[i][cell].fl_overflow   = false;
      }// if(fl_read)
    }// for(cell)
  }// for(i)
  return;
}
