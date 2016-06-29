CXX ?= g++
CC ?= gcc
#CMAKE = cmake
#TOP = $(shell pwd)
MAKE := $(MAKE) --no-print-directory
SRCS1 = $(shell ls *.cpp)
SRCS = stdafx.cpp Search.cpp Graph.cpp Option.cpp GraphProb.cpp Motif.cpp CandidateMethodFactory.cpp CandidateMethod.cpp CandidateMthdFreq.cpp StrategyCandidate.cpp StrategyCandidatePN.cpp Subject.cpp IOfunctions.cpp
HDRS = $(SRCS:.cpp=.h) 
OBJS = $(SRCS:.cpp=.o) 
CXXFLAGS = -std=c++11

export CXX CC CFLAGS CPPFLAGS OPROFILE

all: debug Searcher
	

release: 
	FDR=Release
	@mkdir -p Release
	@mkdir -p ../Release
	CXXFLAGS = $CXXFLAGS -O2

debug: 
	FDR=Debug
	@mkdir -p Debug
	@mkdir -p ../Debug
	$(eval CXXFLAGS += -g)
	
Searcher: $(OBJS)
	cd ../$FDR
	$(echo SRCS1)
	$(echo SRCS)

clean:
	rm -rf Debug
	rm -rf Release

