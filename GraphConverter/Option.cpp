#include "stdafx.h"
#include "Option.h"
#include <boost/program_options.hpp>

using namespace std;

Option::Option()
	:desc("Options")
{
	// define
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("dataset", value<string>(&dataset), "specific which dataset is going to be used (ADHD, ).")
		("nSubject,n", value<int>(&nSubject)->default_value(-1), "[integer] # of subjects to load from dataset "
			"non-positive means load all)")
		("tcPath", value<string>(&tcPath), "the folder for time course data (input)")
		("corrPath", value<string>(&corrPath), "the folder for correlation data "
			"(if --tcPath is not given, this is an input folder. otherwise this is used for output)")
		("graphPath", value<string>(&graphPath), "the folder for graph data (output)")
		("nGraph,g", value<int>(&nGraph)->default_value(-1), "[integer] specific number of graphs, conflict with --nScan")
		("nScan,s", value<int>(&nScan)->default_value(-1), "[integer] specific number of scan per graph, conflict with --nGraph")
		("cmethod,m", value<string>(&corrMethod)->default_value(string("pearson")), "method for calculating correlation between ROI,"
			"supports: pearson, spearman, mutialinfo")
		("cthreshold", value<double>(&conThrshd)->default_value(0.5), "the threshold for determining connectivity")
		;

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
		if(!initCutLogic()) {
			flag_help = true;
			break;
		}
		

		sortUpPath(tcPath);
		sortUpPath(corrPath);
		sortUpPath(graphPath);
		break;
	}

	if(true == flag_help) {
		cerr << desc << endl;
		return false;
	}
	return true;
}

std::string Option::getCutMethod() const
{
	return cutMethod;
}

std::pair<Option::FileType, std::string> Option::getInputFolder() const
{
	return std::pair<FileType, std::string>();
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
		return !corrPath.empty();
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
	if(nGraph >= 0 && nScan >= 0) {
		cerr << "Method nGraph and nScan conflicts" << endl;
		return false;
	} else if(nGraph <= 0 && nScan <= 0) {
		cerr << "None of method nGraph or nScan is set" << endl;
		return false;
	}
	if(nGraph > 0)
		cutMethod = "nGraph";
	else if(nScan > 0)
		cutMethod = "nScan";
	return true;
}
