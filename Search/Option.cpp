#include "stdafx.h"
#include "Option.h"
#include "StrategyFactory.h"
#include "CandidateMethodFactory.h"

using namespace std;

Option::Option()
	:desc("Options")
{
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("prefix", value<string>(&prefix)->default_value("../data"), "[string] data folder prefix")
		("prefix-graph", value<string>(&subFolderGraph)->default_value(string("graph/")), "the subfolder for graph files")
		("npi", value<int>(&nPosInd)->default_value(10), "[integer] number of positive individuals")
		("nni", value<int>(&nNegInd)->default_value(10), "[integer] number of negative individuals")
		("npm", value<int>(&nPosMtf)->default_value(10), "[integer] number of positive motifs")
		("nnm", value<int>(&nNegMtf)->default_value(10), "[integer] number of negative motifs")
		("ns", value<int>(&nSnapshot)->default_value(10), "[integer] number of snapshots, non-positive means load all")
		("n", value<int>(&nNode)->default_value(-1), "size of each graph")
//		("smmin", value<int>(&sMotifMin)->default_value(2), "[integer] minimum size of a motif")
//		("smmax", value<int>(&sMotifMax)->default_value(2), "[integer] maximum size of a motif")
//		("pmi", value<double>(&pMotifInd)->default_value(0.3), "[double] the min prob. of treating "
//			"a motif as existed on a individual (num over snapshot)")
		(CandidateMethodFactory::optName.c_str(), value<vector<string>>(&mtdParam)->multitoken(), CandidateMethodFactory::getUsage().c_str())
//		("strategy", value<string>(&stgName)->default_value(string("Freq")), "name of the searching strategy")
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

		if(!prefix.empty() && prefix.back()!='/') {
			prefix.push_back('/');
		}
		if(!subFolderGraph.empty() && subFolderGraph.back() != '/') {
			subFolderGraph.push_back('/');
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
