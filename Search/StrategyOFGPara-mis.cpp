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
	if(net->id() == MASTER_ID) {
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
	{
		Timer t;
		suTKGather.wait();
		st.timeWait += t.elapseMS();
	}
	suTKGather.reset();
}
void StrategyOFGPara::resultMerge(std::vector<std::pair<Motif, double>>& recv)
{
	lock_guard<mutex> lg(mtk);
	for(auto& p : recv) {
		holder->update(move(p.first), p.second);
	}
}


// -------------- Statistics -----------------

void StrategyOFGPara::gatherStatistics()
{
	if(flagStatDump && net->id() == MASTER_ID) {
		if(statBuff.size() < static_cast<size_t>(net->size())) {
			statBuff.resize(net->size());
			statBuff[MASTER_ID] = st;
		}
	}
	if(net->id() == MASTER_ID) {
		statReceive();
	} else {
		statSend();
	}
}

void StrategyOFGPara::statSend()
{
	net->send(MASTER_ID, MType::SGather, st);
}

void StrategyOFGPara::statReceive()
{
	rph.input(MType::SGather, net->id());
	{
		Timer t;
		suStat.wait();
		st.timeWait += t.elapseMS();
	}
	st.average(net->size());
}

void StrategyOFGPara::statMerge(const int source, Stat& recv)
{
	if(flagStatDump) {
		if(statBuff.size() < static_cast<size_t>(net->size())) {
			statBuff.resize(net->size());
			statBuff[MASTER_ID] = st;
		}
		statBuff[source] = move(recv);
	}
	st.merge(recv);
}

void StrategyOFGPara::statFormatOutput(std::ostream & os, const Stat & st)
{
	os << "  Motifs: explored " << st.nMotifExplored << " , generated " << st.nMotifGenerated
		<< " , ratio: " << (double)st.nMotifExplored / st.nMotifGenerated << "\n";
	os << "  Freqencies: on positive " << st.nFreqPos << " , on negative " << st.nFreqNeg
		<< " , ratio: " << (double)st.nFreqNeg / st.nFreqPos << "\n";
	os << "  Subjects: " << st.nSubjectChecked << " , Graphs: " << st.nGraphChecked << "\n";
	os << "  G-ratio: " << (double)st.nGraphChecked / st.nSubjectChecked << " graph/subject"
		<< " , F-ratio: " << (double)st.nSubjectChecked / (st.nFreqPos + st.nFreqNeg) << " subject/frequency"
		<< " , M-ratio: " << (double)st.nSubjectChecked / (st.nMotifExplored + st.nEdgeChecked) << " subject/motif\n";
	os << "  Network: send (KB) " << st.netByteSend / 1024 << ", receive (KB) " << st.netByteRecv / 1024 << "\n";
	os << "    send (motif) " << st.nMotifSend << " , receive (motif) " << st.nMotifRecv << "\n"
		<< "    send (bound) " << st.boundSend << " , send (local top-k) " << st.topkSend << "\n";
	os << "  Time: total (s) " << st.timeTotal / 1000 << ", search (s) " << st.timeSearch / 1000
		<< ", wait (ms) " << st.timeWait << ", scoring/data (s) " << st.timeData / 1000 << "\n";
}

void StrategyOFGPara::statDump()
{
	int size = net->size();
	ofstream fout(pathStatDump);
	if(!fout) {
		cerr << "[Warn] Cannot open stat file: " + pathOutputScore << endl;
		return;
	}
	fout << "[On Average]:\n";
	statFormatOutput(fout, st);
	for(int i = 0; i < size; ++i) {
		fout << "\n[Worker " << i << "]:\n";
		statFormatOutput(fout, statBuff[i]);
	}
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
		<< ", #-local-result: " << holder->size()
		<< "; last-score: " << holder->lastScore() << ", bound: " << globalBound
		<< "\n last-finished-level: " << *lastFinishLevel
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
