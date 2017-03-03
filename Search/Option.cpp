#include "stdafx.h"
#include "Option.h"
#include "StrategyFactory.h"
#include <boost/program_options.hpp>
#include "../util/Util.h"

using namespace std;

struct Option::Impl {
	boost::program_options::options_description desc;
};

Option::Option()
	:pimpl(new Impl{ boost::program_options::options_description("Options", getScreenSize().first) })
{
	using boost::program_options::value;
	using boost::program_options::bool_switch;
	pimpl->desc.add_options()
		("help", "Print help messages")
		("showInfo", value<bool>(&show)->default_value(1), "Print the initializing information")
		("prefix", value<string>(&prefix)->default_value("../data"), "[string] data folder prefix")
		("prefix-graph", value<string>(&graphFolder)->default_value(string("graph/")),
			"[string] the folder/subfolder for graph files. "
			"If the option starts with a '/', it is a absolute path. Otherwise it is a subfolder of <prefix>")
		("all-data", bool_switch(&holdAllData)->default_value(false),
			"[flag] for distributed case: whether to make each worker process all the input data.")
		("shared-input", bool_switch(&graphFolderShared)->default_value(false),
			"[flag] whether the input folder of each worker holds all the data (the folder is shared, usually true for DFS and NFS). "
			"Used for distributed case where different worker shoudld process different piece of the data.")
		("out", value<string>(&outFolder)->default_value(string("out-")), "[string] the file name prefix for output files")
		("blacklist", value<vector<int>>(&blacklist)->multitoken()->default_value(vector<int>(),""),"[integer]s of individuals removed")
		("n", value<int>(&nNode)->default_value(-1), "[integer] size of each graph (number of nodes)")
		("npi", value<int>(&nPosInd)->default_value(10), "[integer] number of positive individuals (negative means read all)")
		("nni", value<int>(&nNegInd)->default_value(10), "[integer] number of negative individuals (negative means read all)")
		("ns", value<int>(&nSnapshot)->default_value(10), "[integer] number of snapshots, non-positive means load all")
		("typePos", value<vector<int>>(&typePos)->multitoken()->default_value(vector<int>(1, 1), "1"), 
			"The type(s) of positive individual")
		("typeNeg", value<vector<int>>(&typeNeg)->multitoken()->default_value(vector<int>(1, 0), "0"),
			"The type(s) of negative individual")
//		("pmi", value<double>(&pMotifInd)->default_value(0.3), "[double] the min prob. of treating "
//			"a motif as existed on a individual (num over snapshot)")
//		("topk", value<int>(&topK)->default_value(10), "number of returned results")
//		("pmr", value<double>(&pMotifRef)->default_value(0.8), "[double] the min prob. of treating "
//			"a motif as existed all on individual (num over individual)")
		(StrategyFactory::optName.c_str(), value<vector<string>>(&stgParam)->multitoken(), StrategyFactory::getUsage().c_str())
		;
}


Option::~Option()
{
	delete pimpl;
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
			boost::program_options::parse_command_line(argc, argv, pimpl->desc), var_map);
		boost::program_options::notify(var_map);

		sortUpPath(prefix);
		processSubPath(graphFolder);
		processSubPath(outFolder);

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
		cerr << pimpl->desc << endl;
		return false;
	}
	return true;
}

std::string Option::getStrategyName() const
{
	return stgParam[0];
}

std::string& Option::sortUpPath(std::string & path)
{
	if(!path.empty() && path.back() != '/' && path.back() != '\\')
		path.push_back('/');
	return path;
}

std::string & Option::processSubPath(std::string & path)
{
	sortUpPath(path);
	if(path.front() != '/') {
		path = prefix + path;
	}
	return path;
}
