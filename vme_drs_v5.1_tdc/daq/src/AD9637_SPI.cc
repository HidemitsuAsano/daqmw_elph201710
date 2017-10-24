#include"AD9637_SPI.hh"
#include"FPGAModule.hh"
#include"RegisterMap.hh"

#include<iostream>
#include<string>
#include<cstdlib>
#include<unistd.h>

static const std::string MyName = "AD9637_SPI";

// Constructor ------------------------------------------------------------------
AD9637_SPI::AD9637_SPI(FPGAModule& fModule)
  :
  fModule_(fModule)
{

}

// Destructor -------------------------------------------------------------------
AD9637_SPI::~AD9637_SPI()
{

}

// ReadRegister -----------------------------------------------------------------
unsigned int
AD9637_SPI::ReadRegister(unsigned int local_address, int ncycle)
{
  static const std::string MyFunc = "::ReadRegister ";
  return fModule_.ReadModule_nCycle(AD9637::mid, local_address, ncycle);
}

// WriteRegister ----------------------------------------------------------------
int
AD9637_SPI::WriteRegister(unsigned int local_address,
			      unsigned int adc_address,
			      unsigned int reg)
{
  unsigned int write_data = 0;
  write_data = (adc_address << exdata_shift) + reg;
  return fModule_.WriteModule(AD9637::mid, local_address, write_data);
}

// InitializeDevices ------------------------------------------------------------
int
AD9637_SPI::InitializeDevices()
{
  static const std::string MyFunc = "::InitializeDevice ";

  // Software reset -----------------------------------------------------
  unsigned int local_address = AD9637::laddr_set_reg;
  unsigned int adc_address   = AD9637::addr_spiconfig;
  unsigned int write_data    = 0x3c;
  if(WriteRegister(local_address, adc_address, write_data) < 0){
    std::cerr << MyName << MyFunc
	      << "Write error, set reg (software reset)" << std::endl;
    return -1;
  }

  for(unsigned int i = 0; i<NofADC; ++i){
    usleep(1000);    

    local_address = AD9637::laddr_exec + (i & chip_mask);
    adc_address   = 0; // no meaning, but any way insert 0
    write_data    = 0; // no meaning, but any way insert 0
    if(WriteRegister(local_address, adc_address, write_data) < 0){
      std::cerr << MyName << MyFunc
		<< "Write error, software reset" << std::endl;
      return -1;
    }

    usleep(1000);

    unsigned int ret_reg;
    if(0x18 != (ret_reg = ReadRegister(local_address, 1))){
      std::cerr << MyName << MyFunc
		<< "Chip soft reset fail\n" 
		<< "  Returned reg is " << std::hex << ret_reg
		<< std::endl;
      return -1;
    }
  }// for(i)

  std::cout << "#D : " << MyName << MyFunc 
	    << "Software reset done" << std::endl;

  // Power mode reset ---------------------------------------------------
  local_address = AD9637::laddr_set_reg;
  adc_address   = AD9637::addr_pwmode;
  write_data    = 0x3;
  if(WriteRegister(local_address, adc_address, write_data) < 0){
    std::cerr << MyName << MyFunc
	      << "Write error, set reg (power mode reset)" << std::endl;
    return -1;
  }

  for(unsigned int i = 0; i<NofADC; ++i){
    usleep(1000);    

    local_address = AD9637::laddr_exec + (i & chip_mask);
    adc_address   = 0; // no meaning, but any way insert 0
    write_data    = 0; // no meaning, but any way insert 0
    if(WriteRegister(local_address, adc_address, write_data) < 0){
      std::cerr << MyName << MyFunc
		<< "Write error, power mode reset" << std::endl;
      return -1;
    }
  }// for(i)

  std::cout << "#D : " << MyName << MyFunc 
	    << "Power mode reset done" << std::endl;

  sleep(1);

  // Power mode chip run ------------------------------------------------
  local_address = AD9637::laddr_set_reg;
  adc_address   = AD9637::addr_pwmode;
  write_data    = 0x0;
  if(WriteRegister(local_address, adc_address, write_data) < 0){
    std::cerr << MyName << MyFunc
	      << "Write error, set reg (power mode chip run)" << std::endl;
    return -1;
  }

  for(unsigned int i = 0; i<NofADC; ++i){
    usleep(1000);    

    local_address = AD9637::laddr_exec + (i & chip_mask);
    adc_address   = 0; // no meaning, but any way insert 0
    write_data    = 0; // no meaning, but any way insert 0
    if(WriteRegister(local_address, adc_address, write_data) < 0){
      std::cerr << MyName << MyFunc
		<< "Write error, power mode chip run" << std::endl;
      return -1;
    }
  }// for(i)

  std::cout << "#D : " << MyName << MyFunc 
	    << "Set power mode, chip running, done" << std::endl;

  // output format offset binary
  local_address = AD9637::laddr_set_reg;
  adc_address   = AD9637::addr_outmode;
  write_data    = 0x0;
  if(WriteRegister(local_address, adc_address, write_data) < 0){
    std::cerr << MyName << MyFunc
	      << "Write error, set reg (output format)" << std::endl;
    return -1;
  }

  for(unsigned int i = 0; i<NofADC; ++i){
    usleep(1000);    

    local_address = AD9637::laddr_exec + (i & chip_mask);
    adc_address   = 0; // no meaning, but any way insert 0
    write_data    = 0; // no meaning, but any way insert 0
    if(WriteRegister(local_address, adc_address, write_data) < 0){
      std::cerr << MyName << MyFunc
		<< "Write error, output format" << std::endl;
      return -1;
    }
  }// for(i)

  // Initialization done -----------------------------------------------
  std::cout << "#D : " << MyName << MyFunc 
	    << "output format set done" << std::endl;

  std::cout << "#D : " << MyName << MyFunc 
	    << "Initialization is finished" << std::endl;

  return 0;
}

// SetTestMode ------------------------------------------------------------------
int
AD9637_SPI::SetTestMode(int mode)
{
  static const std::string MyFunc = "::SetTestMode ";

  // test mode  -----------------------------------------------------
  unsigned int local_address = AD9637::laddr_set_reg;
  unsigned int adc_address   = AD9637::addr_test;
  unsigned int write_data    = mode & 0xf;
  if(WriteRegister(local_address, adc_address, write_data) < 0){
    std::cerr << MyName << MyFunc
	      << "Write error, set reg (software reset)" << std::endl;
    return -1;
  }

  for(unsigned int i = 0; i<NofADC; ++i){
    usleep(1000);    

    local_address = AD9637::laddr_exec + (i & chip_mask);
    adc_address   = 0; // no meaning, but any way insert 0
    write_data    = 0; // no meaning, but any way insert 0
    if(WriteRegister(local_address, adc_address, write_data) < 0){
      std::cerr << MyName << MyFunc
		<< "Write error, software reset" << std::endl;
      return -1;
    }
  }// for(i)

  std::cout << "#D : " << MyName << MyFunc 
	    << "Test mode is set to 0x" << std::hex << mode << std::endl;
  std::cout << std::dec;

  return  0;
}
