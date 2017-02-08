#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"

/*
Miscellaneous:
	1, result
	2, statistics
	3, log
*/

using namespace std;

// -------------- Result -----------------

void StrategyOFGPara::gatherResult()
{
	if(net->id() == 0) {
		resultReceive();
	} else {
		resultSend();
	}
}

void StrategyOFGPara::resultSend()
{
	vector<pair<Motif, double>> topk = holder->getResultScore();
	net->send(MASTER_ID, MType::MGather, topk);
	cout << logHeadID("LOG") + "Send result: num=" + to_string(topk.size())
		+ ", last-score=" + (topk.empty() ? "nan" : to_string(topk.back().second)) << endl;
}

void StrategyOFGPara::resultReceive()
{
	rph.input(MType::MGather, net->id());
	suTKGather.wait();
	suTKGather.reset();

}
void StrategyOFGPara::resultMerge(std::vector<std::pair<Motif, double>>& recv)
{
	for(auto& p : recv) {
		holder->update(move(p.first), p.second);
	}
}


// -------------- Statistics -----------------

void StrategyOFGPara::gatherStatistics()
{
	if(net->id() == MASTER_ID) {
		statReceive();
	} else {
		statSend();
	}
}

void StrategyOFGPara::statSend()
{
	// TODO: change scan functions to use Stat
	st.nGraphChecked = stNumGraphChecked;
	st.nSubjectChecked = stNumSubjectChecked;
	st.nFreqPos = stNumFreqPos;
	st.nFreqNeg = stNumFreqNeg;
	net->send(MASTER_ID, MType::SGather, st);
}

void StrategyOFGPara::statReceive()
{
	rph.input(MType::SGather, net->id());
	suStat.wait();
}

void StrategyOFGPara::statMerge(Stat& recv)
{
	st.merge(recv);
}

// -------------- Log -----------------

std::string StrategyOFGPara::logHead(const std::string& head) const
{
	return "[" + head + "] ";
}

std::string StrategyOFGPara::logHeadID(const std::string& head) const
{
	return "[" + head + " : " + to_string(net->id()) + "] ";
}

void StrategyOFGPara::reportState() const
{
	ostringstream oss;
	const int size = net->size();
	const int id = net->id();
	// basic
	int mostRecent = ltable.mostRecentLevel();
	oss << logHeadID("PROGRESS") << "#-CE: " << edges.size()
		<< ", LB: " << globalBound << "; #-local-result: " << holder->size()
		<< ", last-score: " << holder->lastScore()
		<< "; last-finished-level: " << *lastFinishLevel
		<< ", most-recent-level: " << mostRecent;
	// local candidate
	oss << "\n    Local candidate: {";
	for(auto v : ltable.getNumCandidates())
		oss << " " << v;
	oss << " }";
	// local activated
	oss << "\n    Local activated: {";
	for(auto v : ltable.getNumActives())
		oss << " " << v;
	oss << " }";
	// remote
	oss << "\n    # of motifs for remote: {";
	for(int i = 0; i < size; ++i)
		oss << " " << i << ":" << rtables[i].size();
	oss << " }";
	// finish marker
	oss << "\n    Finish Marker: {";
	for(int i = 0; i < size; ++i)
		oss << " " << i << ":" << 
		(finishedAtLevel[i] == numeric_limits<int>::max() ? -1 : finishedAtLevel[i]);
	oss << " }";

	oss << "\n";
	cout << oss.str();
	cout.flush();
}
