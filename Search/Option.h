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
	int nSnapshot;//number of snapshots for each individual
	std::vector<int> typePos, typeNeg;//type of the positive and negative individual

	std::vector<int> blacklist;

	std::string prefix, subFolderGraph;//data folder prefix and graph sub-folder
	std::string subFolderOut;//the file name prefix for output files


public:
	Option();
	~Option();

	boost::program_options::options_description& getDesc();
	void addParser(std::function<bool()>& fun);

	bool parseInput(int argc, char* argv[]);

	std::string getStrategyName() const;
	std::string getMethodName() const;
};

