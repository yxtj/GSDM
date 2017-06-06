#include "Option.h"
#include <iostream>
#include "../util/Util.h"

using namespace std;

Option::Option()
	:desc("Options", getScreenSize().first)
{
	using boost::program_options::value;
	using boost::program_options::bool_switch;
	desc.add_options()
		("help", "Print help messages")
		("showInfo", value<bool>(&show)->default_value(1), "Print the initializing information")
		("pathGraph", value<string>(&graphFolder), "[string] the folder/subfolder for graph files.")
		("n", value<int>(&nNode)->default_value(-1), "[integer] size of each graph (number of nodes)")
		("npi", value<int>(&nPosInd)->default_value(10), "[integer] number of positive individuals (negative means read all)")
		("nni", value<int>(&nNegInd)->default_value(10), "[integer] number of negative individuals (negative means read all)")
		("ns", value<int>(&nSnapshot)->default_value(10), "[integer] number of snapshots, non-positive means load all")
		("typePos", value<vector<int>>(&typePos)->multitoken()->default_value(vector<int>(1, 1), "1"),
			"The type(s) of positive individual")
		("typeNeg", value<vector<int>>(&typeNeg)->multitoken()->default_value(vector<int>(1, 0), "0"),
			"The type(s) of negative individual")
		("theta", value<double>(&theta), "[double] within frequency threshold to regard a motif as existed")
		("periodic", value<bool>(&periodic)->default_value(0), "only check the periodic occurrence")
		("fun", value<string>(&funName)->default_value(string("diff")), "name of the objective funtion. Support: diff, ratio")
		("alpha", value<double>(&alpha)->default_value(1.0), "[double] parameter for objective function")
		;
}


Option::~Option()
{
}

boost::program_options::options_description & Option::getDesc()
{
	return desc;
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

		sortUpPath(graphFolder);

		do {
			if(var_map.count("help")) {
				flag_help = true;
				break;
			}
			if(nNode <= 0) {
				throw invalid_argument("nNode is not given.");
			}

			if(funName != "diff" && funName != "ratio") {
				throw invalid_argument("objective function is not given or is not supported.");
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

std::string& Option::sortUpPath(std::string & path)
{
	if(!path.empty() && path.back() != '/' && path.back() != '\\')
		path.push_back('/');
	return path;
}
