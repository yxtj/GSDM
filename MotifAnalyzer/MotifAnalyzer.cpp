// MotifAnalyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../common/Graph.h"
#include "../common/Motif.h"
#include "../common/SubjectInfo.h"
#include "../eval/SubjectData.h"
#include "Option.h"
#include <regex>
#include <boost/filesystem.hpp>

using namespace std;

// --- Subject ---
Graph loadGraph(istream& is) {
	Graph res;
	res.readFromStream(is);
	return res;
}

vector<vector<SubjectInfo>> loadSubList(const string& folder,
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

	vector<vector<SubjectInfo>> res;
	res.reserve(min(limitSub, id2fn.size()));
	size_t cntSub = 0;
	for(auto& sfp : id2fn) {
		if(cntSub++ >= limitSub)
			break;
		// sort the snapshot files
		sort(sfp.second.begin(), sfp.second.end(), [](SubjectInfo& l, SubjectInfo& r) {
			return l.id < r.id ? true : l.id == r.id&&l.seqNum < r.seqNum;
		});
		if(sfp.second.size() > limitSnp) {
			//partial_sort(sfp.second.begin(), sfp.second.begin() + limitSnp, sfp.second.end());
			sfp.second.erase(sfp.second.begin() + limitSnp, sfp.second.end());
		}
		res.push_back(move(sfp.second));
	}
	return res;
}

vector<pair<SubjectInfo, SubjectData>> loadGraph(
	const string& folder, const vector<vector<SubjectInfo>>& subList)
{
	vector<pair<SubjectInfo, SubjectData>> res;
	for(auto& sl : subList) {
		SubjectData sub;
		for(auto& s : sl) {
			ifstream fin(folder + s.genFilename());
			if(!fin) {
				cerr << "cannot open file: " << folder + s.genFilename() << endl;
			}
			sub.addGraph(loadGraph(fin));
		}
		SubjectInfo info(sl.front().id, sl.front().type);
		res.emplace_back(move(info), move(sub));
	}
	return res;
}

// --- Motif ---
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

std::vector<Motif> readMotif(istream& os) {
	std::vector<Motif> res;
	string line;
	while(getline(os, line)) {
		if(line.empty())
			continue;
		res.push_back(parseMotif(line));
	}
	return res;
}

vector<Motif> loadMotif(const string& folder, const string& fnPattern, int nMotif, int nSkip) {
	using namespace boost::filesystem;
	path root(folder);
	if(!exists(root)) {
		cerr << "cannot open motif folder: " << folder << endl;
		throw invalid_argument("cannot open motif folder: " + folder);
	}
	size_t limit = nMotif > 0 ? nMotif : numeric_limits<size_t>::max();
	regex reg(fnPattern);
	int cnt = 0;
	vector<Motif> res;
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		string fn = it->path().filename().string();
		if(!regex_match(fn, reg))
			continue;
		if(++cnt <= nSkip)
			continue;
		std::ifstream fin(folder + fn);
		auto temp = readMotif(fin);
		move(temp.begin(), temp.end(), back_inserter(res));
		if(res.size() >= limit)
			break;
	}
	if(res.size() > limit) {
		res.erase(res.begin() + limit, res.end());
	}
	return res;
}

// --- Test ---
// row: motif, column: subject
vector<vector<bool>> getContainingTable(
	const vector<pair<SubjectInfo, SubjectData>>& subs, const Motif& m)
{
	vector<vector<bool>> res;
	res.reserve(subs.size());
	for(auto& sp : subs) {
		vector<bool> t = sp.second.enumerate(m);
		res.push_back(move(t));
	}
	return res;
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
		cout << "Data folder: " << opt.pathGraph << "\n"
			<< "  # subjects: " << opt.nSubject << "\t" << "# snapshot: " << opt.nSnapshot << "\n"
			<< "  Type(s) of positive subject: " << opt.typePos << "\n"
			<< "  Type(s) of negative subject: " << opt.typeNeg << "\n"
			<< "  sort by type: " << opt.flgSortSubByType << "\n"
			<< "Motifs folder: " << opt.pathMotif << "\n"
			<< "  # motifs: " << opt.nMotif << "\n"
			<< "  Motif file name pattern: " << opt.motifPattern << "\n"
			<< "Output folder: " << opt.pathOutput << "\n"
			<< endl;
	}

	// load data
	vector<pair<SubjectInfo, SubjectData>> subjects;
	try {
		cout << "Loading graph data..." << endl;
		vector<vector<SubjectInfo>> subList = 
			loadSubList(opt.pathGraph, opt.typeAll, opt.nSubject, opt.nSnapshot);
		if(opt.flgSortSubByType) {
			sort(subList.begin(), subList.end(), 
				[](vector<SubjectInfo>& l, vector<SubjectInfo>& r) {
				return l[0].type != r[0].type ? l[0].type < r[0].type :
					l[0].id < r[0].id;
			});
		}
		subjects = loadGraph(opt.pathGraph, subList);
	} catch(exception& e) {
		cerr << "load data failed!\n  " << e.what() << endl;
		return 2;
	}
	cout << "  # subjects: " << subjects.size() << endl;
	int nPos = 0, nNeg = 0;
	for(auto& sp : subjects) {
		if(find(opt.typePos.begin(), opt.typePos.end(), sp.first.type) != opt.typePos.end())
			++nPos;
		else
			++nNeg;
	}
	cout << "    # positive subjects: " << nPos << endl;
	cout << "    # negative subjects: " << nNeg << endl;

	// generate motif testers
	cout << "Loading motifs..." << endl;
	vector<Motif> motifs = loadMotif(opt.pathMotif, opt.motifPattern, opt.nMotif, 0);
	cout << "  # motifs: " << motifs.size() << endl;

	// test and output
	cout << "Processing and outputing..." << endl;
	{
		boost::filesystem::path p(opt.pathOutput);
		boost::filesystem::create_directories(p.parent_path());
	}
	{
		cout << "  Outputing subject info...." << endl;
		ofstream fout(opt.pathOutput + "sub.txt");
		for(auto& sub : subjects) {
			fout << sub.first.id << "\t" << sub.first.type << "\t" << sub.second.size() << "\n";
		}
	}
	for(size_t i = 0; i < motifs.size(); ++i) {
		cout << "  Processing " << (i + 1) << "/" << motifs.size() << " moitfs" << endl;
		vector<vector<bool>> table = getContainingTable(subjects, motifs[i]);
		ofstream fout(opt.pathOutput + "m" + to_string(i) + ".txt");
		for(auto& vec : table) {
			fout << vec[0];
			for(size_t j = 1; j < vec.size(); ++j)
				fout << "," << vec[j];
			fout << "\n";
		}
	}

	return 0;
}

