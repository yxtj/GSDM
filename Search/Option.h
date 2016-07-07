#pragma once
#include <boost/program_options.hpp>
#include <string>

class Option
{
	boost::program_options::options_description desc;
	std::vector<std::function<bool()>> paramParser;
public:
	
	std::vector<std::string> mtdParam;
	std::vector<std::string> stgParam;

	int nNode;//number of nodes;
	int nPosInd, nNegInd;//number of +/- individuals
	int nPosMtf, nNegMtf;//number of +/- motifs
	int nSnapshot;//number of snapshots for each individual

	std::string prefix, subFolderGraph;//data folder prefix and graph sub-folder



public:
	Option();
	~Option();

	boost::program_options::options_description& getDesc();
	void addParser(std::function<bool()>& fun);

	bool parseInput(int argc, char* argv[]);

	std::string getStrategyName() const;
	std::string getMethodName() const;
};

