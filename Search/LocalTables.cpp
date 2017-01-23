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
	lock_guard<mutex> lg(mct);
	// add new level
	candidateTables.resize(max<size_t>(candidateTables.size(), l + 1));
	{
		lock_guard<mutex> lga(mat);
		activatedTable.resize(max<size_t>(activatedTable.size(), l + 1));
	}
	// update the num. left for activation
	CT_t& ct = candidateTables[l];
	auto it = ct.find(m);
	if(it == ct.end()) {
		int np = fnGetNParents(m) - num;
		it = ct.insert(make_pair(move(m), make_pair(newUB, np))).first;
	} else {
		it->second.first = min(it->second.first, newUB);
		it->second.second -= num;
	}
	// move a motif from candiate to active
	if(it->second.second <= 0) {
		lock_guard<mutex> lga(mat);
		activatedTable.push_back(make_pair(move(it->first), it->second.first));
		++nActLevel[l];
		ct.erase(it);
	}

}

void LocalTables::sortUp(const int l)
{
	lock_guard<mutex> lg(mct);
	size_t end = min(static_cast<size_t>(l + 1), candidateTables.size());
	for(size_t i = 0; i < end; ++i) {
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
	auto it = activatedTable.begin();
	while(it != activatedTable.end()) {
		if(it->second < lowerBound) {
			--nActLevel[it->first.getnEdge()];
			it = activatedTable.erase(it);
		} else {
			++it;
		}
	}
	return s0 - activatedTable.size();
}

bool LocalTables::emptyCandidate()
{
	for(size_t i = 0; i < candidateTables.size(); ++i) {
		if(emptyCandidate(i))
			return true;
	}
	return false;
}

bool LocalTables::emptyCandidate(const int level)
{
	return candidateTables[level].empty();
}

bool LocalTables::emptyActivated()
{
	return activatedTable.empty();
}

bool LocalTables::emptyActivated(const int level)
{
	return nActLevel[level] == 0;
}

bool LocalTables::empty()
{
	unique_lock<mutex> ulc(mct, defer_lock);
	unique_lock<mutex> ula(mat, defer_lock);
	lock(ulc, ula);
	if(activatedTable.empty()) {
		for(auto& ct : candidateTables)
			if(!ct.empty())
				return false;
		return true;
	}
	return false;
}

bool LocalTables::empty(const int level)
{
	unique_lock<mutex> ulc(mct, defer_lock);
	unique_lock<mutex> ula(mat, defer_lock);
	lock(ulc, ula);
	return candidateTables[level].empty() && nActLevel[level] == 0;
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
	--nActLevel[mu.first.getnEdge()];
	activatedTable.pop_front();
	return make_pair(true, move(mu));
}
