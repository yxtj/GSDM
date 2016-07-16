#include "stdafx.h"
#include "Option.h"
#include <boost/program_options.hpp>

using namespace std;

Option::Option()
{
}


Option::~Option()
{
}


bool Option::parseInput(int argc, char* argv[]) {
	//define
	boost::program_options::options_description desc("Options");
	using boost::program_options::value;
	desc.add_options()
		("help", "Print help messages")
		("prefix", value<string>(&prefix)->default_value(string("../data/")), "the folder prefix of the output data")
		("prefix-graph", value<string>(&subFolderGraph)->default_value(string("graph/")), "the subfolder for graph files")
		("npi", value<int>(&nPosInd)->default_value(10), "[integer] number of positive individuals")
		("nni", value<int>(&nNegInd)->default_value(10), "[integer] number of negative individuals")
		("ns",value<int>(&nSnapshot)->default_value(10),"[integer] number of snapshots")
		("npm", value<int>(&nPosMtf)->default_value(10), "[integer] number of positive motifs")
		("nnm", value<int>(&nNegMtf)->default_value(10), "[integer] number of negative motifs")
		("n", value<int>(&nNode), "size of each graph")
		("smmin", value<int>(&sMotifMin)->default_value(2), "[integer] minimum size of a motif")
		("smmax", value<int>(&sMotifMax)->default_value(2), "[integer] maximum size of a motif")
		("alpha", value<double>(&degAlpha)->default_value(4), "[double] the alpha parameter in power-law distribution for degree distribution")
		("pmotif", value<double>(&pMotif)->default_value(0.3), "MEAN probability of the motifs, should NOT be too small")
		("seed", value<unsigned>(&seed)->default_value(123456), "seed for random number")
		;

	//parse
	bool flag_help = false;
	try {
		boost::program_options::variables_map var_map;
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

	if(!prefix.empty() && prefix.back() != '/')
		prefix.push_back('/');
	if(!subFolderGraph.empty() && subFolderGraph.back() != '/')
		subFolderGraph.push_back('/');

	if(true == flag_help) {
		cerr << desc << endl;
		return false;
	}
	return true;
}
