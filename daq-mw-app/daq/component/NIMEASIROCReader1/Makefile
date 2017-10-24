COMP_NAME = NIMEASIROCReader1

#LDADD = $(top_builddir)/src/libyaml.la
all: $(COMP_NAME)Comp

PWD = $(shell pwd)

SRCS += $(COMP_NAME).cpp
SRCS += $(COMP_NAME)Comp.cpp
#SRCS += easirocdevice.o easiroc_config.o FileManager.o ParameterManager.o StringManager.o

CPPFLAGS += -I../../util_easiroc
# Socket library
LDLIBS += -L$(DAQMW_LIB_DIR) -lSock\
	-L$(PWD)/../../util_easiroc -lSiTcpRbcp -Wl,-rpath=$(PWD)/../../util_easiroc# \
#	-L$(PWD)/../../util -lgetaddr -Wl,-rpath=$(PWD)/../../util   \
#	-L$(PWD)/../../util -lHexDump -Wl,-rpath=$(PWD)/../../util

# sample install target
#
# MODE = 0755
# BINDIR = /home/daq/bin
#
# install: $(COMP_NAME)Comp
#	mkdir -p $(BINDIR)
#	install -m $(MODE) $(COMP_NAME)Comp $(BINDIR)

include /usr/share/daqmw/mk/comp.mk
