#include "stdafx.h"
#include "Util4IO.h"

using namespace std;

std::vector<std::vector<int>> readGraph(const std::string & fn)
{
	ifstream is(fn);
	if(!is) {
		throw invalid_argument("cannot open graph file " + fn);
	}

	string temp;
	//data format: first line is an integer for # of nodes: "n"
	getline(is, temp);
	int nNode = stoi(temp);
	std::vector<std::vector<int>> res(nNode);
	//data format: "id	a b c " i.e. "id\ta b c "
	for(int i = 0; i < nNode; ++i) {
		getline(is, temp);
		size_t plast, p;
		p = temp.find('\t');
		int id = stoi(temp.substr(0, p));
		plast = p + 1;
		p = temp.find(' ', plast);
		while(p != string::npos) {
			int t = stoi(temp.substr(plast, p - plast));
			res[id].push_back(t);
			plast = p + 1;
			p = temp.find(' ', plast);
		}
	}
	return res;
}

void writeGraph(std::ostream & os, const std::vector<std::vector<int>> & g)
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
