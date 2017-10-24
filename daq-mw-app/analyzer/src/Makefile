### Makefile ###

#COMMON_DIR	= ../../common
BIN_DIR		= ../bin

ROOT_IN		= $(shell root-config --cflags)
ROOT_LIB	= $(shell root-config --libs)

CXX		= g++
#CXXFLGAS	= -Wall -g -fPIC
CXXFLGAS	= -Wall -O3 -fPIC
INCLUDES	= -I. -I$(COMMON_DIR)/src
#LIBS		= -L$(COMMON_DIR)/lib -ldump $(ROOT_LIB) -lz
LIBS		= $(ROOT_LIB) -lz \
	-lboost_filesystem \
	-lboost_iostreams


FLAGS		= $(CXXFLGAS) $(INCLUDES) $(ROOT_IN)

SRCALL		= $(wildcard *.cc)
DEPENDS		= $(SRCALL:.cc=.d)

SRC		= $(filter-out %main.cc,$(SRCALL))
DRS_OBJ		= $(filter-out Scaler%,$(SRC:.cc=.o))


all: simple_drs_decoder \
	simple_drsqdctdc_decoder \
	simple_scaler_decoder

# all:
# 	echo $(DRS_OBJ)

simple_drs_decoder: simple_drs_decoder_main.o $(DRS_OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

simple_drsqdctdc_decoder: simple_drsqdctdc_decoder_main.o $(DRS_OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

simple_scaler_decoder: simple_scaler_decoder_main.o ScalerDecoder.o HexDump.o
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

.cc.o:
	$(CXX) $(FLAGS) -c -MMD $< -o $@

clean:
	$(RM) $(BIN_DIR)/* *.d *.o *~

-include $(DEPENDS)
