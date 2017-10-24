#include"LTC2656_12_SPI.hh"
#include"FPGAModule.hh"
#include"ParameterManager.hh"
#include"RegisterMap.hh"

#include<string>
#include<iostream>
#include<fstream>

static const std::string MyName = "LTC2656_12_SPI";

// Constructor ------------------------------------------------------------------
LTC2656_12_SPI::LTC2656_12_SPI(FPGAModule& fModule)
  :
  fModule_(fModule)
{

}

// Destructor -------------------------------------------------------------------
LTC2656_12_SPI::~LTC2656_12_SPI()
{

}

// OneCycle ---------------------------------------------------------------------
int
LTC2656_12_SPI::OneCycle(unsigned int chip_num,
			 unsigned int dac_cmd,
			 unsigned int dac_addr,
			 unsigned int dac_v
			 )
{
  static const std::string MyFunc = "::OneCycle ";
  
  // Set target chip
  int ret;
  if(0 > (ret = fModule_.WriteModule(LTC::mid, LTC::addr_chipsel, chip_num))){
    return ret;
  }

  // Set Data to DFF in FPGA
  unsigned int spi_data      
    = (dac_cmd << cmd_shift)
    | (dac_addr << addr_shift)
    | (dac_v << ddata_shift);
  // std::cout << "#D : " << MyName << MyFunc
  // 	    << "SPI data set" << std::endl;
  // Dump(spi_data, 24);
  // std::cout << std::endl;

  if(0 > (ret = fModule_.WriteModule(LTC::mid, LTC::addr_regsdi, spi_data))){
    return ret;
  }

  // Execute command
  return fModule_.WriteModule(LTC::mid, LTC::addr_exec, 0);
}

// Load -------------------------------------------------------------------------
int
LTC2656_12_SPI::Load(const char* filename)
{
  static const std::string MyFunc = "::Load ";

  std::ifstream ifs(filename);
  if(!ifs.is_open()){
    std::cerr << "#E : " << MyName << MyFunc
	      << "No such file exists" << std::endl;
    return -1;
  }
  h_Utility::ParameterManager<unsigned int> regDac(ifs);
  
  std::cout << "#D : " << MyName << MyFunc
	    << "Start Laod DAC" << std::endl;

  int ret;
  unsigned int reg_cmd = LTC::cmd_wr_n_upd_n;
  // First DAC
  {
    unsigned int NofCh = 4;
    unsigned int target_device = LTC::ctrl_dac1;
    regDac.stChangeParamSet("firstDAC");
    for(unsigned int caddr = LTC::addr_A; caddr<NofCh; ++caddr){
      unsigned int val = regDac.GetParameter((int)caddr, "register");
      ret = OneCycle(target_device, reg_cmd, caddr, val);
      
      if(ret < 0){
	std::cout << "#E : " << MyName << MyFunc 
		  << "Load error, 1st DAC, No" << caddr << std::endl;
	return ret;
      }
      std::cout << "#D : " << MyName << MyFunc 
		<< "register 1st DAC, No" << caddr 
		<< ", val = " << val
		<< std::endl;
    }
    
    std::cout << std::endl;
  }

  // Second DAC
  {
    unsigned int NofCh = 8;
    unsigned int target_device = LTC::ctrl_dac2;
    regDac.stChangeParamSet("secondDAC");
    for(unsigned int caddr = LTC::addr_A; caddr<NofCh; ++caddr){
      unsigned int val = regDac.GetParameter((int)caddr, "register");
      ret = OneCycle(target_device, reg_cmd, caddr, val);
      
      if(ret < 0){
	std::cout << "#E : " << MyName << MyFunc 
		  << "Load error, 2nd DAC, No" << caddr << std::endl;
	return ret;
      }
      std::cout << "#D : " << MyName << MyFunc 
		<< "register 2nd DAC, No" << caddr
		<< ", val = " << val
		<< std::endl;
    }

    std::cout << std::endl;
  }

  // Third DAC
  {
    unsigned int NofCh = 8;
    unsigned int target_device = LTC::ctrl_dac3;
    regDac.stChangeParamSet("thirdDAC");
    for(unsigned int caddr = LTC::addr_A; caddr<NofCh; ++caddr){
      unsigned int val = regDac.GetParameter((int)caddr, "register");
      ret = OneCycle(target_device, reg_cmd, caddr, val);
      
      if(ret < 0){
	std::cout << "#E : " << MyName << MyFunc 
		  << "Load error, 3rd DAC, No" << caddr << std::endl;
	return ret;
      }
      std::cout << "#D : " << MyName << MyFunc 
		<< "register 3rd DAC, No" << caddr
		<< ", val = " << val
		<< std::endl;
    }

    std::cout << std::endl;
  }

  std::cout << "#D : " << MyName << MyFunc
	    << "End Laod DAC" << std::endl;

  return ret;
}

// Dump -------------------------------------------------------------------------
void
LTC2656_12_SPI::Dump(unsigned int val, int length)
{
  std::cout << "#D 0x"
	    << std::hex << val << std::dec << std::endl;
  for(int i = length-1; i>-1; --i){
    if((val >> i ) & 1){
      std::cout << "!" << std::flush;
    }else{
      std::cout << "." << std::flush;
    }
  }
}
