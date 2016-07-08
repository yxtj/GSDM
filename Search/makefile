CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
MPI_COMPILE_FLAGS = $(shell mpic++ --showme:compile)
MPI_LINK_FLAGS = $(shell mpic++ --showme:link)
SRCS1 = $(shell ls *.cpp)
SRCS = stdafx.cpp Search.cpp Graph.cpp Option.cpp GraphProb.cpp Motif.cpp CandidateMethodFactory.cpp CandidateMethod.cpp CandidateMthdFreq.cpp StrategyCandidate.cpp StrategyCandidatePN.cpp Subject.cpp IOfunctions.cpp
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
CXXFLAGS = -std=c++11
ifeq ($(OS),Windows_NT)
   LIBS = -lboost-program-options -lboost-filesystem -lboost-system
else
	LIBS = -lboost_program_options -lboost_filesystem -lboost_system
endif

export CXX CC CFLAGS CPPFLAGS OPROFILE

all: debug Searcher
	

release: 
	FDR=Release
	@mkdir -p Release
	@mkdir -p ../Release
	$(eval CXXFLAGS += -O2)

debug: 
	FDR=Debug
	@mkdir -p Debug
	@mkdir  -p ../Debug
	$(eval CXXFLAGS += -g)
	
Searcher: $(OBJS)
	$(CXX) $(CXXFLAGS) $(MPI_LINK_FLAGS) $(OBJS) $(LIBS) $^ -o ../$(FDR)/$@

precomp: stdafx.h
	$(CXX) -c stdafx.h -o stdafx.h.gch

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(MPI_COMPILE_FLAGS) $^ -c -o $(FDR)/$@

clean:
	rm -rf Debug
	rm -rf Release
