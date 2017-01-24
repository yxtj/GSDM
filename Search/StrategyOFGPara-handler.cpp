#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "MType.h"

using namespace std;

void StrategyOFGPara::regDSPImmediate(const int type, callback_t fp) {
	driver.registerImmediateHandler(type, bind(fp, this, placeholders::_1, placeholders::_2));
}
void StrategyOFGPara::regDSPProcess(const int type, callback_t fp) {
	driver.registerProcessHandler(type, bind(fp, this, placeholders::_1, placeholders::_2));
}
void StrategyOFGPara::regDSPDefault(callback_t fp) {
	driver.registerDefaultOutHandler(bind(fp, this, placeholders::_1, placeholders::_2));
}

void StrategyOFGPara::initHandlers()
{
	int size = net->size();
	// online signal
	regDSPImmediate(MType::CReg, &StrategyOFGPara::cbRegisterWorker);
	rph.addType(MType::CReg, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suReg));
	// ready signal
	regDSPProcess(MType::CReady, &StrategyOFGPara::cbStart);	
	rph.addType(MType::CReady, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suStart));

	// candidate edge inititalization
	regDSPProcess(MType::CEInit, &StrategyOFGPara::cbCEInit);
	rph.addType(MType::CEInit, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suCEinit));
	// candidate edge maintain
	regDSPProcess(MType::CERemove, &StrategyOFGPara::cbCERemove);
	regDSPProcess(MType::CEUsage, &StrategyOFGPara::cbCEUsage);

	// maintain global top-k score
	regDSPProcess(MType::GGatherLocalTopK, &StrategyOFGPara::cbRecvTopScore);
	rph.addType(MType::GGatherLocalTopK, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&StrategyOFGPara::topKCoordinateFinish, this));
	// update lowerbound
	regDSPProcess(MType::GLowerBound, &StrategyOFGPara::cbUpdateLowerBound);
	
	// normal motifs
	regDSPProcess(MType::MNormal, &StrategyOFGPara::cbRecvCandidateMotifs);
	// abandoned motifs
	regDSPProcess(MType::MAbondan, &StrategyOFGPara::cbRecvAbandonedMotifs);

	// one level finish
	regDSPProcess(MType::GLevelFinish, &StrategyOFGPara::cbLevelFinish);
	// main search loop finish
	regDSPProcess(MType::GSearchFinish, &StrategyOFGPara::cbSearchFinish);
	rph.addType(MType::GSearchFinish, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suSearchEnd));

	// gather result motifs
	regDSPProcess(MType::MGather, &StrategyOFGPara::cbRecvResult);
	rph.addType(MType::MGather, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suTKGather));

	// statistics gathering
	regDSPProcess(MType::SGather, &StrategyOFGPara::cbRecvStat);
	rph.addType(MType::SGather, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suStart));
}

void StrategyOFGPara::cbRegisterWorker(const std::string & d, const RPCInfo & info)
{
	rph.input(info.tag, info.source);
}

void StrategyOFGPara::cbStart(const std::string & d, const RPCInfo & info)
{
	rph.input(info.tag, info.source);
}

void StrategyOFGPara::cbCEInit(const std::string & d, const RPCInfo & info)
{
	auto temp = deserialize<vector<tuple<Edge, double, int>>>(d);
	{
		lock_guard<mutex> lg(mce);
		move(temp.begin(), temp.end(), back_inserter(edges));
	}
	rph.input(info.tag, info.source);
}


void StrategyOFGPara::cbCERemove(const std::string & d, const RPCInfo & info)
{
	vector<Edge> re = deserialize<vector<Edge>>(d);
	lock_guard<mutex> lg(mce);
	auto it = remove_if(edges.begin(), edges.end(), [&](const tuple <Edge, double, int>& p) {
		return find(re.begin(), re.end(), get<0>(p)) != re.end();
	});
	edges.erase(it, edges.end());
}

void StrategyOFGPara::cbCEUsage(const std::string & d, const RPCInfo & info)
{
	vector<pair<Edge, int>> usage = deserialize<vector<pair<Edge, int>>>(d);
	edgeUsageUpdate(usage);
}

void StrategyOFGPara::cbRecvTopScore(const std::string & d, const RPCInfo & info)
{
	vector<double> localK = deserialize<vector<double>>(d);
	topKMerge(localK, info.source);
	rph.input(MType::GGatherLocalTopK, info.source);
}

void StrategyOFGPara::cbUpdateLowerBound(const std::string & d, const RPCInfo & info)
{
	double newLB = deserialize<double>(d);
	lowerBound = newLB;
	updateThresholdCE(newLB);
	updateThresholdResult(newLB);
	updateThresholdWaitingMotifs(newLB);
}

void StrategyOFGPara::cbLevelFinish(const std::string & d, const RPCInfo & info)
{
	int level = deserialize<int>(d);
	lock_guard<mutex> lg(mfl);
	nFinishLevel.resize(max<size_t>(nFinishLevel.size(), level + 1), 0);
	++nFinishLevel[level];
}

void StrategyOFGPara::cbSearchFinish(const std::string & d, const RPCInfo & info)
{
	int endLevel = deserialize<int>(d);
	{
		lock_guard<mutex> lg(mfl);
		endAtLevel[info.source] = endLevel;
	}
	rph.input(MType::GSearchFinish, info.source);
}

void StrategyOFGPara::cbRecvCandidateMotifs(const std::string & d, const RPCInfo & info)
{
	vector<pair<Motif, pair<double, int>>> temp = 
		deserialize<vector<pair<Motif, pair<double, int>>>>(d);
	for(auto& mp : temp) {
		ltable.update(mp.first, mp.second.first, mp.second.second);
	}
}

void StrategyOFGPara::cbRecvAbandonedMotifs(const std::string & d, const RPCInfo & info)
{
	static constexpr double WORSTSCORE = numeric_limits<double>::lowest();
	vector<Motif> temp = deserialize<vector<Motif>>(d);
	for(auto& m : temp) {
		ltable.update(m, WORSTSCORE);
	}
}

void StrategyOFGPara::cbRecvResult(const std::string & d, const RPCInfo & info)
{
	vector<pair<Motif, double>> recv = deserialize<vector<pair<Motif, double>>>(d);
	ResultMerge(recv);
	rph.input(MType::MGather, info.source);
}

void StrategyOFGPara::cbRecvStat(const std::string & d, const RPCInfo & info)
{
	vector<unsigned long long> recv = deserialize<vector<unsigned long long>>(d);
	statMerge(recv);
	rph.input(MType::SGather, info.source);
}

int StrategyOFGPara::updateThresholdCE(double newLB)
{
	lock_guard<mutex> lg(mce);
	auto it = upper_bound(edges.begin(), edges.end(), newLB,
		[](double th, const tuple<Edge, double, int>& p) {return th < get<1>(p); });
	int num = distance(it, edges.end());
	edges.erase(it, edges.end());
	return num;
}

int StrategyOFGPara::updateThresholdResult(double newLB)
{
	lock_guard<mutex> lg(mtk);
	return holder->updateBound(newLB);
}

int StrategyOFGPara::updateThresholdWaitingMotifs(double newLB)
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

void StrategyOFGPara::ResultMerge(std::vector<std::pair<Motif, double>>& recv)
{
	for(auto& p : recv) {
		holder->update(move(p.first), p.second);
	}
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


