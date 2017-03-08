#include "stdafx.h"
#include "StrategyPeriodic.h"

using namespace std;

const std::string StrategyPeriodic::name("period");
const std::string StrategyPeriodic::usage(
	"Select the discriminative and periodic motifs as result.\n"
	"Usage: " + name + " <k> <theta> <obj-fun> <alpha> [sd] [net] [dces] [npar] [log] [stat]\n"
	"  <k>: [integer] return top-k result\n"
	"  <theta>: [double] the minimum show up probability and the period (on all n-snapshot*<theta> parts) of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [name:para] name for the objective function (" + ObjFunction::usage + ")\n"
	//"  <dist>: optional [dist/dist-no], default disabled, run in distributed manner\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-c/dces-b/decs-no](:<ms>), "
	"default enabled, use dynamic candidate edge set ('c' for connected-condition, 'b' for bound-condition) "
	"<ms> is the minimum frequency of candidate edges, default 0.0\n"
	"  [npar]: opetional [npar-estim/npar-exact], default estimated, which way used to calculated the number of parents of a motif\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path\n"
	"  [stat]: optional [stat:<path>/stat-no], default disabled, dump the statistics of all workers to a file"
);

bool StrategyPeriodic::parse(const std::vector<std::string>& param)
{
	vector<string> par(param);
	par[0] = "ofg-para";
	return StrategyOFGPara::parse(par);
}

std::pair<double, double> StrategyPeriodic::scoring(
	const MotifBuilder & mb, const double lowerBound)
{
	int cntPos = pdp->countByPeriod(mb);
	double freqPos = static_cast<double>(cntPos) / pdp->size();
	double scoreUB = freqPos;
	// freqPos is the upperbound of differential & ratio based objective function
	//if(freqPos < minSup || scoreUB <= lowerBound)
	if(scoreUB <= lowerBound)
		return make_pair(numeric_limits<double>::lowest(), numeric_limits<double>::lowest());
	// calculate the how score
	int cntNeg = pdn->count(mb);
	double freqNeg = static_cast<double>(cntNeg) / pdn->size();
	return make_pair(scoreUB, objFun(freqPos, freqNeg));
}
