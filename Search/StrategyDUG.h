#pragma once
#include "StrategyBase.h"
#include "GraphProb.h"

// Strategy for Discriminative Uncertain Graph
class StrategyDUG
	: public StrategyBase
{
	int k;
	std::string dmethod;
	std::string smethod;
	double phi; // for phi-probability method
	double minSup;
	int smin, smax;
	//
	using DisSoreFun_t = double (*)(int, int, int, int);
	DisSoreFun_t disScoreFun;
	using StatSumFun_t = std::function<double(const std::vector<double>&, const std::vector<double>&)>;
	StatSumFun_t statSumFun;
public:
	static const std::string name;
	static const std::string usage;

	virtual bool parse(const std::vector<std::string>& param);

	virtual std::vector<Motif> search(const Option& opt,
		const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg);

private:
	bool parseDMethod();
	bool parseSMethod();
	std::vector<GraphProb> getUGfromCGs(const std::vector<std::vector<Graph>>& gs);

	double probMotifOnUG(const MotifBuilder& m, const GraphProb& ug);
	// DP method
	std::vector<double> disMotifOnUDataset(const Motif& m, std::vector<GraphProb>& ugs);
	
// Discriminative Score Functions:
private:
	static double dsfConfindence(int cMotifPos, int cMotifNeg, int cPos, int cNeg);
	static double dsfFreqRatio(int cMotifPos, int cMotifNeg, int cPos, int cNeg);
	static double dsfGtest(int cMotifPos, int cMotifNeg, int cPos, int cNeg);
	static double dsfHSIC(int cMotifPos, int cMotifNeg, int cPos, int cNeg);

// Statistical Summary Functions:
private:
	double ssfExp(const std::vector<double>& disPos, const std::vector<double>& disNeg);
	double ssfMedian(const std::vector<double>& disPos, const std::vector<double>& disNeg);
	double ssfMode(const std::vector<double>& disPos, const std::vector<double>& disNeg);
	double ssfPhiProb(const std::vector<double>& disPos, const std::vector<double>& disNeg);

// Candidate enumerator:
private:
	std::vector<Edge> StrategyDUG::getEdges(const GraphProb & gp);
	std::vector<Motif> method_edge2_dp(const GraphProb& ugall,
		const std::vector<GraphProb>& ugp, const std::vector<GraphProb>& ugn);
	std::vector<std::pair<MotifBuilder, double>> _edge2_dp(
		const std::vector<std::pair<MotifBuilder, double>>& last, const Edge& e);
private:
	// local parameters shared by internal functions (valid during searching is called)
	int nNode;
	int minSupN;
	std::vector<GraphProb>* pugp, *pugn;
	GraphProb* pugall;
};

