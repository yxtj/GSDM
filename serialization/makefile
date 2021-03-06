CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
SRCS = $(shell ls *.cpp)
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
EXE = libserialization.a
CXXFLAGS = -std=c++11
INC = 
LIBDIR = 
LIBS = -lcommon

export CXX CC CFLAGS CPPFLAGS OPROFILE

#
# Debug build settings
#
DBGDIR = Debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS = -g -O0 -DDEBUG
DBGLIBDIR = $(LIBDIR) -L../$(DBGDIR)
DBGLIBS= $(foreach LIB,$(LIBS),../$(DBGDIR)/lib$(LIB:-l%=%.a))

#
# Release build settings
#
RELDIR = Release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS = -O2 -DNDEBUG
RELLIBDIR = $(LIBDIR) -L../$(RELDIR)
DBGLIBS= $(foreach LIB,$(LIBS),../$(RELDIR)/lib$(LIB:-l%=%.a))

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
	ar rcs $@ $^ $(DBGLIBS)

$(DBGDIR)/%.o: %.cpp
	@mkdir -p $(DBGDIR)
	$(CXX) -c $(CXXFLAGS) $(DBGCFLAGS) $(INC) -o $@ $<

#
# Release rules
#

release: ../$(RELEXE)

../$(RELEXE): $(RELEXE)
	cd ../common; make release
	@mkdir -p ../$(RELDIR)
	cp $(RELEXE) ../$(RELEXE)

$(RELEXE): $(RELOBJS)
	cd ../common; make release
	ar rcs $@ $^ $(RELLIBS)

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

clean:
	rm -rf $(DBGDIR)
	rm -rf $(RELDIR)

#Non-File Targets
.PHONY: all clean 
