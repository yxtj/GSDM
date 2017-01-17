#pragma once
#include "StrategyOFG.h"
#include "../net/NetworkThread.h"
#include "../msgdriver/MsgDriver.h"

class StrategyOFGPara :
	public StrategyOFG
{
	NetworkThread* net;
	MsgDriver driver;
	using callback_t = void(StrategyOFGPara::*)(const std::string&, const RPCInfo&); //= MsgDriver::callback_t;
	using score_t = double;
public:
	static const std::string name;
	static const std::string usage;
	static const std::string usageDesc, usageParam;

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	
	// Steps
private:
	void registerAllWorkers();
	void initialCE_para();
	TopKHolder<Motif, score_t> work_para();
	void cooridnateTopK();
	void gatherStatistics();

	// helpers for registering callbacks
private:
	void regDSPImmediate(const int type, callback_t fp);
	void regDSPProcess(const int type, callback_t fp);
	void regDSPDefault(callback_t fp);

	// callbacks 
public:
	void cbRegisterWorker(const std::string& d, const RPCInfo& info);


};

