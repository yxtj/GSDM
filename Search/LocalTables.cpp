#include "stdafx.h"
#include "LocalTables.h"

using namespace std;


void LocalTables::init(std::function<int(const Motif&)> fn, double LB)
{
	fnGetNParents = fn;
	lowerBound = LB;
	candidateTables.reserve(16);
	nActLevel.reserve(16);
	nActLevelTotal.reserve(16);
}

void LocalTables::update(const Motif & m, const double newUB, const int num)
{
	int l = m.getnEdge();
	lock_guard<mutex> lg(mct);
	// add new level
	if(static_cast<int>(candidateTables.size()) <= l ) {
		size_t len = max(candidateTables.size(), nActLevel.size());
		len = max<size_t>(len, l + 1);
		candidateTables.resize(len);
		lock_guard<mutex> lga(mat);
		nActLevel.resize(len);
		nActLevelTotal.resize(len);
	}
	// update the num. left for activation
	CT_t& ct = candidateTables[l];
	auto it = ct.find(m);
	if(it == ct.end()) {
		// the max(0,...) here is for the special case where fnGetNParents(m)-num < 0
		int np = max(0, fnGetNParents(m) - num);
		it = ct.insert(make_pair(move(m), make_pair(newUB, np))).first;
	} else {
		it->second.first = min(it->second.first, newUB);
		it->second.second -= num;
	}
	// move a motif from candiate to active
	if(it->second.second == 0) {
		if(it->second.first >= lowerBound) {
			lock_guard<mutex> lga(mat);
			activatedTable.push_back(make_pair(move(it->first), it->second.first));
			++nActLevel[l];
			++nActLevelTotal[l];
		}
		// Special Case: when using estimated num. parents algorithm:
		//   #-left may be negative. If remove it the first time, it may be activated again.
		//   Therefore, instead of remove it, I set its #-left to a very large number.
		// TODO: distinguish the method for calculating num. partents, exact type & estimated type
		//ct.erase(it);
		it->second.second = numeric_limits<int>::max();
	}

}

void LocalTables::addToActivated(const Motif & m, const double newUB)
{
	int l = m.getnEdge();
	lock_guard<mutex> lga(mat);
	// add new level
	if(static_cast<int>(nActLevel.size()) <= l) {
		size_t len = max<size_t>(nActLevel.size(), l + 1);
		nActLevel.resize(len);
		nActLevelTotal.resize(len);
	}
	activatedTable.push_back(make_pair(m, newUB));
	++nActLevel[l];
	++nActLevelTotal[l];
}

void LocalTables::sortUp(const int l)
{
	lock_guard<mutex> lg(mct);
	size_t end = min(static_cast<size_t>(l + 1), candidateTables.size());
	for(size_t i = 1; i < end; ++i) {
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

int LocalTables::mostRecentLevel() const
{
	return candidateTables.size();
}

bool LocalTables::emptyCandidate() const
{
	for(size_t i = 0; i < candidateTables.size(); ++i) {
		if(emptyCandidate(i))
			return true;
	}
	return false;
}

bool LocalTables::emptyCandidate(const int level) const
{
	return static_cast<int>(candidateTables.size()) > level
		&& candidateTables[level].empty();
}

bool LocalTables::emptyActivated() const
{
	return activatedTable.empty();
}

bool LocalTables::emptyActivated(const int level) const
{
	return static_cast<int>(nActLevel.size()) > level
		&& nActLevel[level] == 0;
}

int LocalTables::getNumCandidate() const
{
	int res = 0;
	size_t n = candidateTables.size();
	for(size_t i = 0; i < n; ++i)
		res += candidateTables[i].size();
	return res;
}

std::vector<int> LocalTables::getNumCandidates() const
{
	size_t n = candidateTables.size();
	std::vector<int> res(n);
	for(size_t i = 0; i < n; ++i)
		res[i] = candidateTables[i].size();
	return res;
}

int LocalTables::getNumCandidate(const int level) const
{
	return static_cast<int>(candidateTables.size()) > level
		? candidateTables[level].size() : 0;
}

int LocalTables::getNumActive() const
{
	return activatedTable.size();
}

std::vector<int> LocalTables::getNumActives() const
{
	return nActLevel;
}

int LocalTables::getNumActive(const int level) const
{
	return static_cast<int>(nActLevel.size()) > level
		? nActLevel[level] : 0;
}

int LocalTables::getNumEverActive(const int level) const
{
	return static_cast<int>(nActLevelTotal.size()) > level ?
		nActLevelTotal[level] : 0;
}

bool LocalTables::empty() const
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

bool LocalTables::empty(const int level) const
{
	unique_lock<mutex> ulc(mct, defer_lock);
	unique_lock<mutex> ula(mat, defer_lock);
	lock(ulc, ula);
	return candidateTables[level].empty() && nActLevel[level] == 0;
}

