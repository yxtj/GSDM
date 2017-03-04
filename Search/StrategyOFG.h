#pragma once
#include "StrategyBase.h"
#include "TopKHolder.hpp"
#include <vector>
#include <map>
#include <functional>
#include <regex>

class StrategyOFG :
	public StrategyBase
{
protected:
	// input options:
	size_t k; // number of result
//	int smin, smax; // minimum/maximum motif size
	double minSup; // minimum show up probability among postive subjects
	double pSnap; // minimum show up probability among a subject's all snapshots
	std::string objFunName; // the name for the objective function
	double alpha; // penalty for negative frequency
	bool flagDistributed;

	bool flagUseSD; // whether to use the shortest distance optimization
	bool flagNetworkPrune; // whether to prune the motifs with any invalid parent
	bool flagDCESConnected; // whether to use the dynamic candiate edge set (connect with valid motif in last layer)
	bool flagDCESBound; // whether to use the dynamic candiate edge set (upper bound condition)
	bool flagOutputScore; // whether to output the score of the top-k result
	std::string pathOutputScore; // the path of the score file

// local parameters shared by internal functions (valid during searching is called)
	int objFunID;
	int nNode;
//	int nMinSup;
	std::vector<double> topKScores;
	using subject_t = std::vector<Graph>;
	using dataset_t = std::vector<subject_t>;
	const std::vector<std::vector<Graph>>* pgp, *pgn;

	// statistics
	mutable unsigned long long stNumMotifExplored;
	mutable unsigned long long stNumMotifGenerated;
	mutable unsigned long long stNumGraphChecked;
	mutable unsigned long long stNumSubjectChecked;
	mutable unsigned long long stNumFreqPos;
	mutable unsigned long long stNumFreqNeg;

	using objFun_t = double(StrategyOFG::*)(double, double);
	//objFun_t objFun;
	std::function<double(double, double)> objFun;

	// shortest distance signature
	struct Signature {
		std::vector<std::vector<int>> sd;
		Signature(int n) : sd(n, std::vector<int>(n, 2 * n)) {
			//for(int i = 0; i < n; ++i)
			//	sd[i][i] = 0;
		}
		std::vector<std::vector<int>>::reference operator[](const int idx) { return sd[idx]; }
		std::vector<std::vector<int>>::const_reference operator[](const int idx) const { return sd[idx]; }
	};
	std::vector<Signature> sigPos, sigNeg;

	struct MotifSign {
		Signature sd;
		std::vector<std::pair<int, int>> marker; // the modified edges
		MotifSign(int n) :sd(n) {}
	};

public:
	static const std::string name;
	static const std::string usage;

	StrategyOFG() = default;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	virtual std::vector<Motif> search(const Option& opt,
		const DataHolder& dPos, const DataHolder& dNeg);

	/* Objective Functions: */
protected:
	bool setObjFun(const std::string& name);

	double objFun_diffP2N(const double freqPos, const double freqNeg);
	double objFun_marginP2N(const double freqPos, const double freqNeg);
	double objFun_ratioP2N(const double freqPos, const double freqNeg);

	/* Basic Utility/Functions */
protected:
	void initParams(const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);
	void initStatistics();
	void parseDCES(const std::ssub_match& option, const std::ssub_match& minsup, const bool flag);
	void parseLOG(const std::ssub_match& param, const bool flag);

	bool testEdgeInSub(const int s, const int d, const std::vector<Graph>& graphs) const;
	int countEdgeInSub(const int s, const int d, const std::vector<Graph>& graphs) const;

	bool testEdgeXSub(const int s, const int d, const std::vector<std::vector<Graph>>& subs, const double minPortion) const;
	int countEdgeXSub(const int s, const int d, const std::vector<std::vector<Graph>>& subs) const;

	bool testMotifInSub(const MotifBuilder& m, const std::vector<Graph>& graphs) const;
	int countMotifInSub(const MotifBuilder& m, const std::vector<Graph>& graphs) const;

	bool testMotifXSub(const MotifBuilder& m, const std::vector<std::vector<Graph>>& subs, const double minPortion) const;
	int countMotifXSub(const MotifBuilder& m, const std::vector<std::vector<Graph>>& subs) const;


	/* Objective Function Guided Search */
protected:
	std::vector<Motif> method_edge1_bfs();
	// expand one more layer
	std::map<MotifBuilder, int> _edge1_bfs(
		TopKHolder<Motif, double>& holder, const std::vector<MotifBuilder>& last,
		std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& usedEdge);

	// returns: <upper-bound, score>. if upper-bound < lowerBound, returns -numeric_limits<double>::lowest()
	std::pair<double, double> scoring(const MotifBuilder& mb, const double lowerBound);

	/* Prune with number of valid parents (network-based pruning) */
protected:
	std::pair<std::vector<MotifBuilder>, size_t> sortUpNewLayer(std::map<MotifBuilder, int>& layer);

	std::pair<std::vector<MotifBuilder>, size_t> removeDuplicate(std::map<MotifBuilder, int>& layer);

	// estimate num. of parents by the num. of nodes whose degree is 1
	static int quickEstimiateNumberOfParents(const Motif& m);
	static int quickEstimiateNumberOfParents(const MotifBuilder& m);

	std::pair<std::vector<MotifBuilder>, size_t> pruneWithNumberOfParents(std::map<MotifBuilder, int>& mbs);

	/* Dynamic Candidate Edge Set (DCES) */
	// TODO: add dedicated class for CES (optimize for: random removal, range finding by value)
protected:
	void setDCESmaintainOrder(bool inorder);
	std::vector<Edge> initialCandidateEdges();
	std::vector<std::pair<Edge, double>> getExistedEdges(
		const std::vector<std::vector<Graph>>& subs) const;

	// remove unused edges (on longer connected). returns the number of removed edges
	int maintainDCESConnected_inorder(std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& used);
	int maintainDCESConnected_unorder(std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& used);

	// remove low frequency edges. returns the number of removed edges
	int maintainDCESBound_inorder(std::vector<std::pair<Edge, double>>& edges, const double lowerBound);
	int maintainDCESBound_unorder(std::vector<std::pair<Edge, double>>& edges, const double lowerBound);

	//using maintainDCESConnected_t = int(StrategyOFG::*)(std::vector<std::pair<Edge, double>>&, std::vector<bool>&);
	using maintainDCESConnected_t = std::function<int(std::vector<std::pair<Edge, double>>&, std::vector<bool>&)>;
	maintainDCESConnected_t maintainDCESConnected;

	//using maintainDCESBound_t = int(StrategyOFG::*)(std::vector<std::pair<Edge, double>>&, const double);
	using maintainDCESBound_t = std::function<int(std::vector<std::pair<Edge, double>>&, const double)>;
	maintainDCESBound_t maintainDCESBound;

	/* Valid Subject Set */
protected:

	/* Subject Signature */
protected:
	void setSignature();

	bool testMotifInSubSD(const MotifBuilder& m, const MotifSign& ms, const std::vector<Graph>& sub, const Signature& ss) const;
	int countMotifXSubSD(const MotifBuilder& m, const MotifSign& ms,
		const std::vector<std::vector<Graph>>& subs, const std::vector<Signature>& sigs) const;

	Signature genSignture(const std::vector<Graph>& gs, const double theta);
	std::vector<std::vector<int>> calA2AShortestDistance(const Graph& g); // all source-destination pairs

	void updateMotifSD(MotifSign& ms, const MotifBuilder& mOld, int s, int d);
	//	void calMotifSD(MotifSign& ms, const MotifBuilder& mOld, int s, int d);
	void calMotifSD(MotifSign& ms, const MotifBuilder& m);
	bool checkSPNecessary(const MotifBuilder& m, const MotifSign& ms, const Signature& ss) const;

};

