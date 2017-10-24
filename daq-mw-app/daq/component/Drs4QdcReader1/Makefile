COMP_NAME = Drs4QdcReader1

vme_drs_tool_dir=../../../..
vme_drs_tool_ver=vme_drs_v5.1_tdc

all: $(COMP_NAME)Comp

PWD = $(shell pwd)

SRCS += $(COMP_NAME).cpp
SRCS += $(COMP_NAME)Comp.cpp

CPPFLAGS += -I../../util \
	-I$(vme_drs_tool_dir)/$(vme_drs_tool_ver)/daq/src \
	-I$(vme_drs_tool_dir)/$(vme_drs_tool_ver)/common/src

CXXFLAGS += -std=c++11

LDLIBS += -L/usr/lib64/daqmw -lSock \
	-L$(PWD)/../../util -lSiTcpRbcp -Wl,-rpath=$(PWD)/../../util \
	-L$(PWD)/../../util -lgetaddr -Wl,-rpath=$(PWD)/../../util   \
	-L$(PWD)/../../util -lHexDump -Wl,-rpath=$(PWD)/../../util


# sample install target
#
# MODE = 0755
# BINDIR = /tmp/mybinary
#
# install: $(COMP_NAME)Comp
#	mkdir -p $(BINDIR)
#	install -m $(MODE) $(COMP_NAME)Comp $(BINDIR)

include /usr/share/daqmw/mk/comp.mk

