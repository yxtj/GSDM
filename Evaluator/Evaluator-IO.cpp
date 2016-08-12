#include "stdafx.h"
#include "Evaluator-IO.h"
#include "IOfunctions.h"

using namespace std;

//IO part

Graph loadGraph(istream& is) {
	Graph res;
	res.loadDataFromStream(is);
	return res;
}

vector<SubjectData> loadGraph(const string& folder, const vector<int>& graphTypes, int limit, int nSkip) {
	using namespace boost::filesystem;
	path root(folder);
	Subject sub;
	unordered_set<int> types(graphTypes.begin(), graphTypes.end());
	int cnt = 0;
	unordered_map<string, SubjectData> data; //map from subject id to graphs
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		string fn = it->path().filename().string();
		if(!checknParseGraphFilename(fn, &sub) || types.find(sub.type) == types.end())
			continue;
		if(++cnt <= nSkip)
			continue;
		auto dit = data.find(sub.id);
		if(dit == data.end()) {
			dit = data.emplace(sub.id, SubjectData(sub.type, sub.id)).first;
			dit->second.id = sub.id;
			dit->second.type = sub.type;
		}
		ifstream fin(folder + fn);
		dit->second.snapshots.push_back(loadGraph(fin));
		if(cnt >= limit)
			break;
	}
	vector<SubjectData> res;
	for(auto p : data) {
		res.push_back(move(p.second));
	}
	return res;
}

void outputFoundMotifs(ostream& os, const Motif& m) {
	os << m.getnNode() << "\t" << m.getnEdge() << "\t";
	for(const Edge& e : m.edges) {
		os << "(" << e.s << "," << e.d << ") ";
	}
	os << '\n';
}

Motif readMotif(istream& os) {
	string line;
	getline(os, line);
	size_t plast = line.find('\t') + 1;
	size_t p = line.find('\t', plast);
	int nEdge = stoi(line.substr(plast, p - plast));
	plast = p + 1;
	Motif m;
	while(nEdge--) {
		size_t pmid = line.find(',', plast);
		int s = stoi(line.substr(plast, pmid - plast));
		pmid++;
		p = line.find(')', pmid);
		int d = stoi(line.substr(pmid, p - pmid));
		m.addEdge(s, d);
		plast = p + 2;
	}
	return m;
}

vector<Motif> loadMotif(const string& folder, const string& fnPattern, int limit, int nSkip) {
	using namespace boost::filesystem;
	path root(folder);
	regex reg(fnPattern);
	int cnt = 0;
	vector<Motif> res;
	res.reserve(limit);
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		string fn = it->path().filename().string();
		if(!regex_match(fn, reg))
			continue;
		if(++cnt <= nSkip)
			continue;
		ifstream fin(folder + fn);
		res.push_back(readMotif(fin));
		if(res.size() >= limit)
			break;
	}
	return res;
}

void writeConfusionMatrix(std::ostream & os, const std::vector<ConfusionMatrix>& cm)
{
	// TODO
}
