#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"
#include "../util/Timer.h"

using namespace std;

void StrategyOFGPara::work_para()
{
	// initial the first level
	initLRTables();
	initLowerBound();
	assignBeginningMotifs();

	int id = net->id();
	int size = net->size();

	// level 0 starts as finished by definition
	lastFinishLevel = 0;
	nFinishLevel[lastFinishLevel] = size;

	// work on the activated motifs
	Timer twm; // timer for updating the bound for waiting motifs
	Timer tct; // timer for coordinating global top-k
	Timer tsr; // timer for state report
	Timer t; // timer for controlling working frequency (INTERVAL_PROCESS)
	while(!checkSearchFinish()) {
		// process all activated motifs
		int cnt = 0;
		while(!ltable.emptyActivated() && cnt < BATCH_SIZE_ACTIVE) {
			bool b;
			std::pair<Motif, double> mu;
			tie(b, mu) = ltable.getOne(); // implicit lock here
			if(!b) { // the check after lock
				break;
			} else if(mu.second < lowerBound) {
				continue;
			}
			++cnt;
			// update lower bound
			if(explore(mu.first) && holder->size() == k) {
				bool modifyTables = false;
				if(static_cast<int>(twm.elapseMS()) > INTERVAL_UPDATE_WAITING_MOTIFS) {
					modifyTables = true;
					twm.restart();
				}
				updateLowerBound(holder->lastScore(), modifyTables, true);
			}
		}
		// send remote table buffers
		for(int i = 0; i < size; i++) {
			if(i == id)
				continue;
			auto vec = rtables[i].collect();
			if(!vec.empty()) {
//				cout << logHeadID("DBG") + "Collected " + to_string(vec.size())
//					+ " motifs for " + to_string(i) << endl;
				net->send(i, MType::MNormal, vec);
			}
		}
		// send level finish signal if possible
		processLevelFinish();
		// get the global top-k if necessary
		if(static_cast<int>(tct.elapseMS()) >= INTERVAL_COORDINATE_TOP_K) {
			topKCoordinate();
			tct.restart();
		}
		// report local state
		if(static_cast<int>(tsr.elapseS()) >= INTERVAL_STATE_REPORT) {
			reportState();
			tsr.restart();
		}

		int e = INTERVAL_PROCESS - static_cast<int>(t.elapseMS());
		if(e > 0) {
			this_thread::sleep_for(chrono::milliseconds(e));
			t.restart();
		}
	}
	// send level finish signal if not send before
	processLevelFinish();
	// send search finish signal
	rph.input(MType::GSearchFinish, id);
	net->broadcast(MType::GSearchFinish, lastFinishLevel);
	cout << logHeadID("LOG") + "Local search finished." << endl;
	suSearchEnd.wait();
}

bool StrategyOFGPara::explore(const Motif & m)
{
	bool used = false;
	MotifBuilder mb(m);
	double ub, score;
	tie(ub, score) = scoring(mb, lowerBound);
	// score == numeric_limits<double>::lowest() if the upper bound is not promising
	if(holder->updatable(score)) {
		lock_guard<mutex> lg(mtk);
		holder->update(m, score);
		used = true;
	}
	if(ub >= lowerBound) {
		for(auto& mf : expand(m, ub, true)) {
			generalUpdateCandidateMotif(mf.first, mf.second);
		}
	} else {
		for(auto& mf : expand(m, ub, false))
			generalAbandonCandidateMotif(mf.first);
	}
	return used;
}

std::vector<std::pair<Motif, double>> StrategyOFGPara::expand(
	const Motif & m, const double ub, const bool mark)
{
	vector<pair<Motif, double>> res;
	MotifBuilder mb(m);
	int l = m.getnEdge();
	lock_guard<mutex> lg(mce);
	for(size_t i = 0; i < edges.size(); ++i) {
		const Edge& e = get<0>(edges[i]);
		if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
			Motif t(m);
			t.addEdge(e.s, e.d);
			t.sortUpEdges();
			res.emplace_back(move(t), min(ub, get<1>(edges[i])));
			// update the last-used-level field of edges if mark is set
			if(mark)
				get<2>(edges[i]) = l;
			++stNumMotifGenerated;
		}
	}
	return res;
}

bool StrategyOFGPara::processLevelFinish()
{
	bool moved = false;
	int oldlfl = lastFinishLevel;
	while(checkLevelFinish(lastFinishLevel + 1)) {
		ltable.sortUp(lastFinishLevel + 1);
		lock_guard<mutex> lg(mfl);
		++nFinishLevel[lastFinishLevel + 1]; // local finish
		++lastFinishLevel;
//		cout << logHeadID("DBG") + "Locally changed n-finish-level changed to "
//			+ to_string(nFinishLevel[lastFinishLevel]) + ", at level " + to_string(lastFinishLevel) << endl;
		moved = true;
	}
	if(moved) {
		moveToNewLevel(oldlfl);
		for(int i = oldlfl + 1; i < lastFinishLevel + 1; ++i)
			net->broadcast(MType::GLevelFinish, i);
	}
	return moved;
}

void StrategyOFGPara::moveToNewLevel(const int from)
{
	cout << logHeadID("LOG") + "Moved from level " + to_string(from) + 
		" to level " + to_string(lastFinishLevel) << endl;
	if(flagDCESConnected) {
		edgeUsageSend(from);
		removeUnusedEdges();
	}
	ltable.sortUp(lastFinishLevel);
	const int size = net->size();
	const int id = net->id();
	for(int i = 0; i < size; ++i) {
		if(i == id)
			continue;
		rtables[i].sortUp(lastFinishLevel);
	}
}

bool StrategyOFGPara::checkLevelFinish(const int level)
{
	if(static_cast<int>(nFinishLevel.size()) <= level) {
		lock_guard<mutex> lg(mfl);
		nFinishLevel.resize(max<size_t>(nFinishLevel.size(), level + 1), 0);
		return false;
	}
	// TODO: merge nFinishLevel and finishedAtLevel
	int ec = count_if(finishedAtLevel.begin(), finishedAtLevel.end(), [=](const int l) {
		return l < level;
	});
	if(net->size() == ec + nFinishLevel[level - 1]) {
		if(ltable.emptyActivated(level)) {
			return true;
		}
	}
	return false;
}

bool StrategyOFGPara::checkSearchFinish()
{
	// guarantee: normal messages arrive before level-finish message
	// TODO: merge nFinishLevel and finishedAtLevel
	int ec = count_if(finishedAtLevel.begin(), finishedAtLevel.end(), [=](const int l) {
		return l < lastFinishLevel;
	});
	return net->size() == ec + nFinishLevel[lastFinishLevel]
		&& ltable.getNumEverActive(lastFinishLevel + 1) == 0;
}
