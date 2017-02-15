// Validator.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <fstream>
#include <functional>
#include "Option.h"
#include "Scorer.h"
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "../common/SubjectInfo.h"
#include <boost/filesystem.hpp>

using namespace std;

Graph loadGraph(istream& is) {
	Graph res;
	res.readFromStream(is);
	return res;
}

vector<vector<Graph> > loadData(const string& pre, const int n, const int m) {
	vector<vector<Graph> > res;
	res.reserve(n);
	for(int i = 0; i < n; ++i) {
		vector<Graph> temp;
		temp.reserve(m);
		for(int j = 0; j < m; ++j) {
			ifstream fin(pre + to_string(i) + "-" + to_string(j) + ".txt");
			temp.push_back(loadGraph(fin));
		}
		res.push_back(move(temp));
	}
	return res;
}

vector<vector<Graph> > loadData(
	const string& folder, const vector<int>& types, const int nSub, const int nSnap, const int nSkip = 0)
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
	map<decltype(SubjectInfo::id), vector<string>> id2fn;
	for(auto it = boost::filesystem::directory_iterator(root);
		it != boost::filesystem::directory_iterator(); ++it)
	{
		SubjectInfo sub;
		string fn = it->path().filename().string();
		if(boost::filesystem::is_regular_file(it->status()) && sub.parseFromFilename(fn)) {
			// check type
			if(validType.find(sub.type) == validType.end())
				continue;
			id2fn[sub.id].push_back(move(fn));
		}
	}
	// load data
	vector<vector<Graph> > res(min(limitSub, id2fn.size()));
	int cntSub = 0;
	size_t pres = 0;
	for(auto& sfp : id2fn) {
		if(++cntSub <= nSkip)
			continue;
		else if(pres >= limitSub)
			break;
		//		cout <<"rank: "<<rank<< " type: " << types[0] << " id: " << sfp.first << endl;
		// sort the snapshot files
		if(sfp.second.size() <= limitSnp) {
			sort(sfp.second.begin(), sfp.second.end());
		} else {
			auto it = sfp.second.begin() + limitSnp;
			partial_sort(sfp.second.begin(), it, sfp.second.end());
		}
		vector<Graph>& vec = res[pres++];
		size_t cntSnp = 0;
		for(auto& fn : sfp.second) {
			if(++cntSnp > limitSnp)
				break;
			ifstream fin(folder + fn);
			if(!fin) {
				cerr << "cannot open file: " << fn << endl;
			}
			vec.push_back(loadGraph(fin));
		}
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

bool existOnSub(const std::vector<Graph>& gs, const Motif& m, const double theta)
{
	int cnt = 0;
	int limit = static_cast<int>(ceil(gs.size()*theta));
	for(auto& g : gs) {
		if(g.testMotif(m)) {
			if(++cnt >= limit)
				return true;
		}
	}
	return false;
}


double probOnGS(const vector<vector<Graph>>& gs, const Motif& m, const double theta)
{
	int cnt = 0;
	for(auto& l : gs)
		if(existOnSub(l, m, theta))
			++cnt;
	return static_cast<double>(cnt) / gs.size();
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

	function<double(const double, const double)> objFun;
	if(opt.funName == "diff") {
		objFun = bind(&Scorer::diff, placeholders::_1, placeholders::_2, opt.alpha);
	} else if(opt.funName == "ratio") {
		objFun = Scorer::ratio;
	}

	int nPosSub = opt.nPosInd, nPosSkip = 0;
	int nNegSub = opt.nNegInd, nNegSkip = 0;
	cout << "Loading graphs on rank..." << endl;
	vector<vector<Graph> > gPos = loadData(opt.graphFolder, opt.typePos, nPosSub, opt.nSnapshot, nPosSkip);
	cout << "  # positive subjects: " << gPos.size() << endl;
	vector<vector<Graph> > gNeg = loadData(opt.graphFolder, opt.typeNeg, nNegSub, opt.nSnapshot, nNegSkip);
	cout << "  # negative subjects: " << gNeg.size() << endl;

	string line;
	cout << "Format:\n"
		"XXX	nEdge	(a,b) (c,d) (...)\n"
		"Waiting for input (one per line): "<< endl;
	while(getline(cin, line)) {
		Motif m;
		try {
			m = parseMotif(line);
		} catch(...) {
			cout << "cannot parse given motif." << endl;
			continue;
		}
		double fPos = probOnGS(gPos, m, opt.theta);
		double fNeg = probOnGS(gNeg, m, opt.theta);
		double score = objFun(fPos, fNeg);
		cout << "fPos=" << fPos << ", fNeg=" << fNeg << ", score=" << score << endl;
	}
	return 0;
}

