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

	int MASTER_ID = 0;
	int INTERVAL_PROCESS = 10; //milliseconds
	int INTERVAL_UPDATE_WAITING_MOTIFS = 50; //milliseconds
private:
	// candidate edge set
	std::mutex mce;
	std::vector<std::pair<Edge, double>> edges; // TODO: <Edge, freq, last-used-level>
	// top-k results:
	std::mutex mtk;
	TopKBoundedHolder<Motif, double>* holder;
	// score to prune with
	double lowerBound;

	// local tables
	LocalTables ltable; // candidate tables (one per level) + activation table
	std::mutex mnfl;
	int lastFinishLevel;
	std::vector<int> nFinishLevel; // num. of workers which finished all local motifs of level k
	// remote table buffers
	std::vector<RemoteTable> rtables; // one for each remote worker
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

	SyncUnit suSearchEnd;
	void work_para();

	SyncUnit suTKGather;
	void cooridnateTopK();
	
	SyncUnit suStat;
	void gatherStatistics();

	// helpers for registering callbacks
	void regDSPImmediate(const int type, callback_t fp);
	void regDSPProcess(const int type, callback_t fp);
	void regDSPDefault(callback_t fp);
	
/* helpers */
private:
	std::pair<int, int> num2Edge(const int idx);
	int getMotifOwner(const Motif& m);

	bool explore(const Motif& m);
	std::vector<Motif> expand(const Motif& m, const bool used);
//	static int getNParents(const MotifBuilder& m);
	static int getNParents(const Motif& m);

	void assignBeginningMotifs();

	void generalUpdateCandidateMotif(const Motif& m, const double ub); //local + buffer for net
	void generalAbandonCandidateMotif(const Motif& m);

	bool checkNSendLevelFinishSignal();
	bool checkNSendLevelFinishSignal(const int level);
	bool checkSearchFinish();

	void removeUnusedEdges();

	void topKSend();
	void topKReceive();

	void statSend();
	void statReceive();

/* helpers which should be public: */
public:
	// message looper
	void messageReceiver();
	// callbacks:
	void cbRegisterWorker(const std::string& d, const RPCInfo& info);
	void cbStart(const std::string& d, const RPCInfo& info);

	void cbCEInit(const std::string& d, const RPCInfo& info);
	void cbCERemove(const std::string& d, const RPCInfo& info);

	void cbUpdateLowerBound(const std::string& d, const RPCInfo& info);

	void cbLevelFinish(const std::string& d, const RPCInfo& info);
	void cbSearchFinish(const std::string& d, const RPCInfo& info);

	void cbRecvCandidateMotifs(const std::string& d, const RPCInfo& info);
	void cbRecvAbandonedMotifs(const std::string& d, const RPCInfo& info);

	void cbRecvTopK(const std::string& d, const RPCInfo& info);

	void cbRecvStat(const std::string& d, const RPCInfo& info);

/* detailed functions for callbacks (usually be able to used both locally and by net) */
private:
	int updateThresholdCE(double newLB);
	int updateThresholdResult(double newLB);
	int updateThresholdWaitingMotifs(double newLB);

	void topKMerge(std::vector<std::pair<Motif, double>>& recv);
	void statMerge(std::vector<unsigned long long>& recv);
};

