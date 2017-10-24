const unsigned int NofWord = 1024;
const int NofCh = 8;
const int NofDRS = 4;
const int NofRead = NofCh*NofDRS;

void decodeQDC(char* source = "datfile.dat"){
  gROOT->SetStyle("MyStyle");

  std::ifstream ifs(source, std::ios::binary);

  if(!ifs.is_open()){
    std::cout << "no file" << std::endl;
    return;
  }


  new TFile("nanika.root","recreate");
  for(int i = 0; i<16; ++i){
    HB1(i,"QDC", 10000,0,10000);
  }
  
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
      for(int ch = 0; ch<16; ++ch){
	ifs.read((char*)&data, sizeof(unsigned int));
	unsigned int drs_ch   = ((data >> 16) & 0x1f);
	unsigned int adc_data = data & 0xfff;
	HF1(ch, adc_data);
      }

    }
  }

  gFile->Write();
  gFile->Close();
  
  return;
}
