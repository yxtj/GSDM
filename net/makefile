CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
MPI_COMPILE_FLAGS = $(shell mpic++ --showme:compile)
MPI_LINK_FLAGS = $(shell mpic++ --showme:link)
SRCS = $(shell ls *.cpp)
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
EXE = libnet.a
CXXFLAGS = -std=c++11
INC = 
LIBDIR = 
LIBS = -lserialization

export CXX CC CFLAGS CPPFLAGS OPROFILE

#
# Debug build settings
#
DBGDIR = Debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG
DBGLIBDIR = $(LIBDIR) -L../Debug

#
# Release build settings
#
RELDIR = Release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O2 -DNDEBUG
RELLIBDIR = $(LIBDIR) -L../Release

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

#Defauilt Make
all: release

#Debug:
debug: ../$(DEGEXE)

../$(DEGEXE): $(DBGEXE)
	cd ../serialization; make debug
	@mkdir -p ../$(DBGDIR)
	cp $(DBGEXE) ../$(DBGEXE)
	
$(DBGEXE): $(DBGOBJS)
	ar rcs $@ $^ $(LIBS) $(MPI_LINK_FLAGS)

$(DBGDIR)/%.o: %.cpp
	@mkdir -p $(DBGDIR)
	$(CXX) -c $(CXXFLAGS) $(DBGCFLAGS) $(MPI_COMPILE_FLAGS) $(INC) -o $@ $<

#
# Release rules
#

release: ../$(RELEXE)

../$(RELEXE): $(RELEXE)
	cd ../serialization; make release
	@mkdir -p ../$(RELDIR)
	cp $(RELEXE) ../$(RELEXE)

$(RELEXE): $(RELOBJS)
	ar rcs $@ $^ $(LIBS) $(MPI_LINK_FLAGS)

$(RELDIR)/%.o: %.cpp
	@mkdir -p $(RELDIR)
	$(CXX) -c $(CXXFLAGS) $(RELCFLAGS) $(MPI_COMPILE_FLAGS) $(INC) -o $@ $<

#
# common rules
#

#Make the Directories
directories:
	@mkdir -p $(DBGDIR)
	@mkdir -p ../$(DBGDIR)
	@mkdir -p $(RELDIR)
	@mkdir -p ../$(RELDIR)

stdafx.h.gch: stdafx.h
	$(CXX) $(CXXFLAGS) $(MPI_COMPILE_FLAGS) -c stdafx.h -o stdafx.h.gch
	
clean:
	rm -rf $(DBGDIR)
	rm -rf $(RELDIR)

#Non-File Targets
.PHONY: all clean 
