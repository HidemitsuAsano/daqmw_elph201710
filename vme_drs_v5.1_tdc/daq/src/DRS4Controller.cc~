#include"DRS4Controller.hh"
#include"FPGAModule.hh"
#include"RegisterMap.hh"
#include"common_val.hh"

#include<iostream>
#include<string>
#include<cstdlib>
#include<unistd.h>

static const std::string MyName = "DRS4Controller";

// Constructor -----------------------------------------------------------------
DRS4Controller::DRS4Controller(FPGAModule& fModule)
  :
  fModule_(fModule)
{

}

// Destructor ------------------------------------------------------------------
DRS4Controller::~DRS4Controller()
{

}

// ReadRegister ----------------------------------------------------------------
unsigned int
DRS4Controller::ReadRegister(unsigned int local_address, int ncycle)
{
  static const std::string MyFunc = "::ReadRegister ";
  
  return fModule_.ReadModule_nCycle(DRS::mid, local_address, ncycle);
}

// WriteRegister ---------------------------------------------------------------
int
DRS4Controller::WriteRegister(unsigned int local_address,
				  unsigned int drs_addr,
				  unsigned int reg)
{
  unsigned int write_data = 0;
  write_data = (drs_addr << exdata_shift) + reg;
  return fModule_.WriteModule(DRS::mid, local_address, write_data);
}

// ResetDRS --------------------------------------------------------------------
void
DRS4Controller::ResetDRS()
{
  unsigned int reg = 0;
  WriteRegister(DRS::laddr_control, 0, reg);

  reg = DRS::c_rst;
  WriteRegister(DRS::laddr_control, 0, reg);

  reg = 0;
  WriteRegister(DRS::laddr_control, 0, reg);
}

// InitializeDevices -----------------------------------------------------------
bool
DRS4Controller::InitializeDevices()
{
  static const std::string MyFunc = "::InitializeDevice ";

  unsigned int reg = 0;
  // Set config register
  reg = DRS::reg_reserve | DRS::reg_wsrloop | DRS::reg_pllen | DRS::reg_dmode;
  WriteConfig(reg);
  usleep(1000);

  // Set write shift register
  //  reg = DRS::reg_nocascade;
  reg = DRS::reg_cascade;
  WriteWSR(reg);
  usleep(1000);
  WriteWSR(reg);

  // Init SRIN
  usleep(1000);
  InitializeRSR();

  // Start refclk
  reg   = DRS::c_denable | DRS::c_refclk;
  WriteRegister(DRS::laddr_control, 0, reg);

  // Set DRS addr to ch8 on
  WriteDRSAddr(DRS::addr_mux_ch8);
  sleep(1);
  unsigned int pll_lock = ReadRegister(DRS::laddr_status, 1) & 0xf;
  bool ret = false;
  if(pll_lock == 0xf){
    std::cout << "#D : " << MyName << MyFunc
	      << " DRS4 are ready" << std::endl;
    ret = true;
  }else{
    std::cout << "#E : " << MyName << MyFunc
	      << " PLL are not locked. (reg:0x"
	      << std::hex << pll_lock << std::dec
	      << std::endl;
  }

  // read wait count
  static const int scc_delay = 100;
  //  static const int scc_delay = 33;
  WriteRegister(DRS::laddr_scc_delay, 0, scc_delay);
  std::cout << "#D : " << MyName << MyFunc
	    << "Read wait count : " << scc_delay << std::endl;

  return ret;
}

// WriteConfig -----------------------------------------------------------------
void
DRS4Controller::WriteConfig(unsigned int reg)
{
  unsigned int laddr    = DRS::laddr_register + DRS::laddr_drsconfig;
  unsigned int drs_addr = DRS::addr_config;
  unsigned int drs_reg  = (reg & data_mask);
  WriteRegister(laddr, drs_addr, drs_reg);

  laddr = DRS::laddr_exec_command + DRS::laddr_exec_spi;
  reg   = 0;
  WriteRegister(laddr, 0, reg);
}

// WriteWSR --------------------------------------------------------------------
void
DRS4Controller::WriteWSR(unsigned int reg)
{
  unsigned int laddr    = DRS::laddr_register + DRS::laddr_drswsr;
  unsigned int drs_addr = DRS::addr_write;
  unsigned int drs_reg  = (reg & data_mask);
  WriteRegister(laddr, drs_addr, drs_reg);

  laddr = DRS::laddr_exec_command + DRS::laddr_exec_spi;
  reg   = 0;
  WriteRegister(laddr, 0, reg);
}

// WriteWCR --------------------------------------------------------------------
void
DRS4Controller::WriteWCR(unsigned int reg)
{
  unsigned int laddr    = DRS::laddr_register + DRS::laddr_drswcr;
  unsigned int drs_addr = DRS::addr_wconfig;
  unsigned int drs_reg  = (reg & data_mask);
  WriteRegister(laddr, drs_addr, drs_reg);

  laddr = DRS::laddr_exec_command + DRS::laddr_exec_spi;
  reg   = 0;
  WriteRegister(laddr, 0, reg);
}

// WriteDRSAddr ----------------------------------------------------------------
void
DRS4Controller::WriteDRSAddr(unsigned int drs_addr)
{
  unsigned int laddr = DRS::laddr_register + DRS::laddr_drsaddr;
  WriteRegister(laddr, drs_addr, 0);
}

// InitializeRSR ---------------------------------------------------------------
void
DRS4Controller::InitializeRSR()
{
  unsigned int laddr = DRS::laddr_maxcount;
  unsigned int reg   = 0x3ff;
  WriteRegister(laddr, 0, reg);

  usleep(1000);

  laddr = DRS::laddr_control;
  reg   = DRS::c_init_srin;
  WriteRegister(laddr, 0, reg);

  usleep(1000);

  WriteDRSAddr(DRS::addr_read);

  laddr = DRS::laddr_exec_command + DRS::laddr_exec_scc;
  reg   = 0;
  WriteRegister(laddr, 0, reg);
  std::cout << "#D : DRS4Contorller::InitializeRSR" << std::endl;
}

// ReadSRout -------------------------------------------------------------------
void
DRS4Controller::ReadSrout(unsigned int *data)
{
  unsigned int laddr = DRS::laddr_spidata;
  *data = ReadRegister(laddr, NofDrs);
  std::cout << std::hex << *data << std::dec << std::endl;
}

// ReadSRoutDAQ ----------------------------------------------------------------
void
DRS4Controller::ReadSroutDAQ(unsigned int *data)
{
  unsigned int laddr = DRS::laddr_spidata+0x4;
  *data = ReadRegister(laddr, NofDrs);
  std::cout << std::hex << *data << std::dec << std::endl;
}

// ReadCellnum -----------------------------------------------------------------
void
DRS4Controller::ReadCellnum(unsigned int *data)
{
  unsigned int laddr = DRS::laddr_cellnum;
  for(int i = 0; i<NofDrs; ++i){
    *(data+i) = ReadRegister(laddr+i, 1);
    *(data+i)+= (ReadRegister(laddr+i+4, 1) << 8);
    std::cout << *(data+i) << " ";
  }
  std::cout << std::endl;
}
