#ifndef DAQFUNCS_
#define DAQFUNCS_

#include"FPGAModule.hh"
#include"EventBuilder.hh"
#include"RegisterMap.hh"
#include"rbcp.h"
#include"network.hh"
#include"errno.h"
#include"HexDump.hh"

#include<iostream>
#include<cstdio>
#include<csignal>
#include<algorithm>
#include<functional>
#include<vector>

void UserStop_FromCtrlC(int signal);
void daq(char* ip, rbcp_header *rbcpHeader,
	 int run_no=1, int event_no=10);
int  ConnectSocket(const char* ip);
int  Event_Cycle(int sock, unsigned int* buffer, int header_size);
bool Event_Cycle_tdc(int sock, std::vector<unsigned int>& buffer);

int  receive(int sock, char* data_buf, unsigned int length);

#endif
