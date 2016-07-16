#pragma once
#include <string>

class Option
{
public:
	int nPosInd, nNegInd;//number of +/- individuals
	int nSnapshot;//number of snapshots for each individual
	int nPosMtf, nNegMtf;//number of +/- motifs
	int sMotifMin, sMotifMax;//min and max size of a motif
	int nNode;//number of nodes;
	double degAlpha;//the alpha value of the degree power law distribution
	double pMotif;//mean probability of the motifs, should not be too small
	
	std::string prefix, subFolderGraph;

	unsigned seed;
public:
	Option();
	~Option();
	bool parseInput(int argc, char *argv[]);
	
private:
};

