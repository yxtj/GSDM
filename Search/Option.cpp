#include "stdafx.h"
#include "Option.h"
#include "StrategyFactory.h"
#include "CandidateMethodFactory.h"
#include "Util.h"

using namespace std;

Option::Option()
	:desc("Options", getScreenSize().first)
{
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("prefix", value<string>(&prefix)->default_value("../data"), "[string] data folder prefix")
		("prefix-graph", value<string>(&subFolderGraph)->default_value(string("graph/")), "[string] the subfolder for graph files")
		("shared-input", value<bool>(&graphFolderShared)->default_value(true), 
			"[bool] for distribution to make sure each worker loads different part of the dataset. "
			"Whether the each graph input folder contains all the data (usally true for DFS and NFS)")
		("out", value<string>(&subFolderOut)->default_value(string("out-")), "[string] the file name prefix for output files")
		("blacklist", value<vector<int>>(&blacklist)->multitoken()->default_value(vector<int>(),""),"[integer]s of individuals removed")
		("n", value<int>(&nNode)->default_value(-1), "[integer] size of each graph (number of nodes)")
		("npi", value<int>(&nPosInd)->default_value(10), "[integer] number of positive individuals (negative means read all)")
		("nni", value<int>(&nNegInd)->default_value(10), "[integer] number of negative individuals (negative means read all)")
		("ns", value<int>(&nSnapshot)->default_value(10), "[integer] number of snapshots, non-positive means load all")
		("typePos", value<vector<int>>(&typePos)->multitoken()->default_value(vector<int>(1, 1), "1"), 
			"the type(s) of positive individual")
		("typeNeg", value<vector<int>>(&typeNeg)->multitoken()->default_value(vector<int>(1, 0), "0"),
			"the type(s) of negative individual")
//		("smmin", value<int>(&sMotifMin)->default_value(2), "[integer] minimum size of a motif")
//		("smmax", value<int>(&sMotifMax)->default_value(2), "[integer] maximum size of a motif")
//		("pmi", value<double>(&pMotifInd)->default_value(0.3), "[double] the min prob. of treating "
//			"a motif as existed on a individual (num over snapshot)")
		(CandidateMethodFactory::optName.c_str(), value<vector<string>>(&mtdParam)->multitoken(), CandidateMethodFactory::getUsage().c_str())
//		("topk", value<int>(&topK)->default_value(10), "number of returned results")
//		("pmr", value<double>(&pMotifRef)->default_value(0.8), "[double] the min prob. of treating "
//			"a motif as existed all on individual (num over individual)")
		(StrategyFactory::optName.c_str(), value<vector<string>>(&stgParam)->multitoken(), StrategyFactory::getUsage().c_str())
		;
}


Option::~Option()
{
}

boost::program_options::options_description & Option::getDesc()
{
	return desc;
}

void Option::addParser(std::function<bool()>& fun)
{
	paramParser.push_back(move(fun));
}

bool Option::parseInput(int argc, char * argv[])
{
	//parse
	bool flag_help = false;
	try {
		boost::program_options::variables_map var_map;
		boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), var_map);
		boost::program_options::notify(var_map);

		if(!prefix.empty() && prefix.back()!='/' && prefix.back() != '\\') {
			prefix.push_back('/');
		}
		if(!subFolderGraph.empty() && subFolderGraph.back() != '/' && subFolderGraph.back() != '\\') {
			subFolderGraph.push_back('/');
		}
		if(!subFolderOut.empty() && subFolderOut.back() != '/' && subFolderOut.back() != '\\') {
			subFolderOut.push_back('/');
		}
		do {
			if(var_map.count("help")) {
				flag_help = true;
				break;
			}
			if(nNode <= 0) {
				throw invalid_argument("nNode is not given.");
			}

			for(auto& fun : paramParser) {
				if(!fun()) {
					throw invalid_argument("failed in parsing a complex parameter.");
				}
			}

			if(stgParam.empty() || !StrategyFactory::isValid(stgParam[0])) {
				throw invalid_argument("strategy is not given or not supported.");
			}
			if(mtdParam.empty() || !CandidateMethodFactory::isValid(mtdParam[0])) {
				throw invalid_argument("method is not given or not supported.");
			}

		} while(false);

		sort(blacklist.begin(), blacklist.end());

	} catch(std::exception& excep) {
		cerr << "error: " << excep.what() << "\n";
		flag_help = true;
	} catch(...) {
		cerr << "Exception of unknown type!\n";
		flag_help = true;
	}

	if(true == flag_help) {
		cerr << desc << endl;
		return false;
	}
	return true;
}

std::string Option::getStrategyName() const
{
	return stgParam[0];
}

std::string Option::getMethodName() const
{
	return mtdParam[0];
}
