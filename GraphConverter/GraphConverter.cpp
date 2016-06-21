// ADHD200Converter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Option.h"
#include "LoaderFactory.h"
#include "IOfunctions.h"
#include "TCCutter.h"
#include "TC2Corr.h"
#include "Corr2Graph.h"

using namespace std;

multimap<Subject, corr_t> processTC2Corr(map<Subject, tc_t>& smtc, const Option& opt) {
	string cm = opt.corrMethod;
	// TODO: use a delegate function to directly generate TCCutter
	int cparm = cm == "nGraph" ? opt.nGraph : opt.nScan;

	TC2Corr t2c(opt.corrMethod);
	multimap<Subject, corr_t> res;
	for(auto& p : smtc) {
		TCCutter cutter(p.second, cm, cparm);
		while(cutter.haveNext()) {
			tc_t t = cutter.getNext();
			res.emplace(p.first, t2c.getCorr(t));
		}
	}
	return res;
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
		<< "Cutting method: " << opt.getCutMethod() << "\n"
		<< "Cutting method parameter - nGraph: " << opt.nGraph << "\n"
		<< "Cutting method parameter - nScan: " << opt.nScan << "\n"
		<< "Correlation method: " << opt.corrMethod << "\n"
		<< "Correlation threshold: " << opt.conThrshd << "\n"
		<< endl;
	
	cout << "Generating correlation:" << endl;
	multimap<Subject, corr_t> corr;
	try {
		auto p = opt.getInputFolder();
		if(p.first == Option::FileType::TC) {
			map<Subject, tc_t> smtc = loadInputTC(opt.tcPath, opt.dataset);
			corr = processTC2Corr(smtc, opt);
		} else if(p.first == Option::FileType::CORR) {
			corr = loadInputCorr(opt.corrPath);
		}
	} catch(exception& e) {
		cerr << e.what() << endl;
		return 2;
	}

	if(opt.isOutputFolder(Option::FileType::CORR)) {
		cout << "Outputting correlations:" << endl;
		for(auto p : corr) {
			string fn = opt.corrPath + genCorrFilename(p.first);
			ofstream fout(fn);
			if(!fout) {
				cerr << "  Cannot open output file: " << fn << endl;
				continue;
			}
			writeCorr(fout, p.second);
		}
	}

	if(opt.isOutputFolder(Option::FileType::GRAPH)) {
		cout << "Generate graphs:" << endl;
		Corr2Graph c2g(opt.conThrshd);
		for(auto p : corr) {
			string fn = opt.graphPath + genGraphFilename(p.first);
			ofstream fout(fn);
			if(!fout) {
				cerr << "  Cannot open output file: " << fn << endl;
				continue;
			}
			writeGraph(fout, c2g.getGraph(p.second));
		}

	}
    return 0;
}

