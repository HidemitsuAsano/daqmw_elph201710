### Makefile ###

LIB_DIR		= ../lib

CXX		= g++
CXXFLGAS	= -Wall -g -fPIC
INCLUDES	= -I.

FLAGS		= $(CXXFLGAS) $(INCLUDES)

SRC		= $(wildcard *.cc)
OBJ		= $(SRC:.cc=.o)
DEPENDS		= $(SRC:.cc=.d)

libdump.a: $(OBJ)
	ar cr $(LIB_DIR)/$@ $(OBJ)

.cc.o:
	$(CXX) $(FLAGS) -c -MMD $< -o $@

clean:
	rm -f $(LIB_DIR)/* *.d *.o *~

-include $(DEPENDS)

