#ifndef AD9637SPI_
#define AD9637SPI_

#include"Uncopyable.hh"

class FPGAModule;

class AD9637_SPI
  : private Uncopyable<AD9637_SPI>
{
 public:
  // FPGA module part --------------------------------------------------------
  static const unsigned int chip_mask    = 0x3;

public:
  AD9637_SPI(FPGAModule& fModule);
  virtual ~AD9637_SPI();
  
  unsigned int ReadRegister(unsigned int local_address, int ncycle);
  
  int          WriteRegister(unsigned int local_address,
			     unsigned int adc_address,
			     unsigned int wd);
  
  int          InitializeDevices();
  int          SetTestMode(int mode);

 protected:
  FPGAModule& fModule_;

 private:
  static const unsigned int NofADC = 4;
  
  void Dump(unsigned int val, int length);
};
#endif
