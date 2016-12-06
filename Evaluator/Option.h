#pragma once
#include <string>
#include <vector>
#include <boost/program_options.hpp>

class Option
{
	boost::program_options::options_description desc;
public:
	int nGraph;
	int nMotif; // number of subject need to be loaded from the dataset
	int nSkipGraph, nSkipMotif; // skip some of the first items(graph,motif), to efficiently restart after failure

	// path for input/output data folder:
	std::vector<std::string> motifPath;
	std::string motifPattern;
	std::string graphPath;
	std::vector<int> graphTypePos, graphTypeNeg, graphTypes;

	bool flgOutMotifGroup;
	bool flgOutTable, flgOutSmy;
	std::vector<std::string> outputFile;

	std::vector<std::string> testMethodSingle;
	std::vector<std::string> testMethodGroup;
	int testGroupSize;

public:
	Option();
	boost::program_options::options_description& getDesc();

	bool parseInput(int argc, char *argv[]);
private:
	std::string& sortUpPath(std::string& path);
	void mergeGraphType();
	bool checkTestMethod();
};

