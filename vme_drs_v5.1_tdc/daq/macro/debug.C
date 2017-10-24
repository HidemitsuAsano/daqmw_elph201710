const int NofWord = 1024;
const int NofDRS  = 4;
const int NofCh   = 4;
const int NofHist = 32;

#include<algorithm>
#include<fstream>

void debug(char* source = "datfile.dat"){
  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }

  std::ifstream *offsettable[NofHist];
  for(int drs = 0; drs<NofDRS; ++drs){
    for(int ch = 0; ch<8; ++ch){
      offsettable[drs*8 + ch] = new std::ifstream(Form("ot_mp_0325/offsettable%d.txt",ch+drs*8));
    }
  }

  if(!offsettable[0]->is_open()){
    std::cout << "no table" << std::endl;
    return;
  }

  double offset[NofHist][NofWord];
  
  for(int drs = 0; drs<NofDRS; ++drs){
    for(int ch = 0; ch<8; ++ch){
      for(int i = 0; i<NofWord; ++i){
	*offsettable[drs*8 + ch] >> offset[drs*8 + ch][i];
      }
    }
  }

  std::vector<double> x;
  std::vector<double> y;
  TCanvas *c1 = new TCanvas("c1","c1");

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
	//	std::cout << std::hex << wsr[i] << " ";
      }
      //      std::cout << std::dec << endl;

      // cell number read
      unsigned int cellnum[4] = {0};
      for(int i = 0; i<4; ++i){
	ifs.read((char*)&data, sizeof(unsigned int));
	cellnum[i] = data;
	//	std::cout << cellnum[i] << " ";
      }
      //      std::cout << std::dec << endl;

      // read count
      ifs.read((char*)&data, sizeof(unsigned int));
      unsigned int count = data;

      for(int drs = 0; drs<NofDRS; ++drs){
	//	std::cout << "drs " << drs << " " << iofs << std::endl;
	
	for(int ch = 0; ch<NofCh; ++ch){
	  int iofs = 0;
	  if(wsr[drs] == 0xaa && cellnum[drs] < 765){
	    iofs = 0;
	  }else if(wsr[drs] == 0x55 && cellnum[drs] > 765){
	    iofs = 0;
	  }else{
	    iofs = 1;
	  }

	  unsigned int rptr = cellnum[drs];
	  for(int i = 0; i<NofWord; ++i){
	    int drsch = drs*8 + 2*ch + iofs;
	    ifs.read((char*)&data, sizeof(unsigned int));
	    //	    if(i<10){std::cout << std::hex 
	    //			       << i << " "
	    //			       << data << std::endl;}
	    unsigned int adc_data = data & 0xfff;
	    adc_data = adc_data;//+offset[drsch][rptr];
	    y.push_back((double)adc_data);
	    x.push_back((double)i);

	    if(rptr == 1024){
	      rptr = 0;
	      if(iofs == 0 && wsr[drs] != 0xff) iofs = 1;
	      if(iofs == 1 && wsr[drs] != 0xff) iofs = 0;
	    }

	    //	    cell[drs*8 + 2*ch + iofs][rptr++]->Fill((double)adc_data);

	    if(drsch == 0 && rptr == 100){
	      if(adc_data<2150){
		printf("low  %x, %d\n", wsr[0], cellnum[0]);
	      }else{
		printf("high %x, %d\n", wsr[0], cellnum[0]);
	      }
	    }

	    ++rptr;
	  } // word

	}// DrsCh
      }//DRS4
      TGraph *graph = new TGraph(count-1,
				 &x[0],
				 &y[0]);
      SetTitle(graph, "cell number", "ADC ch [0.5 mV/ch]");
      graph->SetMarkerStyle(6);
      graph->Draw("ap");

      c1->Update();
      getchar();     
      
      x.clear();
      y.clear();
    }// Event

  }// while
  
  return;
}
