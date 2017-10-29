COMP_NAME = HulScalerReader1

all: $(COMP_NAME)Comp

PWD = $(shell pwd)

SRCS += $(COMP_NAME).cpp
SRCS += $(COMP_NAME)Comp.cpp

#$(COMP_NAME)Comp: $(COMP_NAME)Comp.o

CPPFLAGS += -Istandalone/src

LDLIBS += -L$(DAQMW_LIB_DIR) -lSock \
  -L$(PWD)/standalone/lib -lBitDump -Wl,-rpath=$(PWD)/standalone/lib \
	-L$(PWD)/standalone/lib -lUDPRBCP -Wl,-rpath=$(PWD)/standalone/lib \
	-L$(PWD)/standalone/lib -lFPGAModule -Wl,-rpath=$(PWD)/standalone/lib

# Socket library
#LDLIBS += -L$(DAQMW_LIB_DIR) -lSock

# sample install target
#
# MODE = 0755
# BINDIR = /home/daq/bin
#
# install: $(COMP_NAME)Comp
#	mkdir -p $(BINDIR)
#	install -m $(MODE) $(COMP_NAME)Comp $(BINDIR)

include /usr/share/daqmw/mk/comp.mk
