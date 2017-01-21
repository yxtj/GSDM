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

	// update lowerbound
	regDSPProcess(MType::TUpdateLB, &StrategyOFGPara::cbUpdateLowerBound);
	
	// normal motifs
	regDSPProcess(MType::MNormal, &StrategyOFGPara::cbRecvCandidateMotifs);
	// abandoned motifs
	regDSPProcess(MType::MAbondan, &StrategyOFGPara::cbRecvAbandonedMotifs);
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
	vector<pair<Edge, double>> temp = deserialize<vector<pair<Edge, double>>>(d);
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
	auto it = remove_if(edges.begin(), edges.end(), [&](const pair<Edge, double>& p) {
		return find(re.begin(), re.end(), p.first) != re.end();
	});
	edges.erase(it, edges.end());
}

void StrategyOFGPara::cbUpdateLowerBound(const std::string & d, const RPCInfo & info)
{
	double newLB = deserialize<double>(d);
	lowerBound = newLB;
	updateThresholdCE(newLB);
	updateThresholdResult(newLB);
	updateThresholdWaitingMotifs(newLB);
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

int StrategyOFGPara::updateThresholdCE(double newLB)
{
	lock_guard<mutex> lg(mce);
	auto it = upper_bound(edges.begin(), edges.end(), newLB,
		[](double th, const pair<Edge, double>& p) {return th < p.second; });
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


