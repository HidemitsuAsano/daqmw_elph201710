### Makefile ###

COMMON_DIR	= ../../common
DAQ_DIR         = ../../daq
BIN_DIR		= ../bin

ROOT_IN		= $(shell root-config --cflags)
ROOT_LIB	= $(shell root-config --libs)

CXX		= g++
CXXFLGAS	= -Wall -g -fPIC
INCLUDES	= -I. -I$(COMMON_DIR)/src -I$(DAQ_DIR)/src
LIBS		= -L$(COMMON_DIR)/lib -ldump $(ROOT_LIB) -lz

FLAGS		= $(CXXFLGAS) $(INCLUDES) $(ROOT_IN)

SRCALL		= $(wildcard *.cc)
DEPENDS		= $(SRCALL:.cc=.d)

SRC		= $(filter-out %main.cc,$(SRCALL))
OBJ		= $(SRC:.cc=.o)

all: simple_decoder ofstable_maker

simple_decoder: simple_decoder_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ simple_decoder_main.o $(OBJ) $(LIBS)

ofstable_maker: ofstable_maker_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ ofstable_maker_main.o $(OBJ) $(LIBS)

.cc.o:
	$(CXX) $(FLAGS) -c -MMD $< -o $@

clean:
	$(RM) $(BIN_DIR)/* *.d *.o *~

-include $(DEPENDS)
