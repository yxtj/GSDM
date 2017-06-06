CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
SRCS = $(shell ls *.cpp)
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
EXE = Evaluator
CXXFLAGS = -std=c++11
INC = 
LIBDIR = 
LIBS = -lEval -lcommon -lutil
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
	
../$(DBGEXE): $(DBGOBJS)
	@mkdir -p ../$(DBGDIR)
	$(MAKE) -C ../common debug
	$(MAKE) -C ../util debug
	$(MAKE) -C ../libEval debug
	$(CXX) $(CXXFLAGS) $(DBGCFLAGS) $^ $(DBGLIBDIR) $(LIBS) -o $@

$(DBGDIR)/%.o: %.cpp
	@mkdir -p $(DBGDIR)
	$(CXX) -c $(CXXFLAGS) $(DBGCFLAGS) $(INC) -o $@ $<

#
# Release rules
#

release: ../$(RELEXE)

../$(RELEXE): $(RELOBJS)
	@mkdir -p ../$(RELDIR)
	$(MAKE) -C ../common release
	$(MAKE) -C ../util release
	$(MAKE) -C ../libEval release
	$(CXX) $(CXXFLAGS) $(RELCFLAGS) $^ $(RELLIBDIR) $(LIBS) -o $@

$(RELDIR)/%.o: %.cpp
	@mkdir -p $(RELDIR)
	$(CXX) -c $(CXXFLAGS) $(RELCFLAGS) $(INC) -o $@ $<

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
	$(CXX) $(CXXFLAGS) -c stdafx.cpp -o stdafx.h.gch
	
clean:
	rm -rf $(DBGDIR)
	rm -rf $(RELDIR)

#Non-File Targets
.PHONY: all clean 
