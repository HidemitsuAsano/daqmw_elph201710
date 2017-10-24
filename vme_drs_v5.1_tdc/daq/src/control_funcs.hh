#ifndef CONTROLFUNCS_
#define CONTROLFUNCS_

#include"CommandMan.hh"
#include"UDPRBCP.hh"
#include"FPGAModule.hh"
#include"LTC2656_12_SPI.hh"
#include"AD9637_SPI.hh"
#include"DRS4Controller.hh"
#include"EventBuilder.hh"
#include"RegisterMap.hh"
#include"rbcp.h"
#include"network.hh"
#include"errno.h"

#include"TDCController.hh"

#include<iostream>
#include<cstdio>


void module_initialize(char* ip, rbcp_header* rbcpHeader, const std::string& file_name="");
void interactive_rbcp(char* ip, rbcp_header* rbcpHeader);
void dac_spi(char* ip, rbcp_header *rbcpHeader, const char* filename = "./register/LTC2656-12-default.txt");
void adc_spi(char* ip, rbcp_header *rbcpHeader);
void drs_init(char* ip, rbcp_header *rbcpHeader);
void lut_load(char* ip, rbcp_header *rbcpHeader, const char* filename=0);
void blcor1_load(char* ip, rbcp_header *rbcpHeader);
void blcor2_load(char* ip, rbcp_header *rbcpHeader);
void nimout(char* ip, rbcp_header *rbcpHeader,
	    int out1=OMN::reg_or16, int out2=OMN::reg_busy);
void system_reg(char* ip, rbcp_header *rbcpHeader);
void daq_reg(char* ip, rbcp_header *rbcpHeader);
void noise_filter_th(char* ip, rbcp_header *rbcpHeader);
void zero_suppress_th(char* ip, rbcp_header *rbcpHeader);
void debug(char* ip, rbcp_header *rbcpHeader);

void set_read_count(char* ip, rbcp_header *rbcpHeader, int count);
void set_clktrig_freq(char* ip, rbcp_header *rbcpHeader, unsigned int reg);
void set_cascade_th(char* ip, rbcp_header *rbcpHeader,
                    unsigned int th1=765, unsigned int th2=1021);


void tdc_peek_lut(char* ip, rbcp_header *rbcpHeader, int ch, int nbins);
void tdc_start_calibration(char* ip, rbcp_header *rbcpHeader);

#endif
