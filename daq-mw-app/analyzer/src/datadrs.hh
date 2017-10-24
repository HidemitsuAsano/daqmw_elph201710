#ifndef DATADRS_
#define DATADRS_

#include"common_val.hh"
#include<vector>

struct dataDrs{
  int nword_header;
  int nword_body;
  int global_tag;
  int local_tag;
  int tic_count;
  int fl_double_data; // 1:12bit wf data x2, 0:14bit wf data x1

  int wsr[NofDrs*2];
  int cellnum[NofDrs*2];
  
  std::vector<std::vector<double> > data_wf;
  unsigned int data_qdc[NofChModule*NofBoards];

  std::vector<std::vector<double> > data_adc;
  std::vector<std::vector<double> > data_bl;
  std::vector<std::vector<double> > data_amp;
  std::vector<std::vector<double> > data_peakx;
  // std::vector<std::vector<unsigned int> > data_ct;
  // std::vector<std::vector<unsigned int> > data_ft;
  std::vector<std::vector<unsigned int> > data_tdc;
  std::vector<std::vector<unsigned int> > data_dt;
  // std::vector<std::vector<unsigned int> > data_ct_2nd;
  // std::vector<std::vector<unsigned int> > data_ft_2nd;
  std::vector<std::vector<unsigned int> > data_tdc_2nd;
  std::vector<std::vector<unsigned int> > data_dt_2nd;
  std::vector<std::vector<unsigned int> > data_width;

  // unsigned int l1_ct;
  // unsigned int l1_ft;
  unsigned int l1_tdc;

  // unsigned int l1_ct1;
  // unsigned int l1_ft1;
  unsigned int l1_tdc1;

  dataDrs()
    : data_wf(NofChModule*NofBoards),
      // data_ct(NofChModule),
      // data_ft(NofChModule),
      data_adc(NofChModule*NofBoards),
      data_bl(NofChModule*NofBoards),
      data_amp(NofChModule*NofBoards),
      data_peakx(NofChModule*NofBoards),
      // data_ct_2nd(NofChModule),
      // data_ft_2nd(NofChModule),
      //data_tdc(NofChModule*NofBoards),
      //data_dt(NofChModule*NofBoards),
      data_tdc(),
      data_dt(),
      //data_tdc_2nd(NofChModule*NofBoards),
      //data_dt_2nd(NofChModule*NofBoards),
      //data_width(NofChModule*NofBoards)
      data_tdc_2nd(),
      data_dt_2nd(),
      data_width()
  {}
  
};

#endif
