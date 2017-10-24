#ifndef DRS4CONTROLLER_
#define DRS4CONTROLLER_

#include"Uncopyable.hh"

class FPGAModule;

class DRS4Controller
  : private Uncopyable<DRS4Controller>
{
 public:

public:
  DRS4Controller(FPGAModule& fModule);
  virtual ~DRS4Controller();

  unsigned int ReadRegister(unsigned int local_address, int ncycle);
  int          WriteRegister(unsigned int local_address,
			     unsigned int drs_addr,
			     unsigned int reg
			     );

  void ResetDRS();
  bool InitializeDevices();
  
  void WriteConfig(unsigned int reg);
  void WriteWSR(unsigned int reg);
  void WriteWCR(unsigned int reg);
  void WriteDRSAddr(unsigned int reg);
  void InitializeRSR();
  
  void ReadSrout(unsigned int* data);
  void ReadSroutDAQ(unsigned int* data);
  void ReadCellnum(unsigned int* cellnum);

 protected:
  FPGAModule& fModule_;


};
#endif
