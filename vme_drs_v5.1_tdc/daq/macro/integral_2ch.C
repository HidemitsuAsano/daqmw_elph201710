const unsigned int NofWord = 1024;
const int NofCh = 8;
const int NofDRS = 4;
const int NofRead = NofCh*NofDRS;

void integral_2ch(char* source = "datfile.dat"){
  gROOT->SetStyle("MyStyle");

  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  std::ifstream *offsettable[NofRead];
  for(int drs = 0; drs<NofDRS; ++drs){
    for(int ch = 0; ch<NofCh; ++ch){
      offsettable[drs*8 + ch] = new std::ifstream(Form("ot_mp_0325/offsettable%d.txt",ch+drs*8));
    }
  }
  
  if(!offsettable[0]->is_open()){
    std::cout << "no table" << std::endl;
    return;
  }

  double offset[NofRead][NofWord];
  
  for(int drs = 0; drs<NofDRS; ++drs){
    for(int ch = 0; ch<NofCh; ++ch){
      for(int i = 0; i<NofWord; ++i){
	*offsettable[drs*8 + ch] >> offset[drs*8 + ch][i];
      }
    }
  }


  TFile *fout = new TFile("nanika.root","recreate");
  TH1F* hist[4];
  hist[0]=new TH1F("hist1","hist1",4000,0,4000);
  hist[1]=new TH1F("hist2","hist2",4000,0,4000);
  hist[2]=new TH1F("hist3","hist3",4000,0,4000);
  hist[3]=new TH1F("hist4","hist4",4000,0,4000);

  while(!ifs.eof()){
    unsigned int data;
    ifs.read((char*)&data, sizeof(unsigned int));

    if(data == 0xffffdddd){
      // header read
      unsigned int wsr;
      unsigned int cellnum;
      ifs.read((char*)&data, sizeof(unsigned int));      
      wsr = data;
      for(int i = 1; i<4; ++i){
	ifs.read((char*)&data, sizeof(unsigned int));
      }

      ifs.read((char*)&data, sizeof(unsigned int));
      cellnum = data;
      for(int i = 1; i<4; ++i){
	ifs.read((char*)&data, sizeof(unsigned int));
      }

      ifs.read((char*)&data, sizeof(unsigned int));
      unsigned int count = data+1;
      for(int ch = 0; ch<2; ++ch){

	unsigned int rptr = cellnum;
	int integral = 0;
	for(unsigned int cell = 0; cell<count; ++cell){
	  ifs.read((char*)&data, sizeof(unsigned int));
	  unsigned int drs_ch   = ((data >> 16) & 0x1f);
	  unsigned int adc_data = data & 0xfff;
	  //	  std::cout << std::dec << cell << " " << std::hex << data << std::endl;
	  if(rptr == 1024){rptr = 0;}
	  if(400 < cell && cell < 500){
	    //	    int adc_val = (int)adc_data+offset[drs_ch][rptr];//-1700;
	    int adc_val = (int)adc_data-1700;
	    integral += (adc_val>>0);
	    hist[ch+2]->Fill(adc_val);

	  }
	  
	  ++rptr;
	}
	//	std::cout << integral << std::endl;
	int adc = integral>>4;
	hist[ch]->Fill(adc-2000);
      }
    }
  }

  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(2,2);

  for(int i = 0; i<4; ++i){
    c1->cd(1+i);
    hist[i]->DrawCopy();
  }

  fout->Write();
  fout->Close();

  return;
}
