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
	regDSPProcess(MType::CEUpdateThreshold, &StrategyOFGPara::cbCEUpdateThreshold);
	regDSPProcess(MType::CERemove, &StrategyOFGPara::cbCERemove);
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

void StrategyOFGPara::cbCEUpdateThreshold(const std::string & d, const RPCInfo & info)
{
	double th = deserialize<double>(d);
	lock_guard<mutex> lg(mce);
	auto it = upper_bound(edges.begin(), edges.end(), th,
		[](double th, const pair<Edge, double>& p) {return th < p.second; });
	edges.erase(it, edges.end());
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

