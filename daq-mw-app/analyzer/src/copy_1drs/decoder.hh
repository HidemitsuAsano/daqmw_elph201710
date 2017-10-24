#ifndef DECODERH_
#define DECODERH_

#include<fstream>
#include<bitset>
#include<vector>
#include <boost/iostreams/filtering_stream.hpp>

struct dataDrs;

class DrsDecoder{

public:
  //----------------------------------------------------------------------------
  // constants for TDC
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
  
public:
  bool HasTDCData() const { return has_tdc_data; }
  void SetDecodeTDC(bool flag=true) { has_tdc_data = flag; }
  bool Open(const char* name);
  bool isGood(){return flag[Good];};
  bool isEOF(){return flag[EoF];};

  bool getNextEvent();
  bool decode(dataDrs& obj);
  bool decodeTDC(dataDrs& obj);
  bool decodeADC(dataDrs& obj);

  static DrsDecoder& getInstance();

private:
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

  typedef std::vector<unsigned int>           bufType;
  typedef std::vector<unsigned int>::iterator itrType;

  bufType head_buffer_;
  bufType exhead_buffer_;
  bufType body_buffer_;
  bufType tdc_buffer_;
  
  enum Flags{ Good, EoF, ExHead, FirstData, DoubleData,
	      sizeFlags};
  std::bitset<sizeFlags> flag;

  boost::iostreams::filtering_istream m_ifs;

  bool has_tdc_data;

  int read(bufType& buf, int nword);

  DrsDecoder();
  ~DrsDecoder();
  DrsDecoder(const DrsDecoder& obj);
  DrsDecoder& operator=(const DrsDecoder& obj);

  // base line restorer
  struct dataTic{
    bool fl_read;
    int time_interval;
    bool fl_overflow;
  };

  typedef std::vector<dataTic> ticType;
  ticType contTic[4]; // for each DRS4 chip

  static const int non_cascade = 1;
  static const int cascade     = 2;

  static const int max_tic     = 13000;

  void   resetTicCont();
  void   setThisEvent(int tic, bool of);
  double getCorBL(int drs, int cellnum);
  void   doneThisEvent();
};

// getInstance --------------------------------------------------------------
inline
DrsDecoder&
DrsDecoder::getInstance()
{
  static DrsDecoder obj;
  return obj;
}

#endif
