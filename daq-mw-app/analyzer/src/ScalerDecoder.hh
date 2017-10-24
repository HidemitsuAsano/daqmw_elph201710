// -*- C++ -*-

#ifndef ScalerDecoder_hh
#define ScalerDecoder_hh

#include <string>
#include <vector>
#include <boost/iostreams/filtering_stream.hpp>


class TTree;


class ScalerDecoder {
public:
  static const unsigned int kHeaderSize = 2;
  static const unsigned int kMagicComp  = 0x504d4f43;
  static const unsigned int kNumChannel = 32;
  static const unsigned int kNumFlag    = 32;

  static const unsigned int kTrigFlagL1Accept = 31;
  static const unsigned int kTrigFlagSpillEnd = 30;
  
  struct DataContainer {
    // scaler values in time slice
    std::vector<std::vector<unsigned int> > scaler;
    std::vector<unsigned int> sum;
    // trigger time stamp
    std::vector<unsigned int> timestamp;
    std::vector<unsigned int> timediff;
    // number of spill
    unsigned int spill;

    // trigger type flag
    std::vector<unsigned int> trigger_flag;
    unsigned int trig_time;
    unsigned int trig_timediff;
    
  };
  
  ScalerDecoder();
  ~ScalerDecoder();

  void Clear();
  void Decode();
  DataContainer& GetContainer() { return m_cont; }
  
  void Initialize(const std::string& fileName);
  bool Read();
  void SetTree(TTree* spill,
               TTree* trig);

private:
  boost::iostreams::filtering_istream m_istream;
  DataContainer m_cont;
  TTree* m_spill;
  TTree* m_trig;

  std::vector<unsigned int> m_data;
  
};

#endif
