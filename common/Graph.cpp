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

size_t Graph::getnNode() const
{
	return size_t(nNode);
}

size_t Graph::getnEdge() const
{
	return size_t(nEdge);
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

void Graph::readFromStream(std::istream & is, const bool compress)
{
	if(!compress)
		readText(is);
	else
		readBinary(is);
}

void Graph::writeToStream(std::ostream & os, const bool compress) const
{
	if(!compress)
		writeText(os);
	else
		writeBinary(os);
}

bool Graph::testEdge(const Edge & e) const
{
	return matrix.at(e.s).at(e.d);
}

bool Graph::testEdge(const int s, const int d) const
{
	return matrix.at(s).at(d);
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

bool Graph::testMotif(const MotifBuilder & m) const
{
	bool res = true;
	try {
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

void Graph::writeText(std::ostream & os) const
{
	os << nNode << '\n';
	for(int i = 0; i < nNode; ++i) {
		os << i << '\t';
		const vector<bool>& line = matrix[i];
		for(int j = 0; j < nNode; ++j) {
			if(line[j])
				os << j << ' ';
		}
		os << '\n';
	}
}

void Graph::readText(std::istream & is)
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

void Graph::writeBinary(std::ostream & os) const
{
	// XXX: no tested
	int len = (nNode + 7) / 8;
	char* buff = new char[len];
	*reinterpret_cast<int32_t*>(buff) = int32_t(nNode);
	os.write(buff, sizeof(int32_t));
	for(int i = 0; i < nNode; ++i) {
		char* p = buff;
		*p = '\0';
		for(int j = 0; j < nNode; ++j) {
			if(j != 0 && j % 8 == 0) {
				p++;
				*p = '\0';
			}
			if(matrix[i][j]) {
				*p |= 1 << (j % 8);
			}
		}
		os.write(buff, len);
	}
	delete[] buff;
}

void Graph::readBinary(std::istream & is)
{
	// XXX: no tested
	int32_t n;
	is.read(reinterpret_cast<char*>(&n), sizeof(int32_t));
	nNode = n;
	int len = (nNode + 7) / 8;
	char* buff = new char[len];
	for(int i = 0; i < nNode; ++i) {
		is.read(buff, len);
		char* p = buff;
		for(int j = 0; j < nNode; ++j) {
			if(j != 0 && j % 8 == 0) {
				p++;
			}
			if((((*p) >> (j % 8)) & 1) == 1) {
				matrix[i][j] = true;
			}
		}
	}
	delete[] buff;

}

std::ostream & operator<<(std::ostream & os, const Graph & g)
{
	g.writeToStream(os);
	return os;
}
