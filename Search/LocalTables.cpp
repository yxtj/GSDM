#include "stdafx.h"
#include "LocalTables.h"

using namespace std;


void LocalTables::init(std::function<int(const Motif&)> fn, double LB)
{
	fnGetNParents = fn;
	lowerBound = LB;
}

void LocalTables::update(const Motif & m, const double newUB, const int num)
{
	int l = m.getnEdge();
	CT_t& ct = candidateTables[l];
	auto it = ct.find(m);
	if(it == ct.end()) {
		int np = fnGetNParents(m);
		it = ct.insert(make_pair(move(m), make_pair(newUB, np))).first;
	} else {
		lock_guard<mutex> lg(mcts[l]);
		it->second.first = min(it->second.first, newUB);
	}
	lock_guard<mutex> lg(mcts[l]);
	it->second.second -= num;
	if(it->second.second <= 0) {
		lock_guard<mutex> lga(mat);
		activatedTable.push_back(make_pair(move(m), it->second.first));
		ct.erase(it);
	}

}

void LocalTables::sortUp(const int l)
{
	size_t end = min(static_cast<size_t>(l + 1), candidateTables.size());
	for(size_t i = 0; i < end; ++i) {
		lock_guard<mutex> lg(mcts[i]);
		candidateTables[i].clear();
	}
}

int LocalTables::updateLowerBound(double newLB)
{
	if(lowerBound >= newLB)
		return 0;
	lowerBound = newLB;
	lock_guard<mutex> lg(mat);
	size_t s0 = activatedTable.size();
	activatedTable.remove_if([=](const pair<Motif, double>& p) {
		return p.second < lowerBound;
	});
	return s0 - activatedTable.size();
}

bool LocalTables::emptyActivated()
{
	return activatedTable.empty();
}

std::pair<bool, std::pair<Motif, double>> LocalTables::getOne()
{
	if(activatedTable.empty()) {
		return make_pair(false, make_pair(Motif(), 0.0));
	}
	lock_guard<mutex> lg(mat);
	if(activatedTable.empty()) {
		return make_pair(false, make_pair(Motif(), 0.0));
	}
	auto mu = move(activatedTable.front());
	activatedTable.pop_front();
	return make_pair(true, move(mu));
}
