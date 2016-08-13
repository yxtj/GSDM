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
	std::string motifPath, motifPattern;
	std::string graphPath;
	std::vector<int> graphTypes;

	std::string logFile;
	std::string outputFile;

	double thrsldMotifSub; // the portion threshold for regarding a motif as existence on a subject

public:
	Option();
	boost::program_options::options_description& getDesc();

	bool parseInput(int argc, char *argv[]);
private:
	std::string& sortUpPath(std::string& path);
};

