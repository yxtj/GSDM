CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
SRCS = $(shell ls *.cpp)
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
EXE = GraphConverter
CXXFLAGS = -std=c++11
INC = 
LIBDIR = 
MYLIB = -lcommon -lutil
LIBS = $(MYLIB)
LIBS += -lboost_program_options -lboost_filesystem -lboost_system

export CXX CC CFLAGS CPPFLAGS OPROFILE

#
# Debug build settings
#
DBGDIR = Debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG
DBGLIBDIR = $(LIBDIR) -L../$(DBGDIR)
DBGMYLIB = $(MYLIB:-l%=../$(DBGDIR)/lib%.a)

#
# Release build settings
#
RELDIR = Release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O2 -DNDEBUG
RELLIBDIR = $(LIBDIR) -L../Release
RELMYLIB = $(MYLIB:-l%=../$(RELDIR)/lib%.a)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

#Defauilt Make
all: release

#Debug:
debug: ../$(DEGEXE)

../$(DBGEXE): $(DBGOBJS) $(DBGMYLIB)
	@mkdir -p ../$(DBGDIR)
	$(CXX) $(CXXFLAGS) $(DBGCFLAGS) $^ $(DBGLIBDIR) $(LIBS) -o $@

$(DBGDIR)/%.o: %.cpp
	@mkdir -p $(DBGDIR)
	$(CXX) -c $(CXXFLAGS) $(DBGCFLAGS) $(INC) -o $@ $<

$(DBGMYLIB):
	$(MAKE) -C $(@:../$(DBGDIR)/lib%.a=../%) debug

#
# Release rules
#

release: ../$(RELEXE)

../$(RELEXE): $(RELOBJS) $(RELMYLIB)
	@mkdir -p ../$(RELDIR)
	$(CXX) $(CXXFLAGS) $(RELCFLAGS) $^ $(RELLIBDIR) $(LIBS) -o $(RELEXE)

$(RELDIR)/%.o: %.cpp
	@mkdir -p $(RELDIR)
	$(CXX) -c $(CXXFLAGS) $(RELCFLAGS) $(INC) -o $@ $<

$(RELMYLIB):
	$(MAKE) -C $(@:../$(RELDIR)/lib%.a=../%) release
	
#
# common rules
#

#Make the Directories
directories:
	@mkdir -p $(DBGDIR)
	@mkdir -p ../$(DBGDIR)
	@mkdir -p $(RELDIR)
	@mkdir -p ../$(RELDIR)

clean:
	rm -rf $(DBGDIR)
	rm -rf $(RELDIR)

#Non-File Targets
.PHONY: all clean 
