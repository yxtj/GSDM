#include "stdafx.h"
#include "RemoteTable.h"

using namespace std;

struct RemoteTable::_DataBlock {
	std::mutex m;
	double lowerBound;
	// motif: current-tightest-upper-bound, #-generation
	std::map<Motif, std::pair<double, int>> table;
};

RemoteTable::RemoteTable()
	: pimpl(new _DataBlock())
{
}

RemoteTable::RemoteTable(RemoteTable && oth)
{
	pimpl = nullptr;
	swap(pimpl, oth.pimpl);
}

RemoteTable::~RemoteTable()
{
	delete pimpl;
}

void RemoteTable::init(const double LB)
{
	pimpl->lowerBound = LB;
}

void RemoteTable::sortUp(const int l)
{
	auto& table = pimpl->table;

	auto it = table.begin();
	while(it != table.end()) {
		if(it->first.getnEdge() <= l) {
			it = table.erase(it);
		} else {
			++it;
		}
	}
}

void RemoteTable::update(const Motif & mt, const double newUB)
{
	auto& table = pimpl->table;
	auto& m = pimpl->m;

	auto it = table.find(mt);
	if(it == table.end()) {
		lock_guard<mutex> lg(m);
		table.emplace(mt, make_pair(newUB, 1));
	} else {
		lock_guard<mutex> lg(m);
		it->second.first = min(it->second.first, newUB);
		++it->second.second;
	}
}

void RemoteTable::updateLowerBound(double newLB)
{
	if(pimpl->lowerBound >= newLB)
		return;
	pimpl->lowerBound = newLB;
/*	lock_guard<mutex> lg(m);
	auto it = table.begin();
	while(it != table.end()) {
		if(it->second.second <= newLB) {
			it = table.erase(it);
		} else {
			++it;
		}
	}*/
}

std::vector<std::pair<Motif, std::pair<double, int>>> RemoteTable::collect()
{
	auto& table = pimpl->table;
	auto& m = pimpl->m;
	auto& lowerBound = pimpl->lowerBound;

	vector<pair<Motif, pair<double, int>>> res;
	lock_guard<mutex> lg(m);
	for(auto it = table.begin(); it != table.end(); ++it) {
		if(it->second.second > 0) {
			if(it->second.first >= lowerBound)
				res.push_back(*it);
			it->second.second = 0;
		}
	}
	return res;
}
