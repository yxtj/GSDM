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
	regDSPProcess(MType::GGatherLocalTopK, &StrategyOFGPara::cbLocalTopK);
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
	regDSPProcess(MType::MGather, &StrategyOFGPara::cbGatherResult);
	rph.addType(MType::MGather, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suTKGather));

	// statistics gathering
	regDSPProcess(MType::SGather, &StrategyOFGPara::cbGatherStat);
	rph.addType(MType::SGather, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, size),
		bind(&SyncUnit::notify, &suStat));
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

void StrategyOFGPara::cbLocalTopK(const std::string & d, const RPCInfo & info)
{
	vector<double> localK = deserialize<vector<double>>(d);
	topKMerge(localK, info.source);
	rph.input(MType::GGatherLocalTopK, info.source);
}

void StrategyOFGPara::cbUpdateLowerBound(const std::string & d, const RPCInfo & info)
{
	double newLB = deserialize<double>(d);
	updateLowerBound(newLB, true, false);
}

void StrategyOFGPara::cbLevelFinish(const std::string & d, const RPCInfo & info)
{
	int level = deserialize<int>(d);
	lock_guard<mutex> lg(mfl);
	finishedAtLevel[info.source] = max(finishedAtLevel[info.source], level);
//	cout << logHeadID("DBG") + "Receive level finish signal from "
//		+ to_string(info.source) + " as " + to_string(level) << endl;
}

void StrategyOFGPara::cbSearchFinish(const std::string & d, const RPCInfo & info)
{
	int endLevel = deserialize<int>(d);
	{
		lock_guard<mutex> lg(mfl);
		//finishedAtLevel[info.source] = endLevel;
		using T = decltype(finishedAtLevel)::value_type;
		finishedAtLevel[info.source] = numeric_limits<T>::max();
	}
	rph.input(MType::GSearchFinish, info.source);
}

void StrategyOFGPara::cbRecvCandidateMotifs(const std::string & d, const RPCInfo & info)
{
	vector<pair<Motif, pair<double, int>>> temp = 
		deserialize<vector<pair<Motif, pair<double, int>>>>(d);
	st.nMotifRecv += temp.size();
	for(auto& mp : temp) {
		ltable.update(mp.first, mp.second.first, mp.second.second);
	}
}

void StrategyOFGPara::cbRecvAbandonedMotifs(const std::string & d, const RPCInfo & info)
{
	static constexpr double WORSTSCORE = numeric_limits<double>::lowest();
	vector<Motif> temp = deserialize<vector<Motif>>(d);
	st.nMotifRecv += temp.size();
	for(auto& m : temp) {
		ltable.update(m, WORSTSCORE);
	}
}

void StrategyOFGPara::cbGatherResult(const std::string & d, const RPCInfo & info)
{
	vector<pair<Motif, double>> recv = deserialize<vector<pair<Motif, double>>>(d);
	resultMerge(recv);
	rph.input(MType::MGather, info.source);
}

void StrategyOFGPara::cbGatherStat(const std::string & d, const RPCInfo & info)
{
	Stat recv = deserialize<Stat>(d);
	statMerge(recv);
	rph.input(MType::SGather, info.source);
}

