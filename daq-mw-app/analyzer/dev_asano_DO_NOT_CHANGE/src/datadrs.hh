#ifndef DATADRS_
#define DATADRS_

#include"common_val_drs4.hh"
#include<vector>

struct dataDrs{
  int nword_header[2];
  int nword_body[2];
  int global_tag[2];
  int local_tag[2];
  int tic_count[2];
  int fl_double_data[2]; // 1:12bit wf data x2, 0:14bit wf data x1

  //int nword_header;
  //int nword_body;
  //int global_tag;
  //int local_tag;
  //int tic_count;
  //int fl_double_data; // 1:12bit wf data x2, 0:14bit wf data x1
  
  int wsr[NofDrs*NofBoards];
  int cellnum[NofDrs*NofBoards];
  int modid;
  
  std::vector<std::vector<double> > data_wf;
  unsigned int data_qdc[NofChModule*NofBoards];

  std::vector<std::vector<double> > data_adc;
  std::vector<std::vector<double> > data_bl;
  std::vector<std::vector<double> > data_amp;
  std::vector<std::vector<double> > data_peakx;

  std::vector<std::vector<unsigned int> > data_tdc;
  std::vector<std::vector<unsigned int> > data_dt;
  std::vector<std::vector<unsigned int> > data_tdc_2nd;
  std::vector<std::vector<unsigned int> > data_dt_2nd;
  std::vector<std::vector<unsigned int> > data_width;
  unsigned int l1_tdc;
  unsigned int l1_tdc1;
  //unsigned int l1_tdc[2];
  //unsigned int l1_tdc1[2];

  dataDrs()
    : data_wf(NofChModule*NofBoards),
      data_adc(NofChModule*NofBoards),
      data_bl(NofChModule*NofBoards),
      data_amp(NofChModule*NofBoards),
      data_peakx(NofChModule*NofBoards),
      data_tdc(NofChModule*NofBoards),
      data_dt(NofChModule*NofBoards),
      data_tdc_2nd(NofChModule*NofBoards),
      data_dt_2nd(NofChModule*NofBoards),
      data_width(NofChModule*NofBoards)
  {}
  
};

#endif
