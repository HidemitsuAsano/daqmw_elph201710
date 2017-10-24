#ifndef EventBuilderH_
#define EventBuilderH_

#include<fstream>
#include<vector>
#include"Uncopyable.hh"

class FPGAModule;

class EventBuilder
  : private Uncopyable<EventBuilder>
{
public:
  static const int full_header_size  = 8;
  static const int short_header_size = 4;

  EventBuilder(FPGAModule& fModule);
  virtual ~EventBuilder();

  unsigned int ReadRegister(unsigned int local_address, int ncycle);
  int          WriteRegister(unsigned int local_address,
			     unsigned int reg
			     );

  int  GetHeaderSize();
  
  void ResetFIFO();

  bool isDataReady();
  void WriteBlCor1Table(const char* dir = "./blcor");
  void WriteBlCor2Table(const char* dir = "./blcor");
  void WriteOfsTable(const char* dir = "./pede");
  void WriteNfTh(std::vector<unsigned int>& cont_high,
		 std::vector<unsigned int>& cont_low
		 );
  void WriteZsTh(std::vector<unsigned int>& cont);

 protected:
  FPGAModule& fModule_;

};
#endif
