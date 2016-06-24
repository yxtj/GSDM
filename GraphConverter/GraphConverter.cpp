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

multimap<Subject, corr_t> processTC2Corr(multimap<Subject, tc_t>& smtc, const Option& opt) {
	TC2Corr t2c(opt.corrMethod);
	multimap<Subject, corr_t> res;
	for(auto& p : smtc) {
		TCCutter cutter(p.second, opt.cutp);
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

	ios::sync_with_stdio(false);
	cout << "Time Course Path: " << opt.tcPath << "\n"
		<< "Correlation Path: " << opt.corrPath << "\n"
		<< "Graph Path: " << opt.graphPath << "\n"
		<< "  Input data: " << opt.getInputFolder().second << "\n"
		<< ios::boolalpha
		<< "  Output to correlation: " << opt.isOutputFolder(Option::FileType::CORR) << "\t"
		<< "  Output to graph: " << opt.isOutputFolder(Option::FileType::GRAPH) << "\n"
		<< "Dataset name: " << opt.dataset << "\n"
		<< "Cutting method: " << opt.getCutMethod() << "\n"
//		<< "Cutting method parameter - nGraph: " << opt.nGraph << "\n"
//		<< "Cutting method parameter - nScan: " << opt.nScan << "\n"
//		<< "Correlation method: " << opt.corrMethod << "\n"
		<< "Correlation threshold: " << opt.graphThrshd << "\n"
		<< endl;
	cout << "Cutting method: " << opt.cutp.method << "\n" 
		<< "Cutting method parameter - nEach: " << opt.cutp.nEach << "\n"
		<< "Cutting method parameter - nTotal: " << opt.cutp.nTotal << "\n"
		<< "Cutting method parameter - nStep: " << opt.cutp.nStep << "\n"
		<< endl;
	
	cout << "Loading input data:" << endl;
	multimap<Subject, corr_t> corr;
	try {
		auto p = opt.getInputFolder();
		if(p.first == Option::FileType::TC) {
			cout << "  Loading time course data..." << endl;
			multimap<Subject, tc_t> smtc = loadInputTC(opt.tcPath, opt.dataset, opt.nSubject);
			cout << "    # of Loaded: " << smtc.size() << endl;
			cout << "  Generating correlation..." << endl;
			corr = processTC2Corr(smtc, opt);
		} else if(p.first == Option::FileType::CORR) {
			cout << "  Loading correlation data:" << endl;
			corr = loadInputCorr(opt.corrPath, opt.nSubject);
		}
		cout << "    # of Loaded " << corr.size() << endl;
	} catch(exception& e) {
		cerr << e.what() << endl;
		return 2;
	}

	if(opt.isOutputFolder(Option::FileType::CORR)) {
		cout << "Outputting correlations..." << endl;
		int cnt = 0;
		for(auto p : corr) {
			string fn = opt.corrPath + genCorrFilename(p.first);
			ofstream fout(fn);
			if(!fout) {
				cerr << "  Cannot open output file: " << fn << endl;
				continue;
			}
			writeCorr(fout, p.second);
			if(++cnt % 100 == 0)
				cout << "  Outputted " << cnt << endl;
		}
		cout << "  Outputted " << cnt << endl;
	}

	if(opt.isOutputFolder(Option::FileType::GRAPH)) {
		cout << "Generate graphs...s" << endl;
		Corr2Graph c2g(opt.graphThrshd);
		int cnt = 0;
		for(auto p : corr) {
			string fn = opt.graphPath + genGraphFilename(p.first);
			ofstream fout(fn);
			if(!fout) {
				cerr << "  Cannot open output file: " << fn << endl;
				continue;
			}
			writeGraph(fout, c2g.getGraph(p.second));
			if(++cnt % 100 == 0)
				cout << "  Outputted " << cnt << endl;
		}
		cout << "  Outputted " << cnt << endl;
	}

	cout << "Finished" << endl;
    return 0;
}

