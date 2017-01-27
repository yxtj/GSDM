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
	if(net->id() == 0) {
		statReceive();
	} else {
		statSend();
	}
}

void StrategyOFGPara::statSend()
{
	vector<unsigned long long> stat = {
		stNumMotifExplored,
		stNumMotifGenerated,
		stNumGraphChecked,
		stNumSubjectChecked,
		stNumFreqPos,
		stNumFreqNeg
	};
	net->send(MASTER_ID, MType::SGather, stat);
}

void StrategyOFGPara::statReceive()
{
	rph.input(MType::SGather, net->id());
	suStat.wait();
}

void StrategyOFGPara::statMerge(std::vector<unsigned long long>& recv)
{
	stNumMotifExplored += recv[0];
	stNumMotifGenerated += recv[1];
	stNumGraphChecked += recv[2];
	stNumSubjectChecked += recv[3];
	stNumFreqPos += recv[4];
	stNumFreqNeg += recv[5];
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
		<< ", LB: " << lowerBound << "; #-local-result: " << holder->size()
		<< ", last-score: " << holder->lastScore()
		<< "; last-finished-level: " << lastFinishLevel
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
	// nFinishedLevel
	oss << "\n    # of finished workers on each level: {";
	for(int i = 0; i < mostRecent; ++i)
		oss << " " << nFinishLevel[i];
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

	cout << oss.str() << endl;
}
