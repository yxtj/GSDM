#include "stdafx.h"
#include "Graph.h"

using namespace std;

Graph::Graph(Graph && g)
	: nNode(g.nNode), nEdge(g.nEdge), matrix(move(g.matrix))
{
}

Graph::Graph(const int n)
	: nNode(n), nEdge(0)
{
}

bool Graph::init()
{
	if(nNode == 0)
		return false;
	matrix.clear();
	for(int i = 0; i < nNode; ++i) {
		matrix.push_back(vector<bool>(nNode, false));
	}
	return true;
}

void Graph::loadDataFromStream(std::istream & is)
{
	string temp;
	//data format: first line is an integer for # of nodes: "n"
	getline(is, temp);
	nNode = stoi(temp);
	nEdge = 0;
	init();
	//data format: "id	a b c " i.e. "id\ta b c "
	for(int i = 0; i < nNode; ++i) {	
		getline(is, temp);
	//while(getline(is, temp) && !temp.empty()){
		size_t plast, p;
		p = temp.find('\t');
		int n = stoi(temp.substr(0, p));
		plast = p + 1;
		p = temp.find(' ', plast);
		while(p != string::npos) {
			int t = stoi(temp.substr(plast, p));
			matrix[n][t] = true;
			++nEdge;
			plast = p + 1;
			p = temp.find(' ', plast);
		}
	}
}

bool Graph::testMotif(const Motif & m) const
{
	bool res = true;
	try{
		for(const Edge& e : m.edges) {
			if(!matrix.at(e.s).at(e.d)) {
				res = false;
				break;
			}
		}
	} catch(...) {
		res = false;
	}
	return res;
}

