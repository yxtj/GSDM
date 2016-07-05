#include "stdafx.h"
#include "Motif.h"

using namespace std;

//Motif::Motif()
//{
//}

Motif::Motif(Motif && m)
	: id(m.id),nNode(m.nNode), nEdge(m.nEdge), edges(move(m.edges))
{
}

//Motif::~Motif()
//{
//}

void Motif::autoGen(const unsigned seed, const int id, const int nEdge, const int nNodeInGraph) {
	uniform_int_distribution<int> nodeDis(0, nNodeInGraph - 1);
	default_random_engine generator(seed);
	auto gfun = [&]() {return nodeDis(generator); };
	vector<int> used; // used nodes
	//get a new edge starting at an used node and ending at a random node
	auto getNextEdge = [&]() {
		int s, d;
		do {
			s = used.empty() ? gfun() : used[generator() % used.size()];
			d = gfun();
		} while(find(used.begin(), used.end(), d) != used.end() &&
			find(edges.begin(), edges.end(), Edge{ s,d }) != edges.end());
//			(find(edges.begin(), edges.end(), Edge{ s,d }) != edges.end() ||
//				find(edges.begin(), edges.end(), Edge{ d,s }) != edges.end()));
		return make_pair(s,d);
	};
	for(int i = 0; i < nEdge; ++i) {
		pair<int,int> p = getNextEdge();
		if(i == 0)
			used.push_back(p.first);
		used.push_back(p.second);
		edges.push_back(Edge{ p.first, p.second });
	}
	sort(edges.begin(), edges.end());
	this->id = id;
	this->nEdge = nEdge;
	this->nNode = used.size();
}

bool Motif::testExistence(const std::vector<std::vector<bool> >& aMatrix) const {
	bool res = true;
	for(const Edge& e : edges) {
		if(!aMatrix[e.s][e.d]){
			res = false;
			break;
		}
	}
	return res;
}
bool Motif::testExistence(const std::vector<std::vector<int> >& aList) const {
	bool res = true;
	for(const Edge& e : edges) {
		const vector<int>& l = aList[e.s];
		if(find(l.begin(),l.end(),e.d)==l.end()) {
			res = false;
			break;
		}
	}
	return res;
}

void Motif::addEdge(const int s, const int d) {
	edges.push_back(Edge{ s,d });
}

void Motif::compile(){
	//set edges and nEdge
	sort(edges.begin(), edges.end());
	auto it = unique(edges.begin(), edges.end());
	edges.erase(it, edges.end());
	nEdge = edges.size();
	//set nNode
	unordered_set<int> usedNode(edges.size() * 2);
	for(const auto& e : edges) {
		usedNode.insert(e.s);
		usedNode.insert(e.d);
	}
	nNode = usedNode.size();
}

bool Motif::check() const {
	if(!checkNumber())
		return false;
	if(!checkBasicLogic())
		return false;
	if(!checkConnect())
		return false;
	return true;
}
bool Motif::checkNumber() const {
	return nEdge == edges.size();
}
bool Motif::checkBasicLogic() const {
	auto it = edges.begin();
	auto itn = ++it;
	while(itn != edges.end()) {
		if(!(*it < *itn)) //if duplicated then ==; if unorder then >
			return false;
		it = itn;
		++itn;
	}
	return true;
}

static void dfs_mark(const unordered_map<int, vector<int> >& graph,
	unordered_set<int>& visited, const int p) 
{
	for(int i : graph.at(p)) {
		auto it = visited.find(p);
		if(it == visited.end()) {
			visited.insert(i);
			dfs_mark(graph, visited, i);
		}
	}
}

bool Motif::checkConnect() const {
	unordered_map<int, vector<int> > graph(edges.size() * 2);
	for(int i = 0; i < nEdge; ++i) {
		int s = edges[i].s, d = edges[i].d;
		graph[s].push_back(d);
		graph[d].push_back(s);
	}
	unordered_set<int> visited;
	dfs_mark(graph, visited, graph.begin()->first);
	return visited.size() == graph.size();
}


bool operator==(const Motif& lth, const Motif& rth) {
	if(lth.nNode != rth.nNode || lth.nEdge != rth.nEdge)
		return false;
	for(int i = 0; i < lth.nNode; ++i) {
		if(lth.edges[i] != rth.edges[i])
			return false;
	}
	return true;
}


