#include "stdafx.h"
#include "Subject.h"
#include "SDSignature.h"

using namespace std;

unsigned long long Subject::nGraphChecked = 0;

Subject::Subject()
	: psign(nullptr)
{
}

Subject::~Subject()
{
	delete psign;
}

void Subject::addGraph(const Graph & g)
{
	gs.push_back(g);
}

void Subject::addGraph(Graph && g)
{
	gs.push_back(move(g));
}

void Subject::addGraph(const std::vector<Graph>& gs)
{
	for(auto& g : gs)
		this->gs.push_back(g);
}

void Subject::addGraph(std::vector<Graph>&& gs)
{
	for(auto& g : gs)
		this->gs.push_back(move(g));
}

void Subject::setGraph(const std::vector<Graph>& gs)
{
	this->gs = gs;
}

void Subject::setGraph(std::vector<Graph>&& gs)
{
	this->gs = move(gs);
}

size_t Subject::size() const
{
	return gs.size();
}

bool Subject::empty() const
{
	return gs.empty();
}

int Subject::getnNode() const
{
	return gs.empty() ? 0 : gs.front().nNode;
}

std::vector<Graph>& Subject::get()
{
	return gs;
}

const std::vector<Graph>& Subject::get() const
{
	return gs;
}

Graph & Subject::get(const int idx)
{
	return gs[idx];
}

const Graph & Subject::get(const int idx) const
{
	return gs[idx];
}

void Subject::initSignature()
{
	psign = generateSignature();
}

void Subject::setTheta(const double theta)
{
	th = static_cast<int>(ceil(gs.size()*theta));
}

int Subject::nNodeQuick() const
{
	return gs.front().nNode;
}

bool Subject::contain_normal(const Edge & e) const
{
	int req = th;
	for(auto& g : gs) {
		++nGraphChecked;
		if(g.testEdge(e))
			if(--req <= 0)
				return true;
	}
	return false;
}

bool Subject::contain_normal(const MotifBuilder & m) const
{
	int req = th;
	for(auto& g : gs) {
		++nGraphChecked;
		if(g.testMotif(m))
			if(--req <= 0)
				return true;
	}
	return false;
}

bool Subject::contain_sd(const Edge & e) const
{
	return (*psign)[e.s][e.d] <= 1;
}

bool Subject::contain_sd(const MotifBuilder & m, const SDSignature & ms) const
{
	if(!checkSDNecessary(m, ms))
		return false;
	return contain_normal(m);
}

unsigned long long Subject::getnGraphChecked()
{
	return nGraphChecked;
}

SDSignature* Subject::generateSignature()
{
	int n = nNodeQuick();
	vector<vector<vector<int>>> buf(n, vector<vector<int>>(n, vector<int>(gs.size())));
	for(size_t k = 0; k < gs.size(); ++k) {
		//std::vector<std::vector<int>> m = calA2AShortestDistance(gs[k]);
		SDSignature m(gs[k]);
		for(int i = 0; i < n; ++i)
			for(int j = 0; j < n; ++j)
				buf[i][j][k] = m[i][j];
	}
	SDSignature* p = new SDSignature(n);
	SDSignature& res = *p;
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j) {
			nth_element(buf[i][j].begin(), buf[i][j].begin() + th - 1, buf[i][j].end());
			res.sd[i][j] = buf[i][j][th - 1];
		}
	return p;
}

bool Subject::checkSDNecessary(const MotifBuilder& m, const SDSignature & ms) const
{
	// all edges should: sdis(e;g) <= sdis(e;m)
	for(const Edge& e : m.edges) {
		if(ms.sd[e.s][e.d] < (*psign)[e.s][e.d])
			return false;
	}
	return true;
}

SDSignature * Subject::getSignature()
{
	return psign;
}

const SDSignature * Subject::getSignature() const
{
	return psign;
}

void Subject::setSignature(const SDSignature & sign)
{
	psign = new SDSignature(sign);
}

void Subject::setSignature(SDSignature && sign)
{
	psign = new SDSignature(sign);
}
