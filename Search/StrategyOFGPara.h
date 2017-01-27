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
	int INTERVAL_PROCESS = 10; //in millisecond
	int INTERVAL_UPDATE_WAITING_MOTIFS = 50; //in millisecond
	int INTERVAL_COORDINATE_TOP_K = 5*1000; //in millisecond
	int BATCH_SIZE_ACTIVE = 100; // num. of active motifs explored at each step
private:
	// candidate edge set
	std::mutex mce;
	std::vector<std::tuple<Edge, double, int>> edges; // <Edge, freq, last-used-level>

	// top-k results:
	std::mutex mtk;
	TopKBoundedHolder<Motif, double>* holder;
	std::mutex mgtk; // global top-k
	std::vector<std::pair<double, int>> globalTopKScores; // globl top-k scores <score, owner>, only used on master
	// score to prune with
	double lowerBound;

	// local tables
	LocalTables ltable; // candidate tables (one per level) + activation table
	std::mutex mfl; // for lastFinsihLevel, nFinishLevel, finishedAtLevel
	int lastFinishLevel;
	std::vector<int> nFinishLevel; // num. of workers which finished all local motifs of level k
	std::vector<int> endAtLevel; // level of each work ends at
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
	void gatherResult();
	
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
	// return new motifs and their upper-bounds ( min(ub, new-edge.ub) )
	std::vector<std::pair<Motif, double>> expand(const Motif& m, const double ub, const bool used);
//	static int getNParents(const MotifBuilder& m);
	static int getNParents(const Motif& m);

	void assignBeginningMotifs();

	void generalUpdateCandidateMotif(const Motif& m, const double ub); //local + buffer for net
	void generalAbandonCandidateMotif(const Motif& m);

	/* Logic for level & search finish while using activation queue mechanishm:
		Condition:	1, all previous workers finished their parts of all previous levels;
					2, there is no more unprocessed active motifs of current level.
		Conclusion:	current worker finishes its part on current level.

	*/
	bool processLevelFinish();
	void moveToNewLevel(const int from); // the tasks need to be done for level movement
	bool checkLevelFinish(const int level);
	bool checkSearchFinish();

	/* Logic for DCES-connected:
		1, [main] Send local edge-usage (last-used-level) at the end of each level.
			Before sending level-finish signal.
		2, [msg] Receive edge-usage and update local usage.
		3, [main] Remove the edges unused until the finished level.
	*/
	void edgeUsageSend(const int since); // send edges used AFTER given level
	void edgeUsageUpdate(const std::vector<std::pair<Edge, int>>& usage);
	void removeUnusedEdges();
	
	/* Logic for DCES-bound & OFG-bound:
		0, [data-structure] Maintain a <score, source> list for global top-k
		1, [main on workers] Periodically send local top-k to master.
		2, [msg on master] Update <score, source> list with received list
			Key trick: only changes the entries from identical source
			=> maintains a correct out-date global top-k
		3, [main on master] Broadcast current updated global k-th score
	*/
	// start a global top-k coordinate process
	void topKCoordinate();
	// a callback to finish a global top-k coordinate process
	void topKCoordinateFinish();
	// replace the entries with the same source & sort up
	void topKMerge(const std::vector<double>& recv, const int source);

	void resultSend();
	void resultReceive();
	void resultMerge(std::vector<std::pair<Motif, double>>& recv);

	void statSend();
	void statReceive();
	void statMerge(std::vector<unsigned long long>& recv);

	void initLowerBound(); // can only be called after CE is ready
	void updateLowerBound(double newLB, bool modifyTables, bool fromLocal);
	int updateLBCandEdge(double newLB);
	int updateLBResult(double newLB);
	int updateLBWaitingMotifs(double newLB);
	void lowerBoundSend();

/* Thread function and callbacks: */
public:
	// message looper
	void messageReceiver();
	// callbacks:
	void cbRegisterWorker(const std::string& d, const RPCInfo& info);
	void cbStart(const std::string& d, const RPCInfo& info);

	void cbCEInit(const std::string& d, const RPCInfo& info);
	void cbCERemove(const std::string& d, const RPCInfo& info);
	void cbCEUsage(const std::string& d, const RPCInfo& info);

	void cbLocalTopK(const std::string& d, const RPCInfo& info);
	void cbUpdateLowerBound(const std::string& d, const RPCInfo& info);

	void cbLevelFinish(const std::string& d, const RPCInfo& info);
	void cbSearchFinish(const std::string& d, const RPCInfo& info);

	void cbRecvCandidateMotifs(const std::string& d, const RPCInfo& info);
	void cbRecvAbandonedMotifs(const std::string& d, const RPCInfo& info);

	void cbGatherResult(const std::string& d, const RPCInfo& info);

	void cbGatherStat(const std::string& d, const RPCInfo& info);

/* log related: */
private:
	std::string logHead(const std::string& head) const;
	std::string logHeadID(const std::string& head) const;
};

