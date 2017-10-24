// -+- C++ -*-

#include <iostream>
#include <iomanip>
#include <vector>
//#include <cstring>

#include "SiTcpRbcp.hh"


const int kMaxBuf = 1024;

//______________________________________________________________________________
SiTcpRbcp::SiTcpRbcp(const std::string& host,
                     int port)
  : m_sock(host, port),
    m_header(),
    m_verbose(0)
{
  m_sock.connectUDP(); 
}

//______________________________________________________________________________
SiTcpRbcp::~SiTcpRbcp()
{
}

//______________________________________________________________________________
int
SiTcpRbcp::read(unsigned int offsetAddress,
                unsigned char* buffer,
                int length)
{
  rbcp_header header;
  header.type    = 0xFF;
  header.command = 0xC0; // read
  header.id      = 0;
  header.length  = length;
  header.address = htonl(offsetAddress);
  
  if(m_verbose){
   std::cout << "#D SiTcpRbcP::read() address = " << std::hex << offsetAddress << std::dec
             << " length = " << length << std::endl;
  }
  std::vector<unsigned char> packet(reinterpret_cast<unsigned char*>(&header),
                                    reinterpret_cast<unsigned char*>(&header)+sizeof(rbcp_header));
  // std::cout << "#D size = " << packet.size() << std::endl;
  // for (auto x : packet) {
  //   std::cout << std::hex << static_cast<unsigned int>(x) << std::dec << " ";
  // }
  // std::cout << std::endl;
  
  int status = m_sock.writeTo(&(packet[0]), packet.size());
  if (status < 0) {
    std::cerr << "SiTcpRbcp::read(): ???" << std::endl;
    return status;
  }
  packet.clear();
  packet.resize(sizeof(rbcp_header)+length, 0);
  
  status = m_sock.readFrom(&(packet[0]), packet.size());
  if (status < 0) {
    std::cerr << "SiTcpRbcp::read(): error" << std::endl;
    return status;
  }
  std::copy(packet.begin()+sizeof(rbcp_header), packet.end(), buffer);
  //  std::memcpy(buffer, &(packet[sizeof(rbcp_header)]), length);

  // for (auto x : packet) {
  //   std::cout << std::hex << static_cast<unsigned int>(x) << std::dec << " ";
  // }
  // std::cout << std::endl;
  
  return status;
}

//______________________________________________________________________________
int
SiTcpRbcp::write(unsigned int offsetAddress,
                unsigned char* buffer,
                int length)
{
  rbcp_header header;
  header.type    = 0xFF;
  header.command = 0x80; // write
  header.id      = 1;
  header.length  = length;
  header.address = htonl(offsetAddress);

  if(m_verbose){
    std::cout << "#D SiTcpRbcp::write() address = " << std::hex << offsetAddress << std::dec << std::endl;
    std::cout << "#D buf = " << std::hex << static_cast<unsigned int>(buffer[0]) << std::dec << std::endl;
  }

  std::vector<unsigned char> packet(reinterpret_cast<unsigned char*>(&header),
                                    reinterpret_cast<unsigned char*>(&header)+sizeof(rbcp_header));
  std::copy(buffer, buffer+length, std::back_inserter(packet));
  
  int status = m_sock.writeTo(&(packet[0]), packet.size());
  if (status < 0) {
    std::cerr << "SiTcpRbcp::write(): ???" << std::endl;
    return status;
  }
  packet.clear();
  packet.resize(sizeof(rbcp_header));
  status = m_sock.readFrom(&(packet[0]), packet.size());
  if (status < 0) {
    std::cerr << "SiTcpRbcp::write(): error" << std::endl;
    return status;
  }

  return status;
}
