const unsigned int NofWord = 1024;
const int NofCh = 8;
const int NofDRS = 4;
const int NofRead = NofCh*NofDRS;

void dispWF_1ch(char* source = "datfile.dat"){
  gROOT->SetStyle("MyStyle");

  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  std::vector<double> x;
  std::vector<double> y;

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
      unsigned int count = data;
      std::cout << count << std::endl;
      for(unsigned int cell = 0; cell<count; ++cell){
	ifs.read((char*)&data, sizeof(unsigned int));
	unsigned int drs_ch   = ((data >> 16) & 0x1f);
	unsigned int adc_data = data & 0xfff;
	//	  std::cout << std::dec << cell << " " << std::hex << data << std::endl;
	y.push_back((double)adc_data);
	x.push_back((double)cell);
      }
	
      TGraph *graph = new TGraph(count-1,
				 &x[0],
				 &y[0]);
      SetTitle(graph, "cell number", "ADC ch [0.5 mV/ch]");
      graph->SetMarkerStyle(1);
      graph->Draw("ap");
      
    }
    
    c1->Update();
    getchar();      
    x.clear();
    y.clear();
  }
  
  return;
}
