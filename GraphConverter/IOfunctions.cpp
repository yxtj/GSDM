#include "stdafx.h"
#include "IOfunctions.h"
#include "LoaderFactory.h"
#include "TCCutter.h"

using namespace std;


std::string genCorrFilename(const Subject & sub)
{
	return to_string(sub.type) + "-" + sub.id + "-" + to_string(sub.scanNum) + ".txt";
}

bool checkCorrFilename(const string & fn)
{
	return checknParseCorrFilename(fn, nullptr);
}

Subject parseCorrFilename(const std::string & fn)
{
	if(fn.empty())
		throw invalid_argument("filename '" + fn + "' is not a valid correlation file name");
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	return Subject(fn.substr(p1 + 1, p2), stoi(fn.substr(0, p1)), stoi(fn.substr(p2 + 1, pend)));
}

bool checknParseCorrFilename(const std::string& fn, Subject* pRes) noexcept
{
	if(fn.empty())
		return false;
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	if(p1 == string::npos || p2 == string::npos || pend == string::npos)
		return false;
	try {
		int type = stoi(fn.substr(0, p1));
		int scanNum = stoi(fn.substr(p2 + 1, pend));
		// keep old res if this operation cannot finish successfully
		if(pRes) {
			pRes->id = fn.substr(p1 + 1, p2);
			pRes->type = type;
			pRes->scanNum = scanNum;
		}
	} catch(...) {
		return false;
	}
	return true;
}

std::map<Subject, tc_t> loadInputTC(const std::string& tcPath, const std::string& dataset)
{
	using namespace boost::filesystem;
	path root(tcPath);
	if(!is_directory(root)) {
		throw invalid_argument("Given time course path is invalid");
	}

	TCLoader* loader = LoaderFactory::generate(dataset);
	map<Subject, tc_t> res;

	vector<Subject> validList = loader->loadValidList(tcPath);
	// get all sub-folders:
	vector<path> ids;
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		if(is_directory(*it))
			ids.push_back(*it);
	}

	for(auto& id : ids) {
		string leaf = id.filename().string();
		//TODO: generalize to other dataset
		//TODO: some folder may have several files
		tc_t tc = loader->loadTimeCourse(tcPath + leaf + "/sfnwmrda"
			+ leaf + "_session_1_rest_1_aal_TCs.1D");
		//TODO: add cutter here
		res.emplace(move(validList[0]), move(tc));
	}
	delete loader;

	return res;
}

std::multimap<Subject, corr_t> loadInputCorr(const std::string& corrPath)
{
	using namespace boost::filesystem;
	path root(corrPath);
	if(!is_directory(root)) {
		throw invalid_argument("Given correlation path is invalid");
	}

	std::multimap<Subject, corr_t> res;
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		const string&& fn = it->path().filename().string();
		Subject sub;
		if(is_regular_file(*it) && checknParseCorrFilename(fn, &sub)) { 
			res.emplace(move(sub), readCorr(fn));
		}
	}
	return res;
}

corr_t readCorr(const std::string & fn)
{
	ifstream fin(fn);
	if(!fin) {
		throw invalid_argument("cannot open correlation file " + fn);
	}
//	int n;
// 	fin >> n;
// 	corr_t res;
// 	res.reserve(n);
	//TODO: change to parsing lines, to speed up.
// 	for(int i = 0; i < n; ++i) {
// 		vector<double> temp(n);
// 		for(int j = 0; j < n; ++j) {
// 			fin >> temp[j];
// 		}
// 		res.push_back(move(temp));
// 	}
//	return res;
	string line;
	getline(fin, line);
	int n = stoi(line);
	corr_t res;
	res.reserve(n);
	for(int i = 0; i < n; ++i) {
		getline(fin, line);
		vector<double> temp;
		temp.reserve(n);
		size_t plast = 0, p = line.find(' ');
		while(p != string::npos) {
			temp.push_back(stod(line.substr(plast, p)));
			plast = p + 1;
			p = line.find(' ', plast);
		}
		res.push_back(move(temp));
	}
	return res;
}

void writeCorr(ostream& os, const corr_t& corr)
{
	os << corr.size() << "\n";
	for(auto& line : corr) {
		for(auto& v : line) {
			os << v << ' ';
		}
		os << "\n";
	}
}

std::string genGraphFilename(const Subject & sub)
{
	return genCorrFilename(sub);
}

bool checknParseGraphFilename(const std::string & fn, Subject * pRes) noexcept
{
	return checknParseCorrFilename(fn, pRes);
}

graph_t readGraph(const std::string & fn)
{
	ifstream is(fn);
	if(!is) {
		throw invalid_argument("cannot open graph file " + fn);
	}

	string temp;
	//data format: first line is an integer for # of nodes: "n"
	getline(is, temp);
	int nNode = stoi(temp);
	graph_t res(nNode);
	//data format: "id	a b c " i.e. "id\ta b c "
	for(int i = 0; i < nNode; ++i) {
		getline(is, temp);
		size_t plast, p;
		p = temp.find('\t');
		int id = stoi(temp.substr(0, p));
		plast = p + 1;
		p = temp.find(' ', plast);
		while(p != string::npos) {
			int t = stoi(temp.substr(plast, p));
			res[id].push_back(t);
			plast = p + 1;
			p = temp.find(' ', plast);
		}
	}
	return res;
}

void writeGraph(std::ostream & os, const graph_t & g)
{
	size_t n = g.size();
	os << n << '\n';
	for(size_t i = 0; i < n; ++i) {
		os << i << '\t';
		for(auto dst : g[i]) {
			os << dst << ' ';
		}
		os << '\n';
	}
}

