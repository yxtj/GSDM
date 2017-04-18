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

	Timer twm; // timer for updating the bound for waiting motifs
	Timer tct; // timer for coordinating global top-k
	Timer tsr; // timer for state report
	// TODO: add time calculation for each task during report interval
	Timer t; // timer for controlling working frequency (INTERVAL_PROCESS)
	//INTERVAL_COORDINATE_TOP_K = 1000;
	// work on the activated motifs
	processLevelFinish();
	while(!checkSearchFinish()) {
		// process all activated motifs
		int cnt = 0;
		while(!ltable.emptyActivated() && cnt < BATCH_SIZE_ACTIVE) {
			bool b;
			std::pair<Motif, double> mu;
			tie(b, mu) = ltable.getOne(); // implicit lock here
			if(!b) { // the check after lock
				break;
			} else if(mu.second < globalBound) {
				continue;
			}
			++cnt;
			// update lower bound
			if(explore(mu.first)) {
				if(holder->size() >= k && holder->lastScore() >= globalBound) {
					bool modifyTables = false;
					if(static_cast<int>(twm.elapseMS()) > INTERVAL_UPDATE_WAITING_MOTIFS) {
						modifyTables = true;
						twm.restart();
					}
					updateLowerBound(holder->lastScore(), modifyTables, true);
				}
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
				st.nMotifSend += vec.size();
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
			st.timeWait += e;
			this_thread::sleep_for(chrono::milliseconds(e));
			t.restart();
		}
	}
	// send level finish signal if not send before
	processLevelFinish();
	// send search finish signal
	rph.input(MType::GSearchFinish, id);
	net->broadcast(MType::GSearchFinish, *lastFinishLevel);
	cout << logHeadID("LOG") + "Local search finished." << endl;
	st.timeSearch = t.elapseMS();
	{
		t.restart();
		suSearchEnd.wait();
		st.timeWait += t.elapseMS();
	}
}

bool StrategyOFGPara::explore(const Motif & m)
{
	bool used = false;
	MotifBuilder mb(m);
	++st.nMotifExplored;
	double ub, score;
	{
		Timer t;
		tie(ub, score) = scoring(mb, globalBound);
		st.timeData += t.elapseMS();
	}
	// score == numeric_limits<double>::lowest() if the upper bound is not promising
	if(holder->updatable(score)) {
		lock_guard<mutex> lg(mtk);
		holder->update(m, score);
		used = true;
	}
	if(ub >= globalBound) {
		for(auto& mf : expand(m, ub, true)) {
			generalUpdateCandidateMotif(mf.first, mf.second);
		}
	} else {
		//for(auto& mf : expand(m, ub, false)) {
		//	generalAbandonCandidateMotif(mf.first);
		//}
	}
	return used;
}

std::pair<double, double> StrategyOFGPara::scoring(const MotifBuilder & mb, const double lowerBound)
{
	// TODO: optimize with parent selection and marked SD checking
	double freqPos = 0.0;
	if(!pdp->empty()) {
		int cntPos = pdp->count(mb);
		freqPos = static_cast<double>(cntPos) / pdp->size();
	}
	double scoreUB = freqPos;
	// freqPos is the upperbound of differential & ratio based objective function
	//if(freqPos < minSup || scoreUB <= lowerBound)
	if(scoreUB <= lowerBound)
		return make_pair(numeric_limits<double>::lowest(), numeric_limits<double>::lowest());
	// calculate the whole score
	double freqNeg = 0.0;
	if(!pdn->empty()) {
		int cntNeg = pdn->count(mb);
		freqNeg = static_cast<double>(cntNeg) / pdn->size();
	}
	return make_pair(scoreUB, objFun(freqPos, freqNeg));
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
			//++stNumMotifGenerated;
			++st.nMotifGenerated;
		}
	}
	return res;
}

bool StrategyOFGPara::processLevelFinish()
{
	bool moved = false;
	int oldlfl = *lastFinishLevel;
	while(checkLocalLevelFinish(*lastFinishLevel + 1)) {
		ltable.sortUp(*lastFinishLevel + 1);
		lock_guard<mutex> lg(mfl);
		++finishedAtLevel[net->id()]; // local finish
//		cout << logHeadID("DBG") + "Change finish-level to " + to_string(*lastFinishLevel) << endl;
		moved = true;
	}
	if(moved) {
		// conditions:
		// 1, local worker finished level lfl
		// 2, all other workers finished level lfl-1
		// 3, NOT all of other worker finished level lfl (if so, lfl will be increased by the loop)
		moveToNewLevel(oldlfl);
		net->broadcast(MType::GLevelFinish, *lastFinishLevel);
	}
	return moved;
}

void StrategyOFGPara::moveToNewLevel(const int from)
{
	cout << logHeadID("LOG") + "Moved from level " + to_string(from) + 
		" to level " + to_string(*lastFinishLevel) << endl;
	if(flagDCESConnected) {
		edgeUsageSend(from);
		// require: all edge usages before level $from$ are already received
		removeUnusedEdges(*lastFinishLevel - 1);
	}
	ltable.sortUp(*lastFinishLevel);
	const int size = net->size();
	const int id = net->id();
	for(int i = 0; i < size; ++i) {
		if(i == id)
			continue;
		rtables[i].sortUp(*lastFinishLevel);
	}
}

bool StrategyOFGPara::checkLocalLevelFinish(const int level)
{
	if(ltable.emptyActivated(level)) {
		int ec = count_if(finishedAtLevel.begin(), finishedAtLevel.end(),
			[=](const int l) {
			return l >= level - 1;
		});
		if(net->size() == ec) {
			return true;
		}
	}
	return false;
/*	return ltable.emptyActivated(level)
		&& net->size() == count_if(
			finishedAtLevel.begin(), finishedAtLevel.end(), [=](const int l) {
		return l >= level - 1;
	});*/
}

bool StrategyOFGPara::checkSearchFinish()
{
	// correctness requirement: normal messages arrive before level-finish messages
	if(ltable.getNumEverActive(*lastFinishLevel + 1) == 0) {
		int ec= count_if(finishedAtLevel.begin(), finishedAtLevel.end(),
			[=](const int l) {
			return l >= *lastFinishLevel;
		});
		if(net->size() == ec) {
			return true;
		}
	}
	return false;
/*	return ltable.getNumEverActive(*lastFinishLevel + 1) == 0
		&& net->size() == count_if(
			finishedAtLevel.begin(), finishedAtLevel.end(), [=](const int l) {
		return l >= *lastFinishLevel;
	});*/
}
