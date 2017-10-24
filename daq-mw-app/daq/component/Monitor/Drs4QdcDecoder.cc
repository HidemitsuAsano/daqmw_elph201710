// -*- C++ -*-

#include <string>
#include <iostream>
#include <bitset>

#include "datadrs.hh"

#include "Drs4QdcDecoder.hh"


//______________________________________________________________________________
unsigned int
decodeTdcWord(unsigned int v,
              unsigned int& ct,
              unsigned int& ft)
{
  unsigned int data_type = (v >> Drs4QdcDecoder::ktdc_data_type_shift) & Drs4QdcDecoder::ktdc_data_type_mask;
  ct = (v >> Drs4QdcDecoder::ktdc_coarse_time_shift) & Drs4QdcDecoder::ktdc_coarse_time_mask;
  ft = (v >> Drs4QdcDecoder::ktdc_fine_time_shift) & Drs4QdcDecoder::ktdc_fine_time_mask;
  return data_type;
}

//______________________________________________________________________________
unsigned int
decodeTdcWord(unsigned int v,
              unsigned int& ch,
              unsigned int& ct,
              unsigned int& ft)
{
  ch = (v >> Drs4QdcDecoder::ktdc_ch_id_shift) & Drs4QdcDecoder::ktdc_ch_id_mask;
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

//______________________________________________________________________________
Drs4QdcDecoder::Drs4QdcDecoder()
{
}

//______________________________________________________________________________
Drs4QdcDecoder::~Drs4QdcDecoder()
{
}

//______________________________________________________________________________
int
Drs4QdcDecoder::operator()(const std::vector<unsigned int>::const_iterator& data_begin,
                    dataDrs& cont,
                    bool decode_tdc)
{
  int n_word_adc = DecodeAdc(data_begin, cont);
  if (n_word_adc<0) return n_word_adc;

  int n_word_tdc = 0;
  if (decode_tdc)
    n_word_tdc = DecodeTdc(data_begin + n_word_adc, cont);
  if (n_word_tdc<0) return n_word_tdc;
  //std::cerr << "n word adc " << n_word_adc << std::endl;
  //std::cerr << "n word tdc " << n_word_tdc << std::endl;

  return n_word_adc + n_word_tdc;
}

//______________________________________________________________________________
int
Drs4QdcDecoder::DecodeAdc(const std::vector<unsigned int>::const_iterator& data_begin,
                   dataDrs& cont)
{
  std::bitset<sizeFlags> flag;
  flag.set(Good);
  flag.set(FirstData);

  for (int i=0; i<NofChModule; ++i) {
    cont.data_wf[i].clear();
  }
  
  // component header
  std::vector<unsigned int>::const_iterator compHeaderBegin = data_begin;
  std::vector<unsigned int>::const_iterator compHeaderEnd   = data_begin+2;

  // base header
  std::vector<unsigned int>::const_iterator baseHeaderBegin;
  std::vector<unsigned int>::const_iterator baseHeaderEnd;
  if ((*compHeaderBegin) != 0x504d4f43) { // 'COMP' in ascii code
    baseHeaderBegin = compHeaderBegin;
  } else {
    baseHeaderBegin = compHeaderEnd;
  }
  baseHeaderEnd = baseHeaderBegin + sizeBaseHeader;

  cont.nword_body   = (*(baseHeaderBegin + i_datasize)) & data_size_mask;
  cont.nword_header =  (*(baseHeaderBegin + i_datasize) >> header_size_shift) & header_size_mask;
  if (flag[FirstData]) {
    if (cont.nword_header == sizeBaseHeader + sizeExHeader) {
      flag.set(ExHead);
    }
    flag.reset(FirstData);
  }

  std::vector<unsigned int>::const_iterator exheadBegin = baseHeaderEnd;
  std::vector<unsigned int>::const_iterator exheadEnd; ;
  if (flag[ExHead])  {
    exheadEnd = exheadBegin + sizeExHeader;
  }
  
  std::vector<unsigned int>::const_iterator bodyBegin;
  if (flag[ExHead]) {
    bodyBegin = exheadEnd;
  } else {
    bodyBegin = baseHeaderEnd;
  }
  std::vector<unsigned int>::const_iterator bodyEnd = bodyBegin + cont.nword_body;
 
  cont.global_tag     = (*(baseHeaderBegin + i_tag) >> gtag_shift) & gtag_mask;
  cont.local_tag      = (*(baseHeaderBegin + i_tag))  & ltag_mask;
  cont.tic_count      = (*(baseHeaderBegin + i_tic))  & tic_mask;
  cont.fl_double_data = (*(baseHeaderBegin + i_tic)) >> datastr_shift;

  // decode ex header
  if (flag[ExHead]) {
    for (unsigned int i=0; i<sizeExHeader; ++i) {
      cont.wsr[i]     = ((*(exheadBegin+i)) >> wsr_shift) & wsr_mask;
      cont.cellnum[i] = (*(exheadBegin+i)) & cellnum_mask;
    }
  }

  // decode body
  for (int i=0; i<cont.nword_body; ++i) {
    unsigned int b0 = *(bodyBegin+i);
    unsigned int data_type = (b0 >> type_shift) & type_mask;
    unsigned int ch        = (b0 >> ch_shift)   & ch_mask;
    if (data_type == type_wf){
      if (cont.fl_double_data){
        double data_high = (double) ((b0 >> wf_high_shift) & wf_mask_12bit);
        double data_low  = (double) (b0 & wf_mask_12bit);

        cont.data_wf[ch].push_back(data_low);
        cont.data_wf[ch].push_back(data_high);
      }else {
        unsigned int data = (b0 & wf_mask_14bit);
        unsigned int extra_bits = data & 0x3;
        double wf_data = (double) (data >> 2) + extra_bits*0.25;
        cont.data_wf[ch].push_back(wf_data);
      }
    } else if (data_type == type_qdc) {
      unsigned int data_qdc = b0 & qdc_mask;
      cont.data_qdc[ch] = data_qdc;
    } else {
      return -1;
    }
  }

  return std::distance(data_begin, bodyEnd);
}

//______________________________________________________________________________
int
Drs4QdcDecoder::DecodeTdc(const std::vector<unsigned int>::const_iterator& data_begin,
                   dataDrs& cont)
{
  //  std::cout << "#D decodeTdc" << std::endl;
  unsigned int nch = cont.data_tdc.size();
  for (unsigned int i=0; i<nch; ++i) {
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


  //  int n = tdc_buffer_.size();
  unsigned int l1_traw  = *(data_begin + i_l1_tdc);
  unsigned int l1_t1raw = *(data_begin + i_l1_tdc_2nd);
  {
    unsigned int ct;
    unsigned int ft;
    decodeTdcWord(l1_traw, ct, ft);
    // cont.l1_ct = ct;
    // cont.l1_ft = ft;
    cont.l1_tdc = getTDC(l1_traw);
  }

  int i = 0;
  for (i=sizeTdcHeader; ; ++i) {
    //    std::cout << "#D i = " << i  << std::endl;

    unsigned int v = *(data_begin+i);
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
      // cont.data_ct[ch].push_back(ct);
      // cont.data_ft[ch].push_back(ft);
      cont.data_tdc[ch].push_back(tdc);
      cont.data_dt[ch].push_back(dt);
    } else if ((data_type == ktdc_type_trailing_edge) || (data_type == ktdc_type_2nd_LE)){
      // cont.data_ct_2nd[ch].push_back(ct);
      // cont.data_ft_2nd[ch].push_back(ft);
      cont.data_tdc_2nd[ch].push_back(tdc);
      cont.data_dt_2nd[ch].push_back(dt);

      unsigned int width = (tdc - cont.data_tdc[ch].back()) & 0xffffff;
      cont.data_width[ch].push_back(width);
    } else if (data_type == ktdc_type_trailer) {
      break;
    }
  }

  int ret = i + 1;
  return ret;
}
