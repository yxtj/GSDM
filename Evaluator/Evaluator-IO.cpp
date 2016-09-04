#include "stdafx.h"
#include "Evaluator-IO.h"

using namespace std;

//IO part

Graph loadGraph(istream& is) {
	Graph res;
	res.readFromStream(is);
	return res;
}

vector<SubjectData> loadGraph(const string& folder, const vector<int>& graphTypes, int nGraph, int nSkip) {
	using namespace boost::filesystem;
	path root(folder);
	if(!exists(root)) {
		cerr << "cannot open graph folder: " << folder << endl;
		throw invalid_argument("cannot open graph folder: " + folder);
	}
	int limit = nGraph > 0 ? nGraph : numeric_limits<int>::max();
	SubjectInfo sub;
	unordered_set<int> types(graphTypes.begin(), graphTypes.end());
	int cnt = 0;
	unordered_map<string, SubjectData> data; //map from subject id to graphs
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		string fn = it->path().filename().string();
		if(!sub.parseFromFilename(fn) || types.find(sub.type) == types.end())
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
		ifstream fin(folder + fn);
		auto temp = readMotif(fin);
		move(temp.begin(), temp.end(), back_inserter(res));
		if(res.size() >= limit)
			break;
	}
	return res;
}

static const char sep = '\t';

void showConfusionMatrixHead(std::ostream & os)
{
	os << "tp" << sep << "fn" << sep << "fp" << sep << "tn";
	os << sep << "accuracy" << sep << "precision" << sep << "recall" << sep << "f1";
}
void showConfusionMatrix(std::ostream & os, const ConfusionMatrix& cm) 
{
	os << cm.tp << sep << cm.fn << sep << cm.fp << sep << cm.tn;
	os << sep << cm.accuracy() << sep << cm.precision() << sep << cm.recall() << sep << cm.f1();
}


void showHead(std::ostream & os)
{
	showConfusionMatrixHead(os);
	os << sep << "fre-pos" << sep << "fre-neg" << sep << "score-diff" << sep << "score-ratio";
}

void showData(std::ostream & os, const ConfusionMatrix& cm)
{
	showConfusionMatrix(os, cm);
	double freq_p = cm.tpr(); // exist among pos. / total of pos.
	double freq_n = cm.fpr(); // exist among neg. / total of neg.
	os << sep << freq_p << sep << freq_n << sep << freq_p - freq_n << sep << freq_p / freq_n;
}
