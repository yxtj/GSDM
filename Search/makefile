CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
#MPI_COMPILE_FLAGS = $(shell mpic++ --showme:compile)
MPI_COMPILE_FLAGS = 
MPI_LINK_FLAGS = $(shell mpic++ --showme:link)
#MPI_LINK_FLAGS = 
SRCS = $(shell ls *.cpp)
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
EXE = Search
CXXFLAGS = -std=c++11
INC = 
LIBDIR = 
LIBS = -lcommon -lutil -lnet -lserialization -lmsgdriver
LIBS += -lboost_program_options -lboost_filesystem -lboost_system

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
	@mkdir -p ../$(DBGDIR)
	cp $(DBGEXE) ../$(DBGEXE)
	
$(DBGEXE): $(DBGOBJS)
	cd ../common; make debug
	cd ../util; make debug
	cd ../net; make debug
	cd ../msgdriver; make debug
	cd ../serialization; make debug
	$(CXX) $(CXXFLAGS) $(DBGCFLAGS) $(MPI_LINK_FLAGS) $^ $(DBGLIBDIR) $(LIBS) -o $(DBGEXE)

$(DBGDIR)/%.o: %.cpp
	@mkdir -p $(DBGDIR)
	$(CXX) -c $(CXXFLAGS) $(DBGCFLAGS) $(MPI_COMPILE_FLAGS) $(INC) -o $@ $<

#
# Release rules
#

release: ../$(RELEXE)

../$(RELEXE): $(RELEXE)
	@mkdir -p ../$(RELDIR)
	cp $(RELEXE) ../$(RELEXE)

$(RELEXE): $(RELOBJS)
	cd ../common; make release
	cd ../util; make release
	cd ../net; make release
	cd ../msgdriver; make release
	cd ../serialization; make release
	$(CXX) $(CXXFLAGS) $(RELCFLAGS) $(MPI_LINK_FLAGS) $^ $(RELLIBDIR) $(LIBS) -o $(RELEXE)
#	$(CXX) $(CXXFLAGS) $(RELCFLAGS) $^ $(RELLIBDIR) $(LIBS) -o $(RELEXE)

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
