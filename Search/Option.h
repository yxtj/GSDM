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

	bool holdAllData; //for distributed case, whether to make all workers hold all the data
	bool graphFolderShared; //for distributed case, when each worker holds different part of data:
							//  whether the input data folder contains all the graphs
	std::string prefix, graphFolder;//data folder prefix and graph sub-folder
	std::string outFolder;//the file name prefix for output files

public:
	Option();
	~Option();

	boost::program_options::options_description& getDesc();
	void addParser(std::function<bool()>& fun);

	bool parseInput(int argc, char* argv[]);

	std::string getStrategyName() const;
	std::string getMethodName() const;
private:
	static std::string& sortUpPath(std::string & path);
	std::string& processSubPath(std::string& path);
};

