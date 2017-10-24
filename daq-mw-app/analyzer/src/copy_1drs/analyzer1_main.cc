#include<iostream>

#include"common_val.hh"
#include"decoder.hh"
#include"datadrs.hh"

#include"TFile.h"
#include"TTree.h"
#include"TString.h"

#include <Compression.h>

//______________________________________________________________________________
enum ArgName{
  exec, inputfile, outputfile,
  sizeArg
};

//______________________________________________________________________________
bool
find_opt(int argc, char* argv[], const std::string& item)
{
  bool ret=false;
  std::vector<std::string> v(argv, argv+argc);
  for (unsigned int i=0; i<v.size(); ++i) {
    if (v[i] == item) {
      ret = true;
      break;
    }
  }
  return ret;
}

//______________________________________________________________________________
int main(int argc, char* argv[])
{
  if(argc <= 2){
    std::cout << "Usage: simple_decoder [input data file] [output rootfile] (--without-tdc)"
	      << std::endl;
    return 0;
  }
  
  std::string nameInFile  = argv[inputfile];
  std::string nameOutFile = argv[outputfile];
  bool hasTDCData         = !find_opt(argc, argv, "--without-tdc");
  std::cout << "hasTDCData = " << hasTDCData << std::endl;
  
  dataDrs data_cont;
  DrsDecoder& gDec = DrsDecoder::getInstance();
  gDec.Open(nameInFile.c_str());
  if(!gDec.isGood()) return -1;
   
  TFile fout(nameOutFile.c_str(), "RECREATE", "", ROOT::CompressionSettings(ROOT::kLZMA, 1));
  TTree tree("tree","tree");

  tree.Branch("TIC",   &data_cont.tic_count,      "TIC/I");
  tree.Branch("DD",    &data_cont.fl_double_data, "DD/I");
  tree.Branch("WSR",   data_cont.wsr,             "WSR[4]/I");
  tree.Branch("CELLN",  data_cont.cellnum,        "CELLN[4]/I");
  tree.Branch("QDC",    data_cont.data_qdc,       "QDC[16]/I");

  for(int i = 0; i<NofChModule; ++i){
    tree.Branch(Form("WaveForm%d",i), &(data_cont.data_wf[i]));
    tree.Branch(Form("adc%d", i),  &(data_cont.data_adc[i]));
    tree.Branch(Form("bl%d", i), &(data_cont.data_bl[i]));
    tree.Branch(Form("amp%d", i), &(data_cont.data_amp[i]));
    tree.Branch(Form("peakx%d", i), &(data_cont.data_peakx[i]));
  }

  if (hasTDCData) {
    for(int i = 0; i<NofChModule; ++i){
      tree.Branch(Form("tdc%d", i), &(data_cont.data_tdc[i]));
      tree.Branch(Form("dt%d", i), &(data_cont.data_dt[i]));
      
      tree.Branch(Form("tdc_2nd%d", i), &(data_cont.data_tdc_2nd[i]));
      tree.Branch(Form("dt_2nd%d", i), &(data_cont.data_dt_2nd[i]));
      
      tree.Branch(Form("width%d", i), &(data_cont.data_width[i]));
    }

    tree.Branch("l1_tdc", &data_cont.l1_tdc);
    tree.Branch("l1_tdc1", &data_cont.l1_tdc1);
  }
  gDec.SetDecodeTDC(hasTDCData);

  int event_num = 0;
  while(gDec.getNextEvent()){
    if(event_num%1000 == 0){
      std::cout << "#D : Event " << event_num << std::endl;
    }
    gDec.decode(data_cont);
    gDec.decodeTDC(data_cont);
    gDec.decodeADC(data_cont);
    tree.Fill();

    //std::cout << "TDC= " << data_cont.data_dt[0].size() << std::endl;
    // if( data_cont.data_dt[0].size()>0 &&
    // 	data_cont.data_dt[1].size()>0 &&
    // 	data_cont.data_dt[2].size()>0 &&
    // 	data_cont.data_dt[3].size()>0 &&
    // 	data_cont.data_dt[4].size()>0 &&
    // 	data_cont.data_dt[5].size()>0 ){
    //   tree.Fill();
    // }

    ++event_num;
  }

  fout.Write();
  fout.Close();
  
  return 0;
}

