// ADHD200Converter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Option.h"
#include "LoaderFactory.h"
#include "TCCutter.h"
#include "TC2Corr.h"
#include "Corr2Graph.h"

using namespace std;

// <subject id, DX type>
multimap<string, corr_t> loadInputData(const Option& opt) {
	auto p = opt.getInputFolder();
	multimap<string, corr_t> res;
	if(p.first == Option::FileType::TC) {
		using namespace boost::filesystem;
		path root(opt.tcPath);
		if(!is_directory(root)) {
			cerr << "Given time course path is invalid" << endl;
			throw invalid_argument("Given time course path is invalid");
		}
		vector<path> ids;
		for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
			if(is_directory(*it))
				ids.push_back(*it);
		}

		TCLoader* loader = LoaderFactory::generate(opt.dataset);
		if(!loader) {
			cerr << "Cannot generate a data loader by name " << opt.dataset << endl;
			throw invalid_argument("Cannot generate data loader");
		}
		TC2Corr t2c;

		for(auto& id : ids) {
			string leaf = id.filename().string();
			//TODO: generalize to other dataset
			//TODO: some folder may have several files
			tc_t tc = loader->loadTimeCourse(opt.tcPath + leaf + "/sfnwmrda"
				+ leaf + "_session_1_rest_1_aal_TCs.1D");
			//TODO: add cutter here
			res.emplace(leaf, t2c.getCorr(tc));
		}
		delete loader;
	}
	if(p.first == Option::FileType::CORR) {

	}
	return res;
}

void outputCorr(ostream& os, const corr_t& corr, const string& sepper=" ") {
	os << corr.size() << "\n";
	for(auto& line : corr) {
		for(auto& v : line) {
			os << v << sepper;
		}
		os << "\n";
	}
}

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInput(argc, argv))
		return 1;

	cout << "Time Course Path: " << opt.tcPath << "\n"
		<< "Correlation Path: " << opt.corrPath << "\n"
		<< "Graph Path: " << opt.graphPath << "\n"
		<< "Dataset name: " << opt.dataset << "\n"
		<< "Cutting method: " << (opt.getCutType().first == Option::CutType::NGRAPH ? "nGraph" : "nScan") << "\n"
		<< "Cutting method parameter - nGraph: " << opt.nGraph << "\n"
		<< "Cutting method parameter - nScan: " << opt.nScan << "\n"
		<< endl;
	
	cout << "Generating correlation:" << endl;
	multimap<string, corr_t> corr;
	try {
		 corr = loadInputData(opt);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return 2;
	}

	{
		auto pOut = opt.getOutputFolder(Option::FileType::CORR);
		if(pOut.first) {
			cout << "Outputting correlations:" << endl;
			for(auto p : corr) {
				//TODO: add type to the name
				string fn = opt.corrPath + p.first + ".txt";
				ofstream fout(fn);
				if(!fout) {
					cerr << "Cannot open output file: " << fn << endl;
					continue;
				}
				outputCorr(fout, p.second);
			}

		}
	}

	auto pOut = opt.getOutputFolder(Option::FileType::GRAPH);
	if(pOut.first) {
		cout << "Generate graphs:" << endl;
		vector<graph_t> graph;
		Corr2Graph c2g(opt.conThrshd);
		for(auto p : corr) {
			graph.push_back(c2g.getGraph(p.second));
		}

	}
    return 0;
}

