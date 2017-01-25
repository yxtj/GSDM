#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"

/*
Global Top-k & bound
*/

using namespace std;

void StrategyOFGPara::topKCoordinate()
{
	if(net->id() != MASTER_ID) {
		net->send(MASTER_ID, MType::GGatherLocalTopK, holder->getScore());
		// the rest work will be done by cbRecvLocalTopK() and topKCoordinateFinish()
	} else {
		topKMerge(holder->getScore(), net->id());
	}
}

void StrategyOFGPara::topKCoordinateFinish()
{
	rph.resetTypeCondition(MType::GGatherLocalTopK);
	net->broadcast(MType::GLowerBound, lowerBound);
	updateLBCandEdge(lowerBound);
	updateLBResult(lowerBound);
	updateLBWaitingMotifs(lowerBound);
}

void StrategyOFGPara::topKMerge(const std::vector<double>& recv, const int source)
{
	vector<pair<double, int>> temp;
	auto it = back_inserter(temp);
	int cnt = 0;
	lock_guard<mutex> lg(mgtk);
	auto first1 = globalTopKScores.begin(), last1 = globalTopKScores.end();
	auto first2 = recv.begin(), last2 = recv.end();
	// replace the entries with the same source & sort up
	//   Implemented by ignoring the entries in gTopKScoures with the same source
	while(first1 != last1 && first2 != last2 && cnt <= k) {
		if(first1->second == source) {
			++first1;
		} else if(first1->first <= *first2) {
			*it++ = *first1++;
			++cnt;
		} else {
			*it++ = make_pair(*first2++, source);
			++cnt;
		}
	}
	while(cnt <= k && first1 != last1) {
		*it++ = *first1++;
		++cnt;
	}
	while(cnt <= k && first2 != last2) {
		*it++ = make_pair(*first2++, source);
		++cnt;
	}
	globalTopKScores = move(temp);
	if(!globalTopKScores.empty())
		lowerBound = max(lowerBound, globalTopKScores.back().first);
}

// -------------- Lower Bound Maintaince ----------------

void StrategyOFGPara::initLowerBound()
{
	// move lb down half a unit to accept every thing at the beginning
	double lb = get<1>(edges.back()) - 0.5 / pgp->size();
	updateLowerBound(lb, true, false);
}

void StrategyOFGPara::updateLowerBound(double newLB, bool modifyTables, bool fromLocal) {
	if(newLB > lowerBound) {
		lowerBound = newLB;
		updateLBCandEdge(newLB);
		if(modifyTables)
			updateLBWaitingMotifs(newLB);
	}
	if(fromLocal) {
		updateLBResult(newLB);
		lowerBoundSend();
	}
}

int StrategyOFGPara::updateLBCandEdge(double newLB)
{
	lock_guard<mutex> lg(mce);
	auto it = upper_bound(edges.begin(), edges.end(), newLB,
		[](double th, const tuple<Edge, double, int>& p) {return th > get<1>(p);
	});
	int num = distance(it, edges.end());
	edges.erase(it, edges.end());
	return num;
}

int StrategyOFGPara::updateLBResult(double newLB)
{
	lock_guard<mutex> lg(mtk);
	return holder->updateBound(newLB);
}

int StrategyOFGPara::updateLBWaitingMotifs(double newLB)
{
	int size = net->size();
	int id = net->id();
	int count = ltable.updateLowerBound(newLB);
	for(int i = 0; i < size; ++i) {
		if(i != id) {
			rtables[i].updateLowerBound(newLB);
		}
	}
	return count;
}

void StrategyOFGPara::lowerBoundSend()
{
	net->broadcast(MType::GLowerBound, lowerBound);
}

