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

multimap<SubjectInfo, corr_t> processTC2Corr(multimap<SubjectInfo, tc_t>& smtc, const Option& opt) {
	TC2Corr t2c(opt.corrMethod);
	multimap<SubjectInfo, corr_t> res;
	string lastID;
	int gid = 0;
	int cnt = 0;
	for(auto& p : smtc) {
		TCCutter cutter(p.second, opt.cutp);
		SubjectInfo sub = p.first;
		if(sub.id != lastID) {
			lastID = sub.id;
			gid = 0;
		}
		while(cutter.haveNext()) {
			tc_t t = cutter.getNext();
			// use seqNum as graphID
			sub.seqNum = gid++;
			res.emplace(sub, t2c.getCorr(t));
		}
		if(++cnt % 100 == 0)
			cout << "  processed " << cnt << endl;
	}
	return res;
}

template<class T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
	for(const auto& v : vec)
		os << v << " ";
	return os;
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
		<< "  Input path: " << opt.getInputFolder().second << "\n"
		<< "  Output to correlation: " << boolalpha << opt.isOutputFolder(Option::FileType::CORR) << "\n"
		<< "  Output to graph: " << boolalpha << opt.isOutputFolder(Option::FileType::GRAPH) << "\n"
		<< "Dataset name: " << opt.dataset << "\n"
		<< "  Number of skipped itmes " << opt.nSkip << "\n"
		<< "  Number of items " << opt.nSubject << "\n"
		<< "Correlation method: " << opt.corrMethod << "\n"
		<< "Correlation threshold: " << opt.graphParam<< "\n"
		<< endl;
	cout << "Cutting method: " << opt.cutp.method << "\n" 
		<< "Cutting method parameter - nEach: " << opt.cutp.nEach << "\n"
		<< "Cutting method parameter - nTotal: " << opt.cutp.nTotal << "\n"
		<< "Cutting method parameter - nStep: " << opt.cutp.nStep << "\n"
		<< endl;
	
	cout << "Loading input data:" << endl;
	multimap<SubjectInfo, corr_t> corr;
	try {
		auto p = opt.getInputFolder();
		if(p.first == Option::FileType::TC) {
			cout << "  Loading time course data..." << endl;
			multimap<SubjectInfo, tc_t> smtc = loadInputTC(
				opt.phenoPath, opt.tcPath, opt.tcQualityControl, opt.dataset, opt.nSubject, opt.nSkip);
			cout << "    # of loaded: " << smtc.size() << endl;
			cout << "  Generating correlation..." << endl;
			corr = processTC2Corr(smtc, opt);
		} else if(p.first == Option::FileType::CORR) {
			cout << "  Loading correlation data:" << endl;
			corr = loadInputCorr(opt.corrPath, opt.nSubject, opt.nSkip);
		}
		cout << "    # of loaded input data " << corr.size() << endl;
	} catch(exception& e) {
		cerr << e.what() << endl;
		return 2;
	}

	if(opt.isOutputFolder(Option::FileType::CORR)) {
		cout << "Outputting correlations..." << endl;
		if(!boost::filesystem::is_directory(opt.corrPath)) {
			boost::filesystem::create_directories(opt.corrPath);
		}
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
				cout << "  # of  outputted " << cnt << endl;
		}
		cout << "  # of outputted " << cnt << endl;
	}

	if(opt.isOutputFolder(Option::FileType::GRAPH)) {
		cout << "Generate graphs..." << endl;
		if(!boost::filesystem::is_directory(opt.graphPath)) {
			boost::filesystem::create_directories(opt.graphPath);
		}
		Corr2Graph c2g;
		try {
			c2g.parseParam(opt.graphParam);
		} catch(exception& e) {
			cerr << e.what() << endl;
			return 1;
		}
		int cnt = 0;
		for(auto p : corr) {
			string fn = opt.graphPath + genGraphFilename(p.first);
			ofstream fout(fn);
			if(!fout) {
				cerr << "  Cannot open output file: " << fn << endl;
				continue;
			}
			c2g.getGraph(p.second).writeToStream(fout, opt.comGraphLevel);
			if(++cnt % 100 == 0)
				cout << "  # of outputted " << cnt << endl;
		}
		cout << "  # of outputted " << cnt << endl;
	}

	cout << "Finished" << endl;
	return 0;
}

