#pragma once
#include <boost/program_options.hpp>
#include <string>

class Option
{
	boost::program_options::options_description desc;
public:
	bool show;

	int nNode;//number of nodes;
	int nPosInd, nNegInd;//number of +/- individuals
	int nSnapshot;//number of snapshots for each individual
	std::vector<int> typePos, typeNeg;//type of the positive and negative individual
	double theta;
	bool periodic; //whether to check the periodic occurrence
	
	std::string graphFolder; //graph folder
	//bool holdAllData; //for distributed case, whether to make all workers hold all the data
	//bool graphFolderShared; //for distributed case, when each worker holds different part of data:
							//  whether the input data folder contains all the graphs
	
	std::string funName;
public:
	Option();
	~Option();

	boost::program_options::options_description& getDesc();

	bool parseInput(int argc, char* argv[]);

private:
	static std::string& sortUpPath(std::string & path);
};

