// Evaluator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Option.h"
#include "../common/Motif.h"
#include "../common/Graph.h"
#include "../libEval/ConfusionMatrix.h"
#include "../libEval/SubjectData.h"
#include "../libEval/MTesterSingle.h"
#include "../libEval/MTesterGroup.h"
#include "../libEval/GroupGenerator.h"
#include "Evaluator-IO.h"

using namespace std;


// row: motif, column: subject
vector<vector<bool>> getContainingTable(const vector<SubjectData>& gs,
	const vector<Motif>& ms, const Option& opt)
{
	vector<vector<bool>> res;
	res.reserve(ms.size());
	for(auto& m : ms) {
		MTesterSingle mt(opt.testMethodSingle);
		mt.set(m);
		vector<bool> t(gs.size());
		for(size_t j = 0; j < gs.size(); ++j) {
			t[j] = mt.testSubject(gs[j]);
		}
		res.push_back(move(t));
	}
	return res;
}

// row: motif, column: subject
vector<vector<bool>> getContainingTable(
	const vector<SubjectData>& gs, const vector<MTesterGroup>& mts)
{
	vector<vector<bool>> res;
	res.reserve(mts.size());
	for(auto& m : mts) {
		vector<bool> t(gs.size());
		for(size_t i = 0; i < gs.size(); ++i)
			t[i] = m.testSubject(gs[i]);
		res.push_back(move(t));
	}
	return res;
}

vector<ConfusionMatrix> transTable2Smy(const vector<SubjectData>& gs,
	const vector<vector<bool>>& tbl, const Option& opt)
{
	// tbl: row-motif, column-subject
	vector<ConfusionMatrix> res(tbl.size());
	unordered_set<int> typePos(opt.graphTypePos.begin(), opt.graphTypePos.end());
	for(size_t j = 0; j < gs.size(); ++j) {
		bool isPosSub = typePos.find(gs[j].type) != typePos.end();
		if(isPosSub) {
			for(size_t i = 0; i < tbl.size(); ++i) {
				if(tbl[i][j])
					res[i].tp++;
				else
					res[i].fn++;
			}
		} else {
			for(size_t i = 0; i < tbl.size(); ++i) {
				if(tbl[i][j])
					res[i].fp++;
				else
					res[i].tn++;
			}
		}
	}
	return res;
}

MTesterGroup transGIndex2GTester(const vector<int>& idx,
	const vector<MTesterSingle>& ms, const Option& opt)
{
	vector<MTesterSingle> m;
	for(int i : idx)
		m.push_back(ms[i]);
	MTesterGroup res(opt.testMethodGroup);
	//res.setParam4Single(opt.testMethodSingle);
	res.set(move(m));
	return res;
}

vector<MTesterGroup> transGIndex2GTester(const vector< vector<int> >& idx,
	const vector<MTesterSingle>& ms, const Option& opt)
{
	vector<MTesterGroup> res;
	for(auto& line : idx) {
		res.push_back(transGIndex2GTester(line, ms, opt));
	}
	return res;
}

template <class T>
ostream& operator<<(ostream& os, const vector<T>& vec) {
	for(auto& v : vec) {
		os << v << " ";
	}
	return os;
}

//main function

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInput(argc, argv))
		return 1;

	ios::sync_with_stdio(false);
	if(opt.show) {
		cout << "Input Graph:\n"
			<< "  Folder: " << opt.graphPath << "\n"
			<< "  Types of positive graph: " << opt.graphTypePos << "\t"
			<< "  Types of negative graph: " << opt.graphTypeNeg << "\n"
			<< "  # of graph: " << opt.nGraph << "\t"
			<< "  # of skipped: " << opt.nSkipGraph << "\n";
		cout << "Input Motif:\n"
			<< "  # of motif folders: " << opt.motifPath.size() << "\n"
			<< "  Folders: " << opt.motifPath << "\n"
			<< "  File name pattern: " << opt.motifPattern << "\n"
			<< "  # of motifs in each folder: " << opt.nMotif << "\t"
			<< "  # of skipped in each folder: " << opt.nSkipMotif << "\n";
		cout << "Testing Method:\n"
			<< "  single motif metohd: " << opt.testMethodSingle << "\n"
			<< "  group generate method: " << opt.groupGenerateMethod << "\n"
			<< "  motif group metohd: " << opt.testMethodGroup << "\n";
		cout << "Output files: " << opt.outputFile << endl;
	}
	GroupGenerator gpGen(opt.groupGenerateMethod);

	// load data
	vector<SubjectData> gts;
	try {
		cout << "Loading graph data...";
		cout.flush();
		gts = loadGraph(opt.graphPath, opt.graphTypes, opt.nGraph, opt.nSkipGraph);
		cout << "  # loaded subjects: " << gts.size() << endl;
	} catch(exception& e) {
		cerr << "load data failed!\n  " << e.what() << endl;
		return 2;
	}
	
	// evaluate for each motif-output folder pair
	for(size_t i = 0; i < opt.motifPath.size(); ++i) {
		const string& mpath = opt.motifPath[i];
		const string& opath = opt.outputFile[i];
		cout << "(" << i+1 << "/" << opt.motifPath.size() <<
			") Loading motifs..." << endl;
		vector<MTesterGroup> mts;
		vector<Motif> ms;
		vector< vector<int> > idx;
		try {
			ms = loadMotif(mpath, opt.motifPattern, opt.nMotif, opt.nSkipMotif);
			cout << "  # of loaded motifs: " << ms.size() << endl;
			vector<MTesterSingle> mtSingle;
			mtSingle.reserve(ms.size());
			for(auto& m : ms) {
				mtSingle.emplace_back(opt.testMethodSingle);
				mtSingle.back().set(m);
			}
			idx = gpGen.generate(ms.size());
			mts = transGIndex2GTester(idx, mtSingle, opt);
			cout << "  # of motif testers: " << mts.size() << endl;
		} catch(exception& e) {
			cerr << "Load motif failed!\n  " << e.what() << endl;
			continue;
		}

		cout << "  Evaluating..." << endl;
		// row: subject, column: motif
		vector<vector<bool>> tbl = getContainingTable(gts, mts);
		mts.clear();
		
		cout << "  Outputing result..." << endl;
		// create the output folder if not existed
		{
			boost::filesystem::path p(opath);
			boost::filesystem::create_directories(p.parent_path());
		}

		if(opt.flgOutMotifGroup) {
			size_t pDash = opath.find_last_of("/\\");
			pDash = pDash == string::npos ? 0 : pDash;
			string fnTbl = opath.substr(0, pDash + 1) + "gp-" + opath.substr(pDash + 1);
			ofstream fout(fnTbl);
			if(!fout) {
				cerr << "Cannot open output file: " << fnTbl << endl;
				continue;
			}
			for(auto& line : idx) {
				fout << line[0];
				for(size_t k = 1; k < line.size(); ++k)
					fout << "," << line[k];
				fout << "\n";
			}
		}

		if(opt.flgOutTable) {
			size_t pDash = opath.find_last_of("/\\");
			pDash = pDash == string::npos ? 0 : pDash;
			//size_t pDot = opath.find_last_of(".", string::npos, opath.size() - (pDash != string::npos ? pDash : 0));
			// output containing table
			string fnTbl = opath.substr(0, pDash+1) + "cont-" + opath.substr(pDash+1);
			ofstream fout(fnTbl);
			if(!fout) {
				cerr << "Cannot open output file: " << fnTbl << endl;
				continue;
			}
			for(auto& line : tbl) {
				fout << line[0];
				for(size_t k = 1; k < line.size(); ++k)
					fout << "," << line[k];
				fout << "\n";
			}
			fout.close();
			fout.clear();

			// output subject type list
			string fnList = opath.substr(0, pDash+1) + "type-" + opath.substr(pDash+1);
			fout.open(fnList);
			if(!fout) {
				cerr << "Cannot open output file: " << fnList<< endl;
				continue;
			}
			unordered_set<int> typePos(opt.graphTypePos.begin(), opt.graphTypePos.end());
			for(auto& s : gts) {
				bool b = typePos.find(s.type) != typePos.end();
				fout << b << "\n";
			}
		}

		if(opt.flgOutSmy) {
			cout << "  Transforming table to summary information..." << endl;
			// vector<ConfusionMatrix> smy = evaluate(gts, ms, opt);
			vector<ConfusionMatrix> smy = transTable2Smy(gts, tbl, opt);
			ofstream fout(opath);
			if(!fout) {
				cerr << "Cannot open output file: " << opath << endl;
				continue;
			}
//			showConfusionMatrixHead(fout);
			showHead(fout);
			fout << '\n';
			for(auto& cm : smy) {
//				showConfusionMatrix(fout, cm);
				showData(fout, cm);
				fout << "\n";
			}
		}
	}
	return 0;
}

