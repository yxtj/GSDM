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
EXE = Search
CXXFLAGS = -std=c++11
INC = 
ifeq ($(OS),Windows_NT)
  LIBS = -lboost-program-options -lboost-filesystem -lboost-system
else
  LIBS = -lboost_program_options -lboost_filesystem -lboost_system
endif

export CXX CC CFLAGS CPPFLAGS OPROFILE

#
# Debug build settings
#
DBGDIR = Debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#
RELDIR = Release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O2 -DNDEBUG

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

#Defauilt Make
all: release

#Debug:
debug: $(DBGEXE)
	@mkdir -p ../$(DBGDIR)
	cp $(DBGEXE) ../$(DBGEXE)
	
$(DBGEXE): $(DBGOBJS)
	$(CXX) $(CXXFLAGS) $(DBGCFLAGS) $(MPI_LINK_FLAGS) $(LIBS) -o $(DBGEXE) $^

$(DBGDIR)/%.o: %.cpp
	@mkdir -p $(DBGDIR)
	$(CXX) -c $(CXXFLAGS) $(DBGCFLAGS) $(MPI_COMPILE_FLAGS) $(INC) -o $@ $<

#
# Release rules
#

release: $(RELEXE)
	@mkdir -p ../$(RELDIR)
	cp $(RELEXE) ../$(RELEXE)

$(RELEXE): $(RELOBJS)
	$(CXX) $(CXXFLAGS) $(RELCFLAGS) $(MPI_LINK_FLAGS) $(LIBS) -o $(RELEXE) $^

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
