#include "stdafx.h"
#include "Option.h"
#include "../util/Util.h"
#include "../libEval/MTesterSingle.h"
#include "../libEval/MTesterGroup.h"

using namespace std;

Option::Option()
	:desc("Options", getScreenSize().first)
{
	// define
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("nMotif", value<int>(&nMotif)->default_value(-1), "[integer] # of motif to load, "
			"non-positive means load all)")
		("nGraph", value<int>(&nGraph)->default_value(-1), "[integer] # of graph to load, "
			"non-positive means load all)")
		("nSkipMotif",value<int>(&nSkipMotif)->default_value(0),"[integer] skip the first k valid motifs.")
		("nSkipGraph", value<int>(&nSkipGraph)->default_value(0), "[integer] skip the first k valid graph.")
		("motifPath", value<vector<string>>(&motifPath)->multitoken(), "The folder(s) for motifs (input)\n"
			"Able to accept multiple motif folders. They are treated as different test groups.")
		("motifPattern", value<string>(&motifPattern)->default_value(string("res-.*\\.txt")), 
			"The file name pattern for the motif files, in ECMAScript regular expressions syntax. "
			"USE \"\" to contain the regular expression for special characters of the shell, like *")
		("graphPath", value<string>(&graphPath), "The folder for graph data (input)")
		("graphTypePos", value<vector<int>>(&graphTypePos)->multitoken()->default_value(vector<int>(1, 1), "1"),
			"The type(s) of positive graph")
		("graphTypeNeg", value<vector<int>>(&graphTypeNeg)->multitoken()->default_value(vector<int>(1, 0), "0"),
			"The type(s) of negative graphs")

		("testMethodSingle", value<vector<string>>(&testMethodSingle)->multitoken(), MTesterSingle::usage.c_str())
		("testGroupSize", value<int>(&testGroupSize)->default_value(1), "[integer] the size of group used for group testing. "
			"values smaller than 2 means not to use group test.")
			("testMethodGroup", value<vector<string>>(&testMethodGroup)->multitoken()->default_value({"all"}, "all"),
			("Valid only if testGroupSize is greater or equal to 2. "+MTesterGroup::usage).c_str())

		("outMG", value<bool>(&flgOutMotifGroup)->default_value(false), "Output the motif IDs of each motif group.")
		("outTable", value<bool>(&flgOutTable)->default_value(false), 
			"Output a binary table of subject-motif containing and a list about the types of all subjects")
		("outSummary", value<bool>(&flgOutSmy)->default_value(true), 
			"Output a summary table about the motifs.")
		("outputFile", value<vector<string>>(&outputFile)->multitoken(), "The file(s) for outputting the result (output)\n"
			"When --motifPath takes multiple inputs, there should be multiple input here and the number should match.")
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
		if(motifPath.empty()) {
			cerr << "motif path is not given" << endl;
			flag_help = true;
			break;
		}
		if(graphPath.empty()) {
			cerr << "graph path is not given" << endl;
			flag_help = true;
			break;
		}
		if(motifPattern.empty()) {
			cerr << "motif file name pattern is not given" << endl;
			flag_help = true;
			break;
		}
		if(graphTypePos.empty()) {
			cerr << "positive graph type list is not given" << endl;
			flag_help = true;
			break;
		}
		if(graphTypeNeg.empty()) {
			cerr << "negative graph type list is not given" << endl;
			flag_help = true;
			break;
		}
		if(motifPath.size() != outputFile.size()) {
			cerr << "the number of motif folders does not match the number of output files" << endl;
			flag_help = true;
			break;
		}
		for(auto& path : motifPath) {
			sortUpPath(path);
		}
		sortUpPath(graphPath);
		mergeGraphType();

		if(!checkTestMethod()) {
			cerr << "the motif test method is not set properly." << endl;
			flag_help = true;
			break;
		}
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

bool Option::checkTestMethod()
{
	if(testMethodSingle.empty())
		return false;
	testGroupSize = max(1, testGroupSize);
	if(testGroupSize > 2 && testMethodGroup.empty())
		return false;
	return true;
}

