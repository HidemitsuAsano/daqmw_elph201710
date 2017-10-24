// -*- C++ -*-

#include <iostream>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <Compression.h>


#include "ScalerDecoder.hh"

enum ArgName {
  kExec,
  kInputFile,
  kOutputFile,
  kSizeArg
};

//______________________________________________________________________________
int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cout << "Usage: " << argv[0]
              << " [input data file] [output rootfile]"
              << std::endl;
    return 0;
  }

  
  const std::string nameInFile  = argv[kInputFile];
  const std::string nameOutFile = argv[kOutputFile];

  ScalerDecoder decoder;
  decoder.Initialize(nameInFile);
  ScalerDecoder::DataContainer& cont = decoder.GetContainer();
  
  TFile fout(nameOutFile.c_str(), "recreate", "", ROOT::CompressionSettings(ROOT::kLZMA, 1));
  TTree stree("stree", "scaler data");
  for (int i=0; i<cont.scaler.size(); ++i) {
    stree.Branch(Form("ch%d", i), &(cont.scaler[i]));
  }
  stree.Branch("timestamp", &(cont.timestamp));
  stree.Branch("timediff", &(cont.timediff));
  stree.Branch("spill", &cont.spill);

  TTree ttree("ttree", "trigger flag");
  ttree.Branch("flag", &(cont.trigger_flag));
  ttree.Branch("timestamp", &(cont.trig_time));
  ttree.Branch("timediff", &(cont.trig_timediff));
  ttree.Branch("spill",    &(cont.spill));

  decoder.SetTree(&stree, &ttree);
  for (int e=0; ; ++e) {
    if (e%1000 == 0) std::cout << "#D num scaler latched = " << e << std::endl;
    if (!decoder.Read()) break;
    decoder.Decode();
  }

  
  fout.Write();
  fout.Close();

  return 0;
}
