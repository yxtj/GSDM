#include "stdafx.h"
#include "Graph.h"

using namespace std;

Graph::Graph(const int n)
	: nNode(n), nEdge(0)
{
	init();
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

void Graph::readFromStream(std::istream & is)
{
	int level = checkCompressionLevel(is);
	return readFromStream(is, level);
}

void Graph::readFromStream(std::istream & is, const int comLevel)
{
	if(comLevel == 0)
		readText(is);
	else if(comLevel == 1)
		readBinary(is);
	else
		readCompressed(is, comLevel);
}

void Graph::writeToStream(std::ostream & os, const int comLevel) const
{
	if(comLevel == 0)
		writeText(os);
	else if(comLevel == 1)
		writeBinary(os);
	else
		writeCompressed(os, comLevel);
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

int Graph::checkCompressionLevel(std::istream & is)
{
	int8_t ch = is.peek();
	if(ch < '0') {
		// return the magic byte
		return int(ch);
	}
	return 0;
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
	char magicByte = 1;
	os.put(magicByte);
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
	is.get(); // remove the magic byte
	int32_t n;
	is.read(reinterpret_cast<char*>(&n), sizeof(int32_t));
	nNode = n;
	nEdge = 0;
	init();
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

void Graph::writeCompressed(std::ostream & os, const int level) const
{
	throw invalid_argument("Compressed graph IO is not implemented");
	os.put(char(level));
}

void Graph::readCompressed(std::istream & is, const int level)
{
	throw invalid_argument("Compressed graph IO is not implemented");
}

std::ostream & operator<<(std::ostream & os, const Graph & g)
{
	g.writeToStream(os);
	return os;
}
