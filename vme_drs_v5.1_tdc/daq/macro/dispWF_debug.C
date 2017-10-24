const unsigned int NofWord = 1024;
const int NofCh = 8;
const int NofDRS = 4;
const int NofRead = NofCh*NofDRS;

void dispWF_debug(char* source = "datfile.dat"){
  gROOT->SetStyle("MyStyle");

  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  HB1(0,"qdc", 4000,0,4000);
  HB1(1,"qdc", 4000,0,4000);

  std::vector<double> x[2];
  std::vector<double> y[2];

  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(2,1);

  while(!ifs.eof()){
    unsigned int data;
    ifs.read((char*)&data, sizeof(unsigned int));

    if(data == 0xffffdddd){
      // header read
      unsigned int wsr[4];
      unsigned int cellnum[4];
      for(int i = 0; i<4; ++i){
	ifs.read((char*)&wsr[i], sizeof(unsigned int));
	std::cout << std::hex << wsr[i] << " " << std::flush;
      }

      std::cout << std::dec << std::endl;
      for(int i = 0; i<4; ++i){
	ifs.read((char*)&cellnum[i], sizeof(unsigned int));
	std::cout << cellnum[i] << " " << std::flush;
      }

      std::cout << std::dec << std::endl;
      ifs.read((char*)&data, sizeof(unsigned int));
      unsigned int count = data;
      std::cout << count << std::endl;
      

      for(int ch = 0; ch<2; ++ch){
	c1->cd(ch+1);

	int integ = 0;
	for(unsigned int cell = 0; cell<count; ++cell){
	  ifs.read((char*)&data, sizeof(unsigned int));
	  unsigned int drs_ch   = ((data >> 16) & 0x1f);
	  unsigned int adc_data = data & 0xfff;
	  //	  std::cout << std::dec << cell << " " << std::hex << data << std::endl;
	  y[ch].push_back((double)adc_data);
	  x[ch].push_back((double)cell);

	  if(20 <cell && cell < 120){
	    integ += adc_data-100;
	  }
	}

	int qdc = (integ >> 4);
	HF1(ch, qdc);
	
	TGraph *graph = new TGraph(count-1,
				   &x[ch][0],
				   &y[ch][0]);
	SetTitle(graph, "cell number", "ADC ch [0.5 mV/ch]");
	graph->SetMarkerStyle(6);
	graph->Draw("ap");
	
      }

    }
    
    c1->Update();
    getchar();      
    x[0].clear();
    y[0].clear();
    x[1].clear();
    y[1].clear();
  }
  
  return;
}
