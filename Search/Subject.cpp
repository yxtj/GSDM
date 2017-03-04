#include "stdafx.h"
#include "Subject.h"

using namespace std;

struct Subject::SDSignature{

};

Subject::Subject()
	: psign(nullptr)
{
//	fCE = bind(&Subject::_contain_e_normal, this, placeholders::_1);
//	fCM = bind(&Subject::_contain_m_normal, this, placeholders::_1);
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

void Subject::setTheta(const double theta)
{
	th = static_cast<int>(ceil(gs.size()*theta));
}

bool Subject::contain(const Edge & e) const
{
	return fCE(e);
}

bool Subject::contain(const Motif & m) const
{
	return fCM(m);
}

bool Subject::_contain_e_normal(const Edge & e) const
{
	int req = th;
	for(auto& g : gs) {
		//++stNumGraphChecked;
		if(g.testEdge(e))
			if(--req <= 0)
				return true;
	}
	return false;
}

bool Subject::_contain_m_normal(const Motif & m) const
{
	int req = th;
	for(auto& g : gs) {
		//++stNumGraphChecked;
		if(g.testMotif(m))
			if(--req <= 0)
				return true;
	}
	return false;
}
