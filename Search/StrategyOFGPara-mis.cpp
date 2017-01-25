#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"

/*
Miscellaneous:
	1, result
	2, statistics
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
