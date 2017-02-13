#include "stdafx.h"
#include "RemoteTable.h"

using namespace std;

struct RemoteTable::_DataBlock {
	std::mutex m; // mutex can not be moved or copied
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
	delete pimpl;
	pimpl = nullptr;
	swap(pimpl, oth.pimpl);
	oth.pimpl = new _DataBlock();
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
	// TODO: only sort up the abandon list
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
	if(newUB < pimpl->lowerBound)
		return;
	auto& table = pimpl->table;
	auto& m = pimpl->m;
	// TODO: add an abandon list for non-promising motifs (each layer)

	auto it = table.find(mt);
	lock_guard<mutex> lg(m);
	if(it == table.end()) {
		table.emplace(mt, make_pair(newUB, 1));
	} else {
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
	// TODO: clear those not in the abandon list
	table.clear();
	return res;
}

size_t RemoteTable::size() const
{
	return pimpl->table.size();
}
