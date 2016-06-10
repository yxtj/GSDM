#include "stdafx.h"
#include "Option.h"

using namespace std;

Option::Option()
{
}


Option::~Option()
{
}

bool Option::parseInt(int argc, char * argv[])
{
	boost::program_options::options_description desc("Options");
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("prefix", value<string>(&prefix)->default_value("../data"), "[string] data folder prefix")
		("prefix-graph", value<string>(&subFolderGraph)->default_value(string("graph/")), "the subfolder for graph files")
		("npi", value<int>(&nPosInd)->default_value(10), "[integer] number of positive individuals")
		("nni", value<int>(&nNegInd)->default_value(10), "[integer] number of negative individuals")
		("npm", value<int>(&nPosMtf)->default_value(10), "[integer] number of positive motifs")
		("nnm", value<int>(&nNegMtf)->default_value(10), "[integer] number of negative motifs")
		("ns", value<int>(&nSnapshot)->default_value(10), "[integer] number of snapshots")
		("smmin", value<int>(&sMotifMin)->default_value(2), "[integer] minimum size of a motif")
		("smmax", value<int>(&sMotifMax)->default_value(2), "[integer] maximum size of a motif")
		("n", value<int>(&nNode)->default_value(77), "size of each graph")
		("strategy", value<string>(&stgName)->default_value(string("Freq")), "name of the searching strategy")
		("pmi", value<double>(&pMotifInd)->default_value(0.3), "[double] the min prob. of treating "
			"a motif as existed on a individual (num over snapshot)")
		("pmr", value<double>(&pMotifRef)->default_value(0.8), "[double] the min prob. of treating "
			"a motif as existed all on individual (num over individual)")
		("topk", value<int>(&topK)->default_value(10), "number of returned results");

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

	if(true == flag_help) {
		cerr << desc << endl;
		return false;
	}
	return true;
}
