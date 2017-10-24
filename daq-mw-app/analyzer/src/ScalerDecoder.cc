// -*- C++ -*-

#include <iostream>
#include <iomanip>

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>

#include <TTree.h>

#include "HexDump.hh"
#include "ScalerDecoder.hh"

//______________________________________________________________________________

struct comp_header_t
{
  unsigned int magic;
  unsigned int id;
};

struct scaler_header_t
{
  unsigned int magic;
  unsigned int nword;
};

struct data_t
{
  unsigned int timestamp;
  unsigned int scaler[ScalerDecoder::kNumChannel/4];
  unsigned int trig_flag;
  unsigned int spill;
};


//______________________________________________________________________________
ScalerDecoder::ScalerDecoder()
  : m_istream(),
    m_cont(),
    m_spill(nullptr),
    m_trig(nullptr),
    m_data()
{
  m_cont.scaler.resize(kNumChannel);
  m_cont.sum.resize(kNumChannel);
  m_cont.trigger_flag.resize(kNumFlag);
}

//______________________________________________________________________________
ScalerDecoder::~ScalerDecoder()
{
  m_istream.reset();
}

//______________________________________________________________________________
void
ScalerDecoder::Clear()
{
  for (auto&& v : m_cont.scaler) v.clear();
  for (auto&& v : m_cont.sum) v = 0;
  m_cont.timestamp.clear();
  m_cont.timediff.clear();
  m_cont.spill = 0;
  
  for (auto&& v : m_cont.trigger_flag) v = 0;
  m_cont.trig_time = 0;
  m_cont.trig_timediff = 0;
  
  m_data.clear();

}

//______________________________________________________________________________
void
ScalerDecoder::Decode()
{
  const data_t* body = reinterpret_cast<const data_t*>(&m_data[0]);
  const unsigned int* ubodyp = reinterpret_cast<const unsigned int*>(body);
  // std::cout << " body " << std::endl;
  // std::for_each(ubodyp, ubodyp+sizeof(data_t)/sizeof(unsigned int), hddaq::HexDump());
  
  unsigned int trig_flag = body->trig_flag;
  // std::cout << "#D trigger flag = " << std::hex << trig_flag << std::dec << std::endl;
  
  for (int i=0; i<kNumFlag; ++i) {
    m_cont.trigger_flag[i] = (trig_flag>>i) & 0x1;
  }

  bool l1accept = false;
  if (m_cont.trigger_flag[kTrigFlagL1Accept]==1) l1accept = true;

  bool spill_end = false;
  if (m_cont.trigger_flag[kTrigFlagSpillEnd]==1) spill_end = true;
  
  //--------------------
  m_cont.spill = body->spill & 0xffff;
  unsigned int current_time = body->timestamp;


  if (l1accept) {
    unsigned int prev_trig = m_cont.trig_time;
    m_cont.trig_time     = current_time;
    m_cont.trig_timediff = current_time - prev_trig;
    m_trig->Fill();
  } else if (!spill_end) {
    unsigned int prev_time = 0;
    if (!m_cont.timestamp.empty()) prev_time = m_cont.timestamp.back();
    m_cont.timediff.push_back(body->timestamp - prev_time);
    m_cont.timestamp.push_back(current_time);
    
    for (int i=0; i<kNumChannel/4; ++i) {
      //      std::cout << "  i = " << i << std::endl;
      
      unsigned int val32b = body->scaler[i];
      for (int jb=0; jb<4; ++jb) {
        unsigned v  = (val32b>>(8*jb)) & 0xff;
        unsigned pv = m_cont.sum[4*i+jb] & 0xff;
        unsigned dv = (v-pv) & 0xff;
        m_cont.scaler[4*i+jb].push_back(dv);
        m_cont.sum[4*i+jb] += dv;
      }
    }
  }

  if (spill_end) {
    // std::cout << "### spill end" << std::endl;
    // std::cout << "spill = " << m_cont.spill << std::endl;
    // std::cout << "time = " << m_cont.timestamp << std::endl;
    // for (int i=0; i<kNumChannel; ++i) {
    //   std::cout << " " << std::setw(4) << i
    //             << " " << std::setw(16) << sp_list[i]
    //             << std::endl;
    m_spill->Fill();
    Clear();
  }  
  
  return;
}

//______________________________________________________________________________
void
ScalerDecoder::Initialize(const std::string& fileName)
{
  m_istream.push(boost::iostreams::gzip_decompressor());
  m_istream.push(boost::iostreams::file_source(fileName, std::ios::binary));
  m_istream.auto_close();
}

//______________________________________________________________________________
bool
ScalerDecoder::Read()
{
  std::vector<unsigned int> header(kHeaderSize);
  m_istream.read(reinterpret_cast<char*>(&header[0]), sizeof(unsigned int)*kHeaderSize);
  if (m_istream.eof() || (m_istream.gcount()==0)) {
    std::cout << "end of file" << std::endl;
    return false;
  }

  int nword = 0;
  if (header[0] != kMagicComp) {
    nword = (header[1] & 0xffff);
  } else {
    m_istream.read(reinterpret_cast<char*>(&header[0]), sizeof(unsigned int)*kHeaderSize);
    nword = (header[1] & 0xffff);
  }

  m_data.resize(nword-kHeaderSize);
  m_istream.read(reinterpret_cast<char*>(&m_data[0]), sizeof(unsigned int)*m_data.size());

  return true;
}

//______________________________________________________________________________
void
ScalerDecoder::SetTree(TTree* spill,
                       TTree* trig)
{
  m_spill = spill;
  m_trig  = trig;
}
