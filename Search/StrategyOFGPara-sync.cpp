#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"
#include "../util/Timer.h"

using namespace std;

void StrategyOFGPara::work_para_sync()
{
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
	//INTERVAL_STATE_REPORT = 1;

	// work on the activated motifs
	int currentLevel = 1; // *lastFinishLevel+1;
	reportState();
	vector<pair<Motif, double>> bufferForNext;
	while(!checkSearchFinish()) {
		for(auto& mb : bufferForNext) {
			//cout<<logHeadID("DBG") << mb.first << endl;
			ltable.addToActivated(move(mb.first), move(mb.second));
		}
		bufferForNext.clear();
		// synchronization barrier
		while(syncBarrierChecking()) {
			st.timeWait += INTERVAL_PROCESS;
			this_thread::sleep_for(chrono::milliseconds(INTERVAL_PROCESS));
			t.restart();
		}
		cout << logHeadID("LOG") << "processing level " << currentLevel
			<< " with active motifs: " << ltable.getNumActive() << endl;
		// process current level
		while(!ltable.emptyActivated()) {
			int cnt = 0;
			while(cnt < BATCH_SIZE_ACTIVE) {
				bool b;
				std::pair<Motif, double> mu;
				tie(b, mu) = ltable.getOne(); // implicit lock here
				if(!b) { // the check after lock
					break;
				} else if(mu.second < globalBound) {
					continue;
				} else if(mu.first.getnEdge() != currentLevel) {
					bufferForNext.push_back(move(mu));
					continue;
				}
				++cnt;
				// expand and update lower bound
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
			// get the global top-k if necessary
			if(static_cast<int>(tct.elapseMS()) >= INTERVAL_COORDINATE_TOP_K) {
				topKCoordinate();
				tct.restart();
			}
			// report local state
			/*if(static_cast<int>(tsr.elapseS()) >= INTERVAL_STATE_REPORT) {
				reportState();
				tsr.restart();
			}
			int e = INTERVAL_PROCESS - static_cast<int>(t.elapseMS());
			if(e > 0) {
				st.timeWait += e;
				this_thread::sleep_for(chrono::milliseconds(e));
				t.restart();
			}*/
		}
		// move to next level and send level finish signal
		if(processLevelFinish(false))
			++currentLevel;
		topKCoordinate();
		reportState();
		//cout << logHeadID("DBG") << "active num: " << bufferForNext.size() << endl;
		ltable.sortUp(currentLevel);
	}
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

bool StrategyOFGPara::syncBarrierChecking()
{
	int localLevel = finishedAtLevel[net->id()];
	return localLevel != *min_element(finishedAtLevel.begin(), finishedAtLevel.end());
}
