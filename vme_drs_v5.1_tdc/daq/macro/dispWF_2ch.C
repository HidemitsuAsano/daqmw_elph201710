const unsigned int NofWord = 1024;
const int NofCh = 8;
const int NofDRS = 4;
const int NofRead = NofCh*NofDRS;

void dispWF_2ch(int shift, char* source = "datfile.dat"){
  gROOT->SetStyle("MyStyle");

  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  std::ifstream *offsettable[NofRead];
  for(int drs = 0; drs<NofDRS; ++drs){
    for(int ch = 0; ch<NofCh; ++ch){
      offsettable[drs*8 + ch] = new std::ifstream(Form("ot_mp_0318/offsettable%d.txt",ch+drs*8));
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

  double x[NofRead][NofWord];
  double y[NofRead][NofWord];

  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(2,1);

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

      std::cout << std::hex << wsr << " "
		<< std::dec << cellnum << " "
		<< std::endl;
      ifs.read((char*)&data, sizeof(unsigned int));
      unsigned int count = data+1;
      std::cout << count << std::endl;
      for(int ch = 0; ch<2; ++ch){
	c1->cd(ch+1);

	unsigned int rptr = cellnum+shift;
	double integral = 0;
	for(unsigned int cell = 0; cell<count; ++cell){
	  ifs.read((char*)&data, sizeof(unsigned int));
	  unsigned int drs_ch   = ((data >> 16) & 0x1f);
	  unsigned int adc_data = data & 0xfff;
	  //	  std::cout << std::dec << cell << " " << std::hex << data << std::endl;
	  if(rptr == 1024){rptr = 0;}
	  y[ch][cell] = (double)adc_data+offset[drs_ch][rptr];
	  x[ch][cell] = (double)cell;
	  ++rptr;
	}
	
	TGraph *graph = new TGraph(count-1,
				   x[ch],
				   y[ch]);
	SetTitle(graph, "cell number", "ADC ch [0.5 mV/ch]");
	graph->SetMarkerStyle(1);
	graph->Draw("ap");

      }
      
      c1->Update();
      getchar();      
    }
  }

  return;
}
