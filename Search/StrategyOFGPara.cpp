#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "Option.h"
#include "../util/Timer.h"
#include <regex>

using namespace std;

const std::string StrategyOFGPara::name("ofg-para");
const std::string StrategyOFGPara::usageDesc(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyOFGPara::name + " <k> <theta> <obj-fun> <alpha> [sd] [net] [dces] [log]"
);
const std::string StrategyOFGPara::usageParam(
	"  <k>: [integer] return top-k result\n"
	//	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  <dist>: optional [dist/dist-no], default disabled, run in distributed manner\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-c/dces-b/decs-no / dces:<ms>/dces-c:<ms>/dces-b:<ms>/decs:<ms>-no], "
	"default enabled, use dynamic candidate edge set ('c' for connected-condition, 'b' for bound-condition) "
	"<ms> is the minimum frequency of candidate edges, default 0.0\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path"
);
const std::string StrategyOFGPara::usage = StrategyOFGPara::usageDesc + "\n" + StrategyOFGPara::usageParam;

std::vector<Motif> StrategyOFGPara::search(const Option & opt, const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	// step 0: initial network thread & initial message driver
	net = NetworkThread::GetInstance();

	// step 1: register all workers
	registerAllWorkers();

	// step 2: initial candidate edge set
	initialCE_para();

	// step 3: search independently (each worker holds motif whose score is larger than current k-th)
	TopKHolder<Motif, score_t> res = work_para();

	// step 4: coordinate the top-k motifs of all workers
	cooridnateTopK();

	// step 5: merge statistics to Rank 0
	gatherStatistics();

	return std::vector<Motif>();
}

void StrategyOFGPara::registerAllWorkers()
{
	// broadcast online signal

	// wait until receive all onine signals from all others (using ImmediateHandler)
	// error if not finished within XX seconds
}

void StrategyOFGPara::initialCE_para()
{
}

TopKHolder<Motif, StrategyOFGPara::score_t> StrategyOFGPara::work_para()
{
	return TopKHolder<Motif, score_t>(k);
}

void StrategyOFGPara::cooridnateTopK()
{
}

void StrategyOFGPara::gatherStatistics()
{
}
