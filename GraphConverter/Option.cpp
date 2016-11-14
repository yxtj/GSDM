#include "stdafx.h"
#include "Option.h"
#include "../util/Util.h"

using namespace std;

Option::Option()
	:desc("Options", getScreenSize().first)
{
	// define
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("dataset", value<string>(&dataset), "Specific which dataset is going to be used (ADHD, ABIDE).")
		("nSkip",value<int>(&nSkip)->default_value(0),"Skip the first nSkip items(subject/corr). Used for failure recovery")
		("nSubject,n", value<int>(&nSubject)->default_value(-1), "# of items(subject/corr) to load from dataset "
			"(non-positive means load all)")
		("tcPath", value<string>(&tcPath), "The folder for time course data (input)")
		("phenoPath", value<string>(&phenoPath), "The folder for the phenotyic file (input). If it is empty, tcPath will be used.")
		("corrPath", value<string>(&corrPath), "The folder for correlation data "
			"(if --tcPath is not given, this is an input folder. otherwise this is used for output)")
		("graphPath", value<string>(&graphPath), "The folder for graph data (output)")
		("tc-qc", value<string>(&tcQualityControl)->default_value(string("all")), 
			"How to use the Quality Control fields to filter the subjects,\n"
			"supports: none (not use), any (fulfill any QC), all (fulfill all QC)")
		("corr-method", value<string>(&corrMethod)->default_value(string("pearson")),
			"The method for calculating correlation between ROI,\n"
			"supports: pearson, spearman, mutialinfo")
		("graph-method", value<vector<string>>(&graphParam)->multitoken()->default_value(vector<string>{"ge", "0.8"}, "ge 0.8"),
			"The methods and parameters for determining connectivity,\n"
			"supports: gt <th>, ge <th>, lt <th>, le <th>, between <thLow> <thUp>, outside <thLow> <thUp>\n"
			"between uses [thLow,thUp) range. outside is the oppsite to between.\n"
			"FOR NEGATIVE NUMBER, USE n0.8 instead of -0.8, because \"-x\" is regarded as an option")
		("com-graph", value<int>(&comGraphLevel)->default_value(0), "The compression level for outputting graphs.\n"
			"The larger the number is, the smaller the output size is. 0 -> normal text, 1 -> binary, 2~n -> compressed whith different level.")
		;
	cutp.reg(*this);
}

boost::program_options::options_description & Option::getDesc()
{
	return desc;
}

void Option::addParser(std::function<bool()>& fun)
{
	paramParser.push_back(move(fun));
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
		if(!checkIOLogic()) {
			flag_help = true;
			break;
		}
		try {
			for(auto& fun : paramParser) {
				if(!fun()) {
					flag_help = true;
					break;
				}
			}
		} catch(exception& e) {
			cerr << e.what() << endl;
			flag_help = true;
			break;
		}

		if(phenoPath.empty()) {
			phenoPath = tcPath;
		}
		sortUpPath(phenoPath);
		sortUpPath(tcPath);
		sortUpPath(corrPath);
		sortUpPath(graphPath);
		sortUpTCQC();
		break;
	}

	if(true == flag_help) {
		cerr << desc << endl;
		return false;
	}
	return true;
}

std::pair<Option::FileType, std::string> Option::getInputFolder() const
{
	if(!tcPath.empty())
		return make_pair(FileType::TC, tcPath);
	if(!corrPath.empty())
		return make_pair(FileType::CORR, corrPath);
	return make_pair(FileType::NONE, string(""));
}

bool Option::isOutputFolder(FileType ft) const
{
	if(FileType::GRAPH == ft) {
		return !graphPath.empty();
	} else if(FileType::CORR == ft) {
		return !corrPath.empty() && !tcPath.empty();
	}
	return false;
}

std::string& Option::sortUpPath(std::string & path)
{
	if(!path.empty() && path.back() != '/')
		path.push_back('/');
	return path;
}

bool Option::checkIOLogic()
{
	if(tcPath.empty() && corrPath.empty()) {
		cerr << "No input data folder specified!" << endl;
		return false;
	}
	if(corrPath.empty() && graphPath.empty()) {
		cerr << "No output data folder specified!" << endl;
		return false;
	}
	if(tcPath.empty() && !corrPath.empty() && graphPath.empty()) {
		cerr << "No output data folder specified!" << endl;
		return false;
	}
	return true;
}

bool Option::initCutLogic()
{
// 	if(nGraph >= 0 && nScan >= 0) {
// 		cerr << "Method nGraph and nScan conflicts" << endl;
// 		return false;
// 	} else if(nGraph <= 0 && nScan <= 0) {
// 		cerr << "None of method nGraph or nScan is set" << endl;
// 		return false;
// 	}
// 	if(nGraph > 0)
// 		cutMethod = "nGraph";
// 	else if(nScan > 0)
// 		cutMethod = "nScan";
	return true;
}

bool Option::sortUpTCQC()
{
	transform(tcQualityControl.begin(), tcQualityControl.end(), tcQualityControl.begin(), ::tolower);
	return tcQualityControl == "none" || tcQualityControl == "any" || tcQualityControl == "all";
}
