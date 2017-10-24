#ifndef LTC2656_12SPI_
#define LTC2656_12SPI_

#include"Uncopyable.hh"

class FPGAModule;

/*
  SPI manager of LTC2656fa_12

  Data structure of SPI is 32 bits full width mode
  <<--- (XXXXXXXX) + (CMD 4bits) + (ADDR 4bits) + (DATA 12bits) + (XXXX)

  RBCP data structure
  Extended DATA is used for data transmit
  extended DATA <= (CMD 4bits) + (ADDR 4bits) + (DATA 8bits higher side)
  DATA          <= (DATA 4bits lower side) + (4bits don't care)
*/

class LTC2656_12_SPI
  : private Uncopyable<LTC2656_12_SPI>
{
 public:
  // FPGA module part --------------------------------------------------------
  static const unsigned int cmd_shift   = 20U;
  static const unsigned int addr_shift  = 16U;
  static const unsigned int ddata_shift = 4U;  // Make 4 bits don't care

public:
  LTC2656_12_SPI(FPGAModule& fModule);
  virtual ~LTC2656_12_SPI();
  
  int OneCycle(unsigned int chip_num,
	       unsigned int dac_cmd,
	       unsigned int dac_addr,
	       unsigned int dac_v);
  int Load(const char* filename);

 protected:
  FPGAModule& fModule_;

 private:
  void Dump(unsigned int val, int length);
};

#endif
