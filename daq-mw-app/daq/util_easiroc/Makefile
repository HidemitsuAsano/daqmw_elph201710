#!/usr/bin/make

CXX = g++
CXXFLAGS = -I/usr/include/daqmw \
	-I../../../vme_drs_v5.1_tdc/daq/src/
#CXXFLAGS += -std=c++11
CXXFLAGS += -O3 -Wall -fPIC
LDFLAGS = -shared
LIBS = -L/usr/lib64/daqmw #-lSock

all: libSiTcpRbcp.so \
	libgetaddr.so \
	libHexDump.so


libSiTcpRbcp.so: SiTcpRbcp.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

libgetaddr.so: getaddr.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

libHexDump.so: HexDump.o
	$(CXX) $(LDFLAGS) $^ -o $@



%.o: %.cc
	$(CXX) $(CXXFLAGS)  -c -MMD $< -o $@

clean:
	rm -rf *.o *~ *.so *.d
