// -*- C++ -*-
#ifndef Drs4QdcDecoder_hh
#define Drs4QdcDecoder_hh

#include <vector>

struct dataDrs;

class Drs4QdcDecoder{

public:

  //----------------------------------------------------------------------------
  // constants for DRS4 decode
  enum IndexBaseHeader{
    i_magic, i_datasize, i_tag, i_tic,
    sizeBaseHeader
  };

  enum IndexExHeader{
    i_drs0, i_drs1, i_drs2, i_drs3,
    sizeExHeader
  };
  
  static const unsigned int data_size_mask    = 0xffff;
  static const unsigned int header_size_mask  = 0xf;
  static const unsigned int header_size_shift = 16;

  static const unsigned int gtag_shift        = 16;
  static const unsigned int gtag_mask         = 0xf;
  static const unsigned int ltag_mask         = 0xffff;

  static const unsigned int tic_mask          = 0xfffff;
  static const unsigned int datastr_mask      = 0x1;
  static const unsigned int datastr_shift     = 20;

  static const unsigned int wsr_shift         = 16;
  static const unsigned int wsr_mask          = 0xff;
  static const unsigned int cellnum_mask      = 0x3ff;

  static const unsigned int ch_mask           = 0xf;
  static const unsigned int ch_shift          = 24;

  static const unsigned int type_shift        = 28;
  static const unsigned int type_mask         = 0xf;
  static const unsigned int type_header       = 0xf;
  static const unsigned int type_wf           = 0xd;
  static const unsigned int type_qdc          = 0x9;

  // double data structure
  static const unsigned int wf_high_shift    = 12;
  static const unsigned int wf_mask_12bit    = 0xfff;
  // sigle data structure
  static const unsigned int wf_mask_14bit    = 0x3fff;

  static const unsigned int qdc_mask          = 0xffff;

  enum Flags{ Good, EoF, ExHead, FirstData, DoubleData,
              sizeFlags};


  //----------------------------------------------------------------------------
  // constants for TDC decode
  
  enum IndexTdcHeader{
    i_tdc_magic, i_tdc_l1_cnt, i_tdc_l2_cnt, i_l1_tdc, i_l1_tdc_2nd,
    i_frame_size, sizeTdcHeader
  };

  static const unsigned int ktdc_data_type_shift  = 28;
  static const unsigned int ktdc_data_type_mask   = 0xf;
  enum ETdcDataType {
    ktdc_type_leading_edge    = 0x0,
    ktdc_type_trailing_edge   = 0x1,
    ktdc_type_2nd_LE          = 0x2,
    ktdc_type_magic           = 0x4,
    ktdc_type_l1_counter      = 0x5,
    ktdc_type_l2_counter      = 0x6,
    ktdc_type_l1_tdc          = 0x7,
    ktdc_type_frame_size      = 0xc,
    ktdc_type_last_frame_size = 0xd,
    ktdc_type_error           = 0x9,
    ktdc_type_num_hit         = 0xa,
    ktdc_type_trailer         = 0xb
  };
  static const unsigned int ktdc_num_frame_shift   = 16;
  static const unsigned int ktdc_num_frame_mask    = 0xfff;
  static const unsigned int ktdc_frame_size_shift  = 0;
  static const unsigned int ktdc_frame_size_mask   = 0xfff;
  static const unsigned int ktdc_ch_id_shift       = 24;
  static const unsigned int ktdc_ch_id_mask        = 0xf;
  static const unsigned int ktdc_coarse_time_shift = 10;
  static const unsigned int ktdc_coarse_time_mask  = 0x3fff; // 14 bits
  static const unsigned int ktdc_fine_time_shift   = 0;
  static const unsigned int ktdc_fine_time_mask    = 0x3ff; // 10 bits
  //----------------------------------------------------------------------------
  
  
  Drs4QdcDecoder();
  ~Drs4QdcDecoder();
  Drs4QdcDecoder(const Drs4QdcDecoder&);
  Drs4QdcDecoder& operator=(const Drs4QdcDecoder&);

  int operator()(const std::vector<unsigned int>::const_iterator& data_begin,
                 dataDrs& cont,
                 bool decode_tdc=false);

  int DecodeAdc(const std::vector<unsigned int>::const_iterator& data_begin,
                dataDrs& cont);

  int DecodeTdc(const std::vector<unsigned int>::const_iterator& data_begin,
                dataDrs& cont);
  
};

#endif
