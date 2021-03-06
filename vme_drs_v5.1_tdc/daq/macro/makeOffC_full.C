const int NofWord = 1024;
const int NofCh   = 8;
const int NofDRS  = 4;
const int NofHist = NofCh*NofDRS;

#include<algorithm>

void makeOffC_full(char* source = "datfile.dat"){
  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  TFile *fout = new TFile("offset.root","RECREATE");
  TH1F *cell[NofHist][NofWord];
  for(int ch = 0; ch<NofHist; ++ch){
    for(int i = 0; i<NofWord; ++i){
      cell[ch][i] = new TH1F(Form("cell%d_%d",ch, i),
			     Form("cell%d_%d",ch, i),
			     4096, 0, 4096);
    }  
  }
  
  while(!ifs.eof()){
    //  for(int loop = 0; loop<100; ++loop){
    unsigned int data;
    ifs.read((char*)&data, sizeof(unsigned int));
    //    std::cout << std::hex << data << std::endl;
    if(data == 0xffffdddd){

      // wsr
      unsigned int wsr[4] = {0};
      for(int i = 0; i<4; ++i){
	ifs.read((char*)&data, sizeof(unsigned int));
	wsr[i] = data;
      }

      // cell number read
      unsigned int cellnum[4] = {0};
      for(int i = 0; i<4; ++i){
	ifs.read((char*)&data, sizeof(unsigned int));
	cellnum[i] = data;
      }

      // read count
      ifs.read((char*)&data, sizeof(unsigned int));
      unsigned int count = data;

      for(int drs = 0; drs<NofDRS; ++drs){
	for(int ch = 0; ch<NofCh; ++ch){
	  
	  unsigned int rptr = cellnum[drs];
	  for(int i = 0; i<NofWord; ++i){
	    ifs.read((char*)&data, sizeof(unsigned int));
	    //	    if(i<10){std::cout << std::hex 
	    //			       << i << " "
	    //			       << data << std::endl;}
	    unsigned int adc_data = data & 0xfff;
	    if(rptr == 1024){rptr = 0;}
	    cell[drs*8 + ch][rptr++]->Fill((double)adc_data);
	  }
	}
      }
    }
    
  }

  // double mean[NofHist][NofWord];
  // double max_in1ch[NofHist];
  // double max_ch[16];
  // TF1 *fit = new TF1("fgaus","gaus");
  // for(int drs = 0; drs<NofDRS; ++drs){
  //   for(int ch = 0; ch<NofCh; ++ch){
  //     for(int i = 0; i<NofWord; ++i){
  // 	//	mean[drs*8+ch][i] = cell[drs*8+ch][i]->GetMean();
  // 	double max_x = cell[drs*8+ch][i]->GetBinCenter(cell[drs*8+ch][i]->GetMaximumBin());
  // 	cell[drs*8+ch][i]->Fit("fgaus","RQNO","", max_x-10, max_x+10);;
  // 	mean[drs*8+ch][i] = fit->GetParameter("Mean");
  // 	//	if(mean[drs*8+ch][i] >400){
  // 	//	  std::cout << "drs "<< drs << " ch " << ch << " i " << i << " "
  // 	//		    << mean[drs*8+ch][i] << std::endl;
  // 	//	}
	
  //     }
  //     max_in1ch[drs*8+ch] = *max_element(mean[drs*8+ch], mean[drs*8+ch]+1023);
  //     std::cout << "max in " << drs*8+ch << " "<< max_in1ch[drs*8+ch] << std::endl;
  //   }
  // }

  // std::cout << "make table" << std::endl;
  // std::ofstream *ofs[NofHist];
  // for(int ch = 0; ch<16; ++ch){
  //   for(int cas = 0; cas<2; ++cas){
  //     ofs[ch*2 +cas] = new std::ofstream(Form("offsettable%d.txt", ch*2+cas));
    
  //     double max = max_in1ch[ch*2+1] > max_in1ch[ch*2] ? max_in1ch[ch*2+1] : max_in1ch[ch*2];
  //     for(int i = 0; i<NofWord; ++i){
  //  	unsigned int val = (unsigned int)max-mean[2*ch+cas][i];
  //  	*ofs[ch*2 +cas] << val << std::endl;
  // 	if(val>255){
  // 	  std::cout << "CH" << ch*2+cas << " " << val << std::endl;
  // 	}
  //     }

  //     ofs[ch*2 +cas]->close();
  //   }
  // }
  
  fout->Write();
  fout->Close();

  
  return;
}
