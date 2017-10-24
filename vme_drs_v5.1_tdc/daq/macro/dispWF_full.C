const unsigned int NofWord = 1024;
const int NofCh = 8;
const int NofDRS = 4;
const int NofRead = NofCh*NofDRS;

void dispWF_full(char* source = "datafile.dat"){
  gROOT->SetStyle("MyStyle");

  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  std::ifstream *offsettable[NofRead];
  for(int drs = 0; drs<NofDRS; ++drs){
    for(int ch = 0; ch<NofCh; ++ch){
      offsettable[drs*8 + ch] = new std::ifstream(Form("ot_fromROI/offsettable%d.txt",ch+drs*10));
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

  TCanvas *c1 = new TCanvas("c1","c1",1200,800);
  c1->Divide(8,4);
  TCanvas *c2 = new TCanvas("c2","c2");

  while(!ifs.eof()){
    unsigned int data;
    ifs.read((char*)&data, sizeof(unsigned int));

    if(data == 0xffffdddd){
      // cell number read
      unsigned int cellnum[4] = {0};
      for(int i = 0; i<4; ++i){
	ifs.read((char*)&data, sizeof(unsigned int));
	cellnum[i] = data;
	std::cout << data << " ";
      }
      std::cout << std::endl;

      for(int drs = 0; drs<4; ++drs){
	
      for(int channel = 0; channel<8; ++channel){
	c1->cd(drs*8 + channel+1);
	unsigned int ch_buffer;
	ifs.read((char*)&ch_buffer, sizeof(unsigned int));

	unsigned int rptr = cellnum[drs];
	for(int i = 0; i<NofWord; ++i){
	  ifs.read((char*)&data, sizeof(unsigned int));
	  if(rptr == 1024){rptr = 0;}
	  y[drs*8+channel][i] = (double)data-offset[drs*8+channel][rptr++];
	  x[drs*8+channel][i] = (double)i;
	}

	TGraph *graph = new TGraph(NofWord,
				   x[drs*8+channel],
				   y[drs*8+channel]);
	SetTitle(graph, "cell number", "ADC ch [0.5 mV/ch]");
	graph->SetMarkerStyle(1);
	graph->Draw("ap");
	if((drs*8+channel) == 0){
	  c2->cd(0);
	  graph->Draw("apl");
	}
      }
      }
      c1->Update();
      c2->Update();
      getchar();      
    }
  }

  return;
}
