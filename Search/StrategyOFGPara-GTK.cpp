#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"

/*
Global Top-k & bound
*/

using namespace std;

void StrategyOFGPara::topKCoordinate()
{
	ostringstream oss;
	auto tt = holder->getScore();
	oss << logHeadID("GTK-Coord") << tt.size() << " { ";
	for(auto& v : tt)
		oss << v << " ";
	oss << "}";
	cout << oss.str() << endl;
	if(net->id() != MASTER_ID) {
		// On workers: send local top-k to master
		net->send(MASTER_ID, MType::GGatherLocalTopK, holder->getScore());
		// the rest calculattion work will be done by master 
		// global top-k will be received later and handled by cbUpdateLowerBound()
	} else {
		// On master: initialize current local top-k
		globalTopKScores.update(holder->getScore(), net->id());
		// then wait for workers' GGatherLocalTopK message and 
		//   process them with cbLocalTopK()
		rph.input(MType::GGatherLocalTopK, net->id());
		// after received the GGatherLocalTopK messages from all the workers,
		//   finish this process with topKCoordinateFinish
	}
}

void StrategyOFGPara::topKCoordinateFinish()
{
	rph.resetTypeCondition(MType::GGatherLocalTopK);
	if(globalTopKScores.full()) {
		updateLowerBound(globalTopKScores.lowest(), true, true);
	}
	cout << logHead("LOG") + "Global top-k coordination finished, LB="
		+ to_string(globalBound) << endl;
}

void StrategyOFGPara::topKMerge(const std::vector<double>& recv, const int source)
{
	lock_guard<mutex> lg(mgtk);
	globalTopKScores.update(recv, source);
}

// -------------- Lower Bound Maintaince ----------------

void StrategyOFGPara::initLowerBound()
{
	// move lb down half a unit to accept every thing at the beginning
	double lb = get<1>(edges.back()) - 0.5 / pgp->size();
	updateLowerBound(lb, true, true);
}

void StrategyOFGPara::updateLowerBound(double newLB, bool modifyTables, bool fromLocal) {
	if(newLB > globalBound) {
		// this epsilon checking is IMPORTANT.
		//   because in float numbers: (3.0/100 - 2.0/100) > (2.0/100 - 1.0/100)
		// this method may loose the bound but does not lose anyone qualified
		if(!(newLB - numeric_limits<double>::epsilon() > globalBound))
			return;
		newLB -= numeric_limits<double>::epsilon();
		// normal logic:
		globalBound = newLB;
		updateLBCandEdge(newLB);
		if(modifyTables)
			updateLBWaitingMotifs(newLB);
		cout << logHeadID("DBG") + "LB changed to " + to_string(globalBound)
			+ (fromLocal ? " by local" : " by remote") << endl;
	}
	if(fromLocal) {
		lowerBoundSend();
	} else {
		updateLBResult(newLB);
	}
}

int StrategyOFGPara::updateLBCandEdge(double newLB)
{
	lock_guard<mutex> lg(mce);
	auto it = upper_bound(edges.begin(), edges.end(), newLB,
		[](double th, const tuple<Edge, double, int>& p) {
		return th > get<1>(p);
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
	net->broadcast(MType::GLowerBound, globalBound);
}

