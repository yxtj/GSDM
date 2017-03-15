#include "stdafx.h"
#include "Option.h"
#include <boost/program_options.hpp>
#include "../util/Util.h"
#include "../eval/MTesterSingle.h"
#include "../eval/MTesterGroup.h"
#include "../eval/GroupGenerator.h"

using namespace std;

struct Option::implDesc {
	boost::program_options::options_description desc;
};

Option::Option()
	:pimpl(new implDesc{ boost::program_options::options_description("Options", getScreenSize().first) })
{
	// define
	using boost::program_options::value;
	pimpl->desc.add_options()
		("help", "Print help messages")
		("showInfo", value<bool>(&show)->default_value(1), "Print the initializing information")
		("nSubject", value<int>(&nSubject)->default_value(0), "[integer] # of graph to load, "
			"(non-positive means load all).")
		("nSnapshot", value<int>(&nSnapshot)->default_value(0), "[integer] # of graph to load, "
			"(non-positive means load all).")
		("nMotif", value<int>(&nMotif)->default_value(0), "[integer] # of motif to load, "
			"(non-positive means load all).")
		("motifPath", value<string>(&pathMotif), "The folder for motifs (input)\n")
		("motifPattern", value<string>(&motifPattern)->default_value(string("res-.*\\.txt")), 
			"The file name pattern for the motif files, in ECMAScript regular expressions syntax. "
			"USE \"\" to contain the regular expression for special characters of the shell, like *")
		("graphPath", value<string>(&pathGraph), "The folder for graph data (input).")
		("typePos", value<vector<int>>(&typePos)->multitoken()->default_value(vector<int>(1, 1), "1"),
			"The type(s) of positive subjects.")
		("typeNeg", value<vector<int>>(&typeNeg)->multitoken()->default_value(vector<int>(1, 0), "0"),
			"The type(s) of negative subjects.")
		("sortSubByType", value<bool>(&flgSortSubByType)->default_value(1), "[flag] sort the subjects by type")
		("outputPath", value<string>(&pathOutput), "The file for outputting the result (output).\n")
		;
}

Option::~Option() {
	delete pimpl;
}

bool Option::parseInput(int argc, char* argv[]) {
	//parse
	bool flag_help = false;
	boost::program_options::variables_map var_map;
	try {
		boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, pimpl->desc), var_map);
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
		if(pathMotif.empty()) {
			cerr << "motif path is not given" << endl;
			flag_help = true;
			break;
		}
		if(pathGraph.empty()) {
			cerr << "graph path is not given" << endl;
			flag_help = true;
			break;
		}
		if(motifPattern.empty()) {
			cerr << "motif file name pattern is not given" << endl;
			flag_help = true;
			break;
		}
		if(typePos.empty()) {
			cerr << "positive graph type list is not given" << endl;
			flag_help = true;
			break;
		}
		if(typeNeg.empty()) {
			cerr << "negative graph type list is not given" << endl;
			flag_help = true;
			break;
		}
		sortUpPath(pathMotif);
		sortUpPath(pathGraph);
		sortUpPath(pathOutput);
		if(!mergeGraphType()) {
			cerr << "poseitive type and negative type overlap with each other" << endl;
			flag_help = true;
			break;
		}

		break;
	}

	if(true == flag_help) {
		cerr << pimpl->desc << endl;
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

bool Option::mergeGraphType()
{
	typeAll.clear();
	copy(typePos.begin(), typePos.end(), back_inserter(typeAll));
	copy(typeNeg.begin(), typeNeg.end(), back_inserter(typeAll));
	sort(typeAll.begin(), typeAll.end());
	auto it = unique(typeAll.begin(), typeAll.end());
	return it == typeAll.end();
}

