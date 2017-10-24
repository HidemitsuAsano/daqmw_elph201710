// -*- C++ -*-

// Author: Tomonori Takahashi 

#ifndef HDDAQ__HEX_DUMP_H
#define HDDAQ__HEX_DUMP_H

#include <functional>

namespace hddaq 
{
  
  class HexDump
    : public std::unary_function<unsigned int, void> 
  {
    
  private:
    int m_count;
    
  public:
     HexDump();
    ~HexDump();
    
    void operator() (unsigned int data);
    void operator() (unsigned short data);

  };

}
#endif
