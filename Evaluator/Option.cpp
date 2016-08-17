#include "stdafx.h"
#include "Option.h"
#include "Util.h"
#include "MotifTester.h"

using namespace std;

Option::Option()
	:desc("Options", getScreenSize().first)
{
	// define
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("nMotif", value<int>(&nMotif)->default_value(-1), "[integer] # of motif to load"
			"non-positive means load all)")
		("nGraph", value<int>(&nGraph)->default_value(-1), "[integer] # of graph to load"
			"non-positive means load all)")
		("nSkipMotif",value<int>(&nSkipMotif)->default_value(0),"[integer] skip the first k valid motifs.")
		("nSkipGraph", value<int>(&nSkipGraph)->default_value(0), "[integer] skip the first k valid graph.")
		("motifPath", value<string>(&motifPath), "the folder for motifs (input)")
		("motifPattern", value<string>(&motifPattern)->default_value(string("res-.*\\.txt")), 
			"the file name pattern for the motif files, in ECMAScript regular expressions syntax. "
			"USE \"\" to contain the regular expression for special characters of the shell, like *")
		("graphPath", value<string>(&graphPath), "the folder for graph data (input)")
		("graphTypePos", value<vector<int>>(&graphTypePos)->multitoken(), "the type(s) of positive graph")
		("graphTypeNeg", value<vector<int>>(&graphTypeNeg)->multitoken()->default_value(vector<int>(1, 0), "0"),
			"the type(s) of negative graphs")
		//("thrsldMotifSub", value<double>(&thrsldMotifSub)->default_value(0.4, "0.4"), 
		//	"the portion threshold for regarding a motif as existence on a subject")
		(MotifTester::name.c_str(), value<vector<string>>(&motifTestMethod)->multitoken(), MotifTester::usage.c_str())
		("logFile", value<string>(&logFile), "the file for detailed motif checking log (output)")
		("outputFile", value<string>(&outputFile), "the file for outputting the result (output)")
		;
}

boost::program_options::options_description & Option::getDesc()
{
	return desc;
}


bool Option::parseInput(int argc, char* argv[]) {
	//parse
	bool flag_help = false;
	boost::program_options::variables_map var_map;
	try {
		boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), var_map);
		boost::program_options::notify(var_map);

		if(var_map.count("help")) {
			flag_help = true;
		}
	} catch(std::exception& excep) {
		cerr << "error: " << excep.what() << "\n";
		flag_help = true;
	} catch(...) {
		cerr << "Exception of unknown type!\n";
		flag_help = true;
	}

	while(!flag_help) { // technique for condition checking
		sortUpPath(motifPath);
		sortUpPath(graphPath);
		mergeGraphType();
		break;
	}

	if(true == flag_help) {
		cerr << desc << endl;
		return false;
	}
	return true;
}

std::string& Option::sortUpPath(std::string & path)
{
	if(!path.empty() && path.back() != '/')
		path.push_back('/');
	return path;
}

void Option::mergeGraphType()
{
	graphTypes.clear();
	graphTypes.reserve(graphTypePos.size() + graphTypeNeg.size());
	graphTypes = graphTypePos;
	for(auto& v : graphTypeNeg)
		graphTypes.push_back(v);
	sort(graphTypes.begin(), graphTypes.end());
}

