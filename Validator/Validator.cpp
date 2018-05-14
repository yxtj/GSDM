// Validator.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <fstream>
#include <functional>
#include "Option.h"
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "../common/SubjectInfo.h"
#include "../holder/DataHolder.h"
#include "../objfunc/ObjFunction.h"
#include <boost/filesystem.hpp>

using namespace std;

Graph loadGraph(istream& is) {
	Graph res;
	res.readFromStream(is);
	return res;
}

vector<vector<string>> loadFnList(const string& folder,
	const vector<int>& types, const int nSub, const int nSnap, const int nSkip = 0)
{
	boost::filesystem::path root(folder);
	if(!exists(root)) {
		cerr << "cannot open graph folder: " << folder << endl;
		throw invalid_argument("cannot open graph folder: " + folder);
	}
	size_t limitSub = nSub >= 0 ? nSub : numeric_limits<size_t>::max();
	size_t limitSnp = nSnap > 0 ? nSnap : numeric_limits<size_t>::max();
	unordered_set<int> validType(types.begin(), types.end());

	// sort up the file list (ensure the file order)
	map<decltype(SubjectInfo::id), vector<SubjectInfo>> id2fn;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		SubjectInfo sub;
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && sub.parseFromFilename(fn)) {
			// check type
			if(validType.find(sub.type) == validType.end())
				continue;
			id2fn[sub.id].push_back(move(sub));
		}
	}
	if(nSkip > 0) {
		auto it = id2fn.begin();
		advance(it, nSkip);
		id2fn.erase(id2fn.begin(), it);
	}

	vector<vector<string>> fnList;
	fnList.reserve(min(limitSub, id2fn.size()));
	size_t cntSub = 0;
	for(auto& sfp : id2fn) {
		if(cntSub++ >= limitSub)
			break;
		//		cout <<"rank: "<<rank<< " type: " << types[0] << " id: " << sfp.first << endl;
		// sort the snapshot files
		if(sfp.second.size() > limitSnp) {
			//partial_sort(sfp.second.begin(), sfp.second.begin() + limitSnp, sfp.second.end());
			sort(sfp.second.begin(), sfp.second.end(), [](SubjectInfo& l, SubjectInfo& r) {
				return l.id < r.id ? true : l.id == r.id&&l.seqNum < r.seqNum;
			});
			//sort(sfp.second.begin(), sfp.second.end());
			sfp.second.erase(sfp.second.begin() + limitSnp, sfp.second.end());
		}
		vector<string> fns;
		fns.reserve(min(sfp.second.size(), limitSnp));
		for(auto& sub : sfp.second)
			fns.push_back(sub.genFilename());
		fnList.push_back(move(fns));
	}
	return fnList;
}

DataHolder loadData(const string& folder, const vector<vector<string>>& fnList)
{
	DataHolder res;
	for(auto& sfl : fnList) {
		Subject sub;
		for(auto& fn : sfl) {
			ifstream fin(folder + fn);
			if(!fin) {
				cerr << "cannot open file: " << fn << endl;
			}
			sub.addGraph(loadGraph(fin));
		}
		res.addSubject(move(sub));
	}
	return res;
}

Motif parseMotif(const std::string& line) {
	size_t plast = line.find('\t') + 1;
	size_t p = line.find('\t', plast);
	int nEdge = stoi(line.substr(plast, p - plast));
	plast = p + 2;
	Motif m;
	while(nEdge--) {
		size_t pmid = line.find(',', plast);
		int s = stoi(line.substr(plast, pmid - plast));
		pmid++;
		p = line.find(')', pmid);
		int d = stoi(line.substr(pmid, p - pmid));
		m.addEdge(s, d);
		plast = p + 3;
	}
	return m;
}

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& param) {
	for(auto& p : param)
		os << p << " ";
	return os;
}

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInput(argc, argv)) {
		return 1;
	}
	if(opt.show) {
		cout << "Data folder: " << opt.graphFolder << "\n"
			<< "Data parameters:\n"
			<< "  # Nodes: " << opt.nNode << "\n"
			<< "  # Subject +/-: " << opt.nPosInd << " / " << opt.nNegInd << "\n"
			<< "  # Snapshots: " << opt.nSnapshot << "\n"
			<< "  Type(s) of positive subject: " << opt.typePos << "\n"
			<< "  Type(s) of negative subject: " << opt.typeNeg << "\n"
			<< "Theta = " << opt.theta << "\n"
			<< "  Periodic check: " << opt.periodic << "\n"
			<< "Objective function: " << opt.funName
			<< endl;
	}

	ObjFunction objFun;
	objFun.setFunc(opt.funName);

	int nPosSub = opt.nPosInd, nPosSkip = 0;
	int nNegSub = opt.nNegInd, nNegSkip = 0;
	cout << "Loading graphs on rank..." << endl;
	DataHolder dPos = loadData(opt.graphFolder,
		loadFnList(opt.graphFolder, opt.typePos, nPosSub, opt.nSnapshot, nPosSkip));
	cout << "  # positive subjects: " << dPos.size() << endl;
	DataHolder dNeg = loadData(opt.graphFolder, 
		loadFnList(opt.graphFolder, opt.typeNeg, nNegSub, opt.nSnapshot, nNegSkip));
	cout << "  # negative subjects: " << dNeg.size() << endl;

	dPos.setTheta(opt.theta);
	dNeg.setTheta(opt.theta);
	function<double(const DataHolder&, const Motif& m)> probOnGS;
	if(!opt.periodic) {
		probOnGS = [](const DataHolder& dh, const Motif& m) {
			return dh.count(m) / static_cast<double>(dh.size());
		};
	} else {
		probOnGS = [](const DataHolder& dh, const Motif& m) {
			return dh.countByPeriod(m) / static_cast<double>(dh.size());
		};
	}

	cout << "Format:\n"
		"XXX	nEdge	(a,b) (c,d) (...)\n"
		"Waiting for input (one per line): "<< endl;
	string line;
	while(getline(cin, line)) {
		if(line.empty())
			continue;
		Motif m;
		try {
			m = parseMotif(line);
		} catch(...) {
			cout << "cannot parse given motif." << endl;
			continue;
		}
		double fPos = probOnGS(dPos, m);
		double fNeg = probOnGS(dNeg, m);
		double score = objFun.score(fPos, fNeg);
		cout << "fPos=" << fPos << ", fNeg=" << fNeg << ", score=" << score << endl;
	}
	return 0;
}

