#include<ios>
#include<iostream>
#include<algorithm>

#include"common_val.hh"
#include"decoder.hh"
#include"datadrs.hh"
#include "register_config.hh"

#include"TFile.h"
#include"TH1.h"
#include"TH1F.h"
#include"TF1.h"
#include"TString.h"
#include<Compression.h>


const int NofChInUnit = 4;
//const int th_cellnum = 765;
const int th_cellnum1 = register_config::kEVBCascadeThreshold1;
const int th_cellnum2 = register_config::kEVBCascadeThreshold2;


enum ArgName{
  exec, inputfile, outputfile,
  sizeArg
};

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ofstable_maker [input data file] [output rootfile]"
	      << std::endl;
    return 0;
  }
  
  std::string nameInFile  = argv[inputfile];
  std::string nameOutFile = argv[outputfile];

  dataDrs data_cont;
  DrsDecoder& gDec = DrsDecoder::getInstance();
  gDec.Open(nameInFile.c_str());
  if(!gDec.isGood()) return -1;

  // Output ROOT file -------------------------------------------------
  TFile fout(nameOutFile.c_str(), "RECREATE", "", ROOT::CompressionSettings(ROOT::kLZMA, 1));
  TH1F *hist_cell[NofChDrs][NofCell];
  for(int ch = 0; ch<NofChDrs; ++ch){
    for(int i = 0; i<NofCell; ++i){
      hist_cell[ch][i] = new TH1F(Form("cell%d_%d",ch, i),
				  Form("cell%d_%d",ch, i),
				  4096, 0, 4096);
    }//for(i)
  }// for(ch)

  // Decode data -------------------------------------------------------
  int event_num = 0;
  while(gDec.getNextEvent()){
    if(event_num%1000 == 0){
      std::cout << "#D : Event " << event_num << std::endl;
    }
    gDec.decode(data_cont);  

    for(int drs = 0; drs<NofDrs; ++drs){
      for(int ch = 0; ch<NofChInUnit; ++ch){
	int iofs = 0;
	if(true 
	   && data_cont.wsr[drs] == 0xaa 
	   //&& data_cont.cellnum[drs] < th_cellnum
           && ((data_cont.cellnum[drs] < th_cellnum1) || (data_cont.cellnum[drs] >= th_cellnum2))
	   ){
	  iofs = 1;
	}else if(true 
		 && data_cont.wsr[drs] == 0x55 
                 //&& data_cont.cellnum[drs] > th_cellnum
                 && ((data_cont.cellnum[drs] > th_cellnum1) && (data_cont.cellnum[drs] <= th_cellnum2))
		 ){
	  iofs = 1;
	}else{
	  iofs = 0;
	}

	int rptr = data_cont.cellnum[drs];
	for(int i = 0; i<NofCell; ++i){
	  //	  std::cout << drs << " " << ch << " " << i << std::endl;
	  double adc_data = data_cont.data_wf[drs*4 + ch][i];
	  if(rptr == 1024){
	    rptr = 0;
	    if(iofs == 0 && data_cont.wsr[drs] != 0xff){
	      iofs = 1;
	    }else if(iofs == 1 && data_cont.wsr[drs] != 0xff){
	      iofs = 0;
	    }// iofs
	  }// rptr
	  hist_cell[drs*8 + 2*ch + iofs][rptr++]->Fill(adc_data);
	}// word
      }// Ch
    }// DRS

    ++event_num;
  }// Event (while)

  // Fit with Gaus ---------------------------------------------------------
  double mean[NofChDrs][NofCell];
  double max_in1ch[NofChDrs];
  //  double max_ch[NofChInUnit*NofDrs];
  TF1 *fit = new TF1("fgaus","gaus");
  for(int drs = 0; drs<NofDrs; ++drs){
    for(int ch = 0; ch<8; ++ch){
      for(int i = 0; i<NofCell; ++i){
  	double max_x = hist_cell[drs*8+ch][i]->GetBinCenter(hist_cell[drs*8+ch][i]->GetMaximumBin());
  	hist_cell[drs*8+ch][i]->Fit("fgaus","RQNO","", max_x-10, max_x+10);;
  	mean[drs*8+ch][i] = fit->GetParameter("Mean");
	
      }// for(i) word
      double *maxptr = std::max_element(mean[drs*8+ch], mean[drs*8+ch]+1023);
      max_in1ch[drs*8+ch] = *maxptr;
      int max_cell = int(maxptr - mean[drs*8+ch]);
      std::cout << "#D : max in " << drs*8+ch << " "<< max_in1ch[drs*8+ch] 
		<< " in cell " << max_cell << std::endl;
    }// for(ch) ch
  }// for(drs) DRS

  // Make offset table -----------------------------------------------------
  std::cout << "#D : make table" << std::endl;
  std::ofstream ofs[NofChDrs];
  for(int ch = 0; ch<NofChModule; ++ch){
    for(int cas = 0; cas<2; ++cas){
      ofs[ch*2 +cas].open(Form("offsettable%02d.txt", ch*2+cas), std::ios::out);
    
      double max = max_in1ch[ch*2+1] > max_in1ch[ch*2] ? max_in1ch[ch*2+1] : max_in1ch[ch*2];
      for(int i = 0; i<NofCell; ++i){
   	unsigned int val = (unsigned int)max-mean[2*ch+cas][i];
   	double diff      = max-mean[2*ch+cas][i];
   	double frac      = diff - (double)val;
	unsigned int extra_bit    = 0;
	if(0.25 <= frac && frac < 0.5){
	  extra_bit = 1;
	}else if(0.5 <= frac && frac < 0.75){
	  extra_bit = 2;
	}else if(0.75 <= frac){
	  extra_bit = 3;      
	}

	unsigned int val_lut = (val << 2) + extra_bit;
	
   	ofs[ch*2 +cas] << val_lut << std::endl;
	//  	if(val>255){
	if(false){
  	  std::cout << "#D : CH" << ch*2+cas << " " << val_lut << std::endl;
  	}
      }// for(i) word

      ofs[ch*2 +cas].close();
    }// for(cas)
  }// for(ch)

  fout.Write();
  fout.Close();
  
  return 0;
}
