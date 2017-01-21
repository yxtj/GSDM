#pragma once
#include "StrategyOFG.h"
#include "TopKBoundedHolder.hpp"
#include "LocalTables.h"
#include "RemoteTable.h"
#include "../net/NetworkThread.h"
#include "../msgdriver/MsgDriver.h"
#include "../msgdriver/tools/ReplyHandler.h"
#include "../msgdriver/tools/SyncUnit.h"

class StrategyOFGPara :
	public StrategyOFG
{
	NetworkThread* net;
	MsgDriver driver;
	bool running_;
	ReplyHandler rph;
	using callback_t = void(StrategyOFGPara::*)(const std::string&, const RPCInfo&);
private:
	// candidate edge set
	std::mutex mce;
	std::vector<std::pair<Edge, double>> edges;
	// top-k results:
	std::mutex mtk;
	TopKBoundedHolder<Motif, double>* holder;
	// local tables
	LocalTables ltable; // candidate tables (one per level) + activation table
	// remote table buffers
	std::vector<RemoteTable> rtables; // one for each remote worker
	// score to prune with
	double lowerBound;
public:
	static const std::string name;
	static const std::string usage;
	static const std::string usageDesc, usageParam;

	StrategyOFGPara() = default;

	virtual bool parse(const std::vector<std::string>& param);

	// initialize and search
	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	
	// Steps
private:
	void initParams(
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	void initLRTables();
	void initHandlers();

	SyncUnit suReg, suStart;
	void registerAllWorkers();

	SyncUnit suCEinit;
	void initialCE_para();

	bool searchFinished;
	void work_para();

	void cooridnateTopK();

	void gatherStatistics();

	// helpers for registering callbacks
	void regDSPImmediate(const int type, callback_t fp);
	void regDSPProcess(const int type, callback_t fp);
	void regDSPDefault(callback_t fp);
	
/* helpers */
private:
	std::pair<int, int> num2Edge(const int idx);
	int getMotifOwner(const Motif& m);

	std::pair<std::vector<Motif>, double> explore(const Motif& m);
	std::vector<Motif> expand(const Motif& m);
//	static int getNParents(const MotifBuilder& m);
	static int getNParents(const Motif& m);

	void assignBeginningMotifs();

	void generalUpdateCandidateMotif(const Motif& m, const double ub); //local + buffer for net
	void generalAbandonCandidateMotif(const Motif& m);

/* helpers should be public: */
public:
	// message looper
	void messageReceiver();
	// callbacks:
	void cbRegisterWorker(const std::string& d, const RPCInfo& info);
	void cbStart(const std::string& d, const RPCInfo& info);

	void cbCEInit(const std::string& d, const RPCInfo& info);
	void cbCERemove(const std::string& d, const RPCInfo& info);

	void cbUpdateLowerBound(const std::string& d, const RPCInfo& info);

	void cbRecvCandidateMotifs(const std::string& d, const RPCInfo& info);
	void cbRecvAbandonedMotifs(const std::string& d, const RPCInfo& info);

/* detial functions callbacks (usually be able to used both locally and by net) */
private:
	int updateThresholdCE(double newLB);
	int updateThresholdResult(double newLB);
	int updateThresholdWaitingMotifs(double newLB);
};

