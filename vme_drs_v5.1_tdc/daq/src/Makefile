### Makefile ###

COMMON_DIR	= ../../common
BIN_DIR		= ../bin

CXX		= g++
CXXFLGAS	= -Wall -fPIC
INCLUDES	= -I. -I$(COMMON_DIR)/src
LIBS		= -L$(COMMON_DIR)/lib -ldump -lz

FLAGS		= $(CXXFLGAS) $(INCLUDES)

SRCALL		= $(wildcard *.cc)
DEPENDS		= $(SRCALL:.cc=.d)

SRC		= $(filter-out %main.cc,$(SRCALL))
OBJ		= $(SRC:.cc=.o)


# tdc buffer threshold setting programs
tdc_buf		:= ch trig l1buf rofifo l2buf
tdc_buf_th_progs:= $(addprefix set_tdc_, $(addsuffix _full_threshold, $(tdc_buf)))
tdc_buf_th_opt	:= -DTDC_CH_BUF -DTDC_TRIG_BUF -DTDC_L1BUF -DTDC_ROFIFO -DTDC_L2BUF

# need additioanl "$" for variables (e.g. $@ --> $$@, $^ --> $$^)
define compile_tdc_buf_progs
$(1): $1_main.o  $(OBJ)
	$(CXX) -o $(BIN_DIR)/$$@ $$^ $(LIBS)
endef

define compile_tdc_buf_objs
$(1)_main.o: set_tdc_buffer_full_threshold_main.cc
	$(CXX) $(FLAGS) $2 -c -MMD $$^ -o $$@
endef

all:	interactive_command set_vth init_module \
	set_daqreg set_nimout set_ofslut exec_daq ofstable \
	write_reg read_reg \
	read_tdc_lut \
	set_tdc_measurement_mode \
	read_tdc_cdc_edge \
	set_tdc_reset \
	$(tdc_buf_th_progs) 

interactive_command: interactive_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ interactive_main.o $(OBJ) $(LIBS)

init_module: init_module_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ init_module_main.o $(OBJ) $(LIBS)

set_vth: set_vth_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ set_vth_main.o $(OBJ) $(LIBS)

set_daqreg: set_daqreg_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ set_daqreg_main.o $(OBJ) $(LIBS)

set_ofslut: set_ofslut_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ set_ofslut_main.o $(OBJ) $(LIBS)

set_nimout: set_nimout_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ set_nimout_main.o $(OBJ) $(LIBS)

exec_daq: exec_daq_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ exec_daq_main.o $(OBJ) $(LIBS)

ofstable: ofstable_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ ofstable_main.o $(OBJ) $(LIBS)

write_reg: write_reg_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ write_reg_main.o $(OBJ) $(LIBS)

read_reg: read_reg_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ read_reg_main.o $(OBJ) $(LIBS)

read_tdc_lut: read_tdc_lut_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

read_tdc_cdc_edge: read_tdc_cdc_edge_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

set_tdc_measurement_mode: set_tdc_measurement_mode_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

set_tdc_disable: set_tdc_disable_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

set_tdc_disable: set_tdc_enable_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)

set_tdc_reset: set_tdc_reset_main.o $(OBJ)
	$(CXX) -o $(BIN_DIR)/$@ $^ $(LIBS)


# TDC buffer config exec
$(foreach arg, $(tdc_buf_th_progs), \
	$(eval $(call compile_tdc_buf_progs, $(arg))))

# TDC buffer config obj
$(foreach arg, 1 2 3 4 5, \
	$(eval $(call compile_tdc_buf_objs, \
		$(word $(arg), $(tdc_buf_th_progs)), \
		$(word $(arg), $(tdc_buf_th_opt)))))


.cc.o:
	$(CXX) $(FLAGS) -c -MMD $< -o $@

clean:
	$(RM) $(BIN_DIR)/* *.o *.d *~

-include $(DEPENDS)
