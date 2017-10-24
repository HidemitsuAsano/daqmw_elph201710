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
int main(int argc, char* argv[])
{
  if(argc <= 2){
    std::cout << "Usage: simple_decoder [input data file] [output rootfile]"
	      << std::endl;
    return 0;
  }
  
  std::string nameInFile  = argv[inputfile];
  std::string nameOutFile = argv[outputfile];

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
  }

  int event_num = 0;
  while(gDec.getNextEvent()){
    //    if(event_num%1000 == 0){
      std::cout << "#D : Event " << event_num << std::endl;
      //    }
    gDec.decode(data_cont);  
    tree.Fill();
    ++event_num;
  }

  fout.Write();
  fout.Close();
  
  return 0;
}
