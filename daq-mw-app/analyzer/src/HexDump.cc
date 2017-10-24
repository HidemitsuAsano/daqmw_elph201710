// -*- C++ -*-

// Author: Tomonori Takahashi

#include "HexDump.hh"

#include <iostream>

namespace hddaq
{

//______________________________________________________________________________
HexDump::HexDump()
  : m_count(0)
{
  std::cout << std::hex;
}

//______________________________________________________________________________
HexDump::~HexDump()
{
  std::cout << std::dec << std::endl;
}

//______________________________________________________________________________
void 
HexDump::operator()(unsigned int data)
{
  if (0 == m_count)
    {
      std::cout.width(8);
      std::cout.fill('0');
      std::cout << m_count;
      std::cout << " : ";
    }
  std::cout.width(8);
  std::cout.fill('0');
  std::cout << data << " ";
  ++m_count;
  if (0 == m_count%8) 
    {
      std::cout << "\n";
      std::cout.width(8);
      std::cout.fill('0');
      std::cout <<  m_count;
      std::cout << " : ";
    }
  return;
}


//______________________________________________________________________________
void 
HexDump::operator()(unsigned short data)
{
  if (0 == m_count)
    {
      std::cout.width(8);
      std::cout.fill('0');
      std::cout << m_count;
      std::cout << " : ";
    }
  std::cout.width(4);
  std::cout.fill('0');
  std::cout << data << " ";
  ++m_count;
  if (0 == m_count%8) 
    {
      std::cout << "\n";
      std::cout.width(8);
      std::cout.fill('0');
      std::cout <<  m_count;
      std::cout << " : ";
    }
  return;
}

}
