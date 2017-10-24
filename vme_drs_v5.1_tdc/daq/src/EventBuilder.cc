#include"EventBuilder.hh"
#include"FPGAModule.hh"
#include"RegisterMap.hh"
#include"common_val.hh"

#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>

static const std::string MyName = "EventBuilder";

// Constructor ------------------------------------------------------------------
EventBuilder::EventBuilder(FPGAModule& fModule)
  :
  fModule_(fModule)
{

}

// Destructor -------------------------------------------------------------------
EventBuilder::~EventBuilder()
{

}

// ReadRegister -----------------------------------------------------------------
unsigned int
EventBuilder::ReadRegister(unsigned int local_address, int ncycle)
{
  static const std::string MyFunc = "::ReadRegister ";
  
  return fModule_.ReadModule_nCycle(EVB::mid, local_address, ncycle);
}

// WriteRegister ----------------------------------------------------------------
int
EventBuilder::WriteRegister(unsigned int local_address,
		       unsigned int reg)
{
  unsigned int write_data = 0;
  write_data = reg;
  return fModule_.WriteModule(EVB::mid, local_address, write_data);
}

// GetHeaderSize ----------------------------------------------------------------
int
EventBuilder::GetHeaderSize()
{
  unsigned int reg = ReadRegister(EVB::laddr_control, 1);
  if(0 == (reg & EVB::c_short_header)){
    return full_header_size;
  }else{
    return short_header_size;
  }
}

// ResetFIFO --------------------------------------------------------------------
void
EventBuilder::ResetFIFO()
{
  WriteRegister(EVB::laddr_reset,0);
}

// isDataReady ------------------------------------------------------------------
bool
EventBuilder::isDataReady()
{
  unsigned int reg = ReadRegister(EVB::laddr_dready, 1);
  if((reg & 0x1) == 1){
    return true;
  }else{
    return false;
  }
}

// WriteOfsTable ----------------------------------------------------------------
void
EventBuilder::WriteOfsTable(const char* dir)
{
  static const std::string MyFunc = "::WriteOfsTable ";  

  // Load table
  unsigned int ofstable[NofChDrs][NofCell];
  for(int i = 0; i<NofChDrs; ++i){
    char filename[256];
    sprintf(filename, "%s/offsettable%02d.txt", dir, i);
    std::ifstream ifs(filename);
    if(!ifs.is_open()){
      std::cout << "No such file : " << filename << std::endl;
      return;
    }

    std::cout << "#D filename = " << filename << std::endl;
    for(int cell = 0; cell<NofCell; ++cell){
      ifs >> ofstable[i][cell];
      // std::cout << ofstable[i][cell] << std::endl;
    }
  }

  std::cout << "#D " << MyName << MyFunc << std::endl;
  // Write LUT
  for(int i = 0; i<NofChDrs; ++i){
    if(i%4 == 0)  std::cout << "sending..." << std::flush;
    for(int cell = 0; cell<NofCell; ++cell){
      unsigned int laddr = EVB::laddr_readfifo + i;
      //      unsigned int reg   = (cell << 11) + ofstable[i][cell];
      unsigned int ptr = cell;
      if(ptr == 1024){ptr = 0;}
      unsigned int reg   = (cell << 11) + ofstable[i][ptr];
      // std::cout << "#D ch = " << i << " cell = " << cell
      //           << " laddr = " << laddr
      //           << " reg = " << reg
      //           << std::endl;
      WriteRegister(laddr, reg);
    }
  }
  std::cout << std::endl;
  std::cout << "#D : " << MyName << MyFunc
	    << "Tables are loaded" << std::endl;

  return;
}

// WriteBlCor1Table -------------------------------------------------------------
void
EventBuilder::WriteBlCor1Table(const char* dir)
{
  static const std::string MyFunc = "::WriteBlCor1Table ";  

  // Load table
  unsigned int ofstable[NofCell];
  char filename[256];
  sprintf(filename, "%s/baseline_cor1.txt", dir);
  std::ifstream ifs(filename);
  if(!ifs.is_open()){
    std::cout << "No such file : " << filename << std::endl;
    return;
  }

  for(int cell = 0; cell<NofCell; ++cell){
    ifs >> ofstable[cell];
      //      std::cout << ofstable[i][cell] << std::endl;
  }

  std::cout << "#D " << MyName << MyFunc << std::endl;
  // Write LUT
  for(int cell = 0; cell<NofCell; ++cell){
    if(cell%100 == 0)  std::cout << "sending..." << std::flush;
    unsigned int laddr = EVB::laddr_blcor1;
      //      unsigned int reg   = (cell << 11) + ofstable[i][cell];
    unsigned int ptr = cell;
    if(ptr == 1024){ptr = 0;}
    unsigned int reg   = (cell << 11) + ofstable[ptr];
    WriteRegister(laddr, reg);
  }
  std::cout << std::endl;
  std::cout << "#D : " << MyName << MyFunc
	    << "Baseline correction 1 is  loaded" << std::endl;

  return;
}

// WriteBlCor2Table -------------------------------------------------------------
void
EventBuilder::WriteBlCor2Table(const char* dir)
{
  static const std::string MyFunc = "::WriteBlCor2Table ";  
  static const int NofLUT         = 512;

  static const int shift_lut_addr = 7;
  static const int shift_lut_wen  = 16;

  // Load table
  unsigned int ofstable[NofLUT];
  char filename[256];
  sprintf(filename, "%s/baseline_cor2.txt", dir);
  std::ifstream ifs(filename);
  if(!ifs.is_open()){
    std::cout << "No such file : " << filename << std::endl;
    return;
  }

  for(int ptr = 0; ptr<NofLUT; ++ptr){
    ifs >> ofstable[ptr];
    //    std::cout << ofstable[ptr] << std::endl;
  }

  std::cout << "#D " << MyName << MyFunc << std::endl;
  // Write LUT
  for(int i = 0; i<NofLUT; ++i){
    if(i%100 == 0)  std::cout << "sending..." << std::flush;
    unsigned int laddr = EVB::laddr_blcor2;
      //      unsigned int reg   = (cell << 10) + ofstable[i][cell];
    unsigned int ptr = i;   // LUT addr
    unsigned int wen = 0xf; // LUT write enable
    unsigned int reg   = (wen << shift_lut_wen) + (ptr << shift_lut_addr) + ofstable[i];
    WriteRegister(laddr, reg);
  }
  std::cout << std::endl;
  std::cout << "#D : " << MyName << MyFunc
	    << "Baseline correction 2 is  loaded" << std::endl;

  return;
}

// WriteNoiseFilterTh ----------------------------------------------------------
void
EventBuilder::WriteNfTh(std::vector<unsigned int>& cont_high,
			std::vector<unsigned int>& cont_low
			)
{
  for(int i = 0; i<NofChModule; ++i){
    unsigned int laddr = EVB::laddr_low_th + i;
    unsigned int th_high = cont_high[i];
    unsigned int th_low  = cont_low[i];
    unsigned int reg     = (th_high << 12) + th_low;
    WriteRegister(laddr, reg);
  }
}

// WriteZsTh -------------------------------------------------------------------
void
EventBuilder::WriteZsTh(std::vector<unsigned int>& cont)
{
  for(int i = 0; i<NofChModule; ++i){
    unsigned int laddr = EVB::laddr_zs_th + i;
    unsigned int reg   = cont[i];
    WriteRegister(laddr, reg);
  }

  return;
}
