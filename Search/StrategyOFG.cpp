#include "stdafx.h"
#include "StrategyOFG.h"
#include "Option.h"
#include "../util/Timer.h"
#include <regex>

using namespace std;

const std::string StrategyOFG::name("ofg");
const std::string StrategyOFG::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyOFG::name + " <k> <theta> <obj-fun> <alpha> [sd] [net] [dces] [log]\n"
	"  <k>: [integer] return top-k result\n"
//	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  <dist>: optional [dist/dist-no], default disabled, run in distributed manner\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-c/dces-b/decs-no](:<ms>), "
	"default enabled, use dynamic candidate edge set ('c' for connected-condition, 'b' for bound-condition) "
	"<ms> is the minimum frequency of candidate edges, default 0.0\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path"
);


bool StrategyOFG::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 4, 8, name);
		k = stoi(param[1]);
//		smin = stoi(param[2]);
//		smax = stoi(param[3]);
//		minSup = stod(param[4]);
		pSnap = stod(param[2]);
		if(!setObjFun(param[3]))
			throw invalid_argument("Unsupported objective function for Strategy " + name + " : " + param[3]);
		// TODO: change to use a separated functio to parse the parameters for certain objective function
		if(objFunID == 1)
			alpha = stod(param[4]);
		flagUseSD = true;
		flagNetworkPrune = true;
		flagDCESConnected = true;
		flagDCESBound = true;
		minSup = 0.0;
		flagOutputScore = false;
		regex reg_sd("sd(-no)?");
		regex reg_net("net(-no)?");
		regex reg_dces("dces(-[cb])?(-no)?(:0\\.\\d+)?");
		regex reg_log("log(:.+)?(-no)?");
		for(size_t i = 5; i < param.size(); ++i) {
			smatch m;
			if(regex_match(param[i], m, reg_sd)) {
				bool flag = !m[1].matched;
				flagUseSD = flag;
			} else if(regex_match(param[i], m, reg_net)) {
				bool flag = !m[1].matched;
				flagNetworkPrune = flag;
			} else if(regex_match(param[i], m, reg_dces)) {
				bool flag = !m[2].matched;
				parseDCES(m[1], m[3], flag);
			} else if(regex_match(param[i], m, reg_log)) {
				bool flag = !m[2].matched;
				parseLOG(m[1], flag);
			} else {
				throw invalid_argument("Unknown option for strategy " + name + ": " + param[i]);
			}
		}
		// TODO: consider whether to put in an option
		setDCESmaintainOrder(false);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyOFG::search(const Option & opt, DataHolder & dPos, DataHolder & dNeg)
{
	if(!checkInput(dPos, dNeg))
		return std::vector<Motif>();
	// initialization
	initParams(dPos, dNeg);
	initStatistics();

	if(flagUseSD) {
		cout << "Generating subject signatures..." << endl;
		Timer timer;
		setSignature();
		cout << "  Signatures generated in " << timer.elapseS() << " s" << endl;
	}

	cout << "Enumeratig..." << endl;
	vector<Motif> res;
	Timer timer;
	res = method_edge1_bfs();
	auto ts = timer.elapseS();

	auto oldFlag = cout.setf(ios::fixed);
	auto oldPrec = cout.precision(2);
	auto stNumFreqPos = dPos.getnMotifChecked();
	auto stNumFreqNeg = dNeg.getnMotifChecked();
	auto stNumEdgeChecked = dPos.getnEdgeChecked() + dNeg.getnEdgeChecked();
	auto stNumSubjectChecked = dPos.getnSubjectChecked() + dNeg.getnSubjectChecked();
	auto stNumGraphChecked = Subject::getnGraphChecked();
	cout << "  Finished in " << ts << " seconds\n";
	cout << "  Motifs: explored " << stNumMotifExplored << " , generated " << stNumMotifGenerated
		<< " , ratio: " << (double)stNumMotifExplored / stNumMotifGenerated << "\n";
	cout << "  Freqencies: on positive " << stNumFreqPos << " , on negative " << stNumFreqNeg
		<< " , ratio: " << (double)stNumFreqNeg / stNumFreqPos << "\n";
	cout << "  Subjects: " << stNumSubjectChecked << " , Graphs: " << stNumGraphChecked << "\n";
	cout << "  G-ratio: " << (double)stNumGraphChecked / stNumSubjectChecked << " graph/subject"
		<< " , F-ratio: " << (double)stNumSubjectChecked / (stNumFreqPos + stNumFreqNeg) << " subject/frequency"
		<< " , M-ratio: " << (double)stNumSubjectChecked / (stNumMotifExplored + stNumEdgeChecked) << " subject/motif\n";

	cout.precision(oldPrec);
	cout.setf(oldFlag);
	return res;
}

bool StrategyOFG::setObjFun(const std::string & name)
{
	if(name == "diff") {
		//objFun = &StrategyOFG::objFun_diffP2N;
		objFun = bind(&StrategyOFG::objFun_diffP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 1;
		return true;
	} else if(name == "margin") {
		objFun = bind(&StrategyOFG::objFun_marginP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 2;
		return true;
	} else if(name == "ratio") {
		//objFun = &StrategyFuncFreqSP::objFun_ratioP2N;
		objFun = bind(&StrategyOFG::objFun_ratioP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 3;
		return true;
	}
	return false;
}

double StrategyOFG::objFun_diffP2N(const double freqPos, const double freqNeg)
{
	return freqPos - alpha*freqNeg;
}

double StrategyOFG::objFun_marginP2N(const double freqPos, const double freqNeg)
{
	return (1.0 - freqPos) + alpha*freqNeg;
}

double StrategyOFG::objFun_ratioP2N(const double freqPos, const double freqNeg)
{
	//return freqNeg != 0.0 ? freqPos / freqNeg : freqPos;
	return freqPos*freqPos / (freqPos + freqNeg);
}

void StrategyOFG::initParams(DataHolder& dPos, DataHolder& dNeg)
{
	dPos.setTheta(pSnap);
	dNeg.setTheta(pSnap);
	pdp = &dPos;
	pdn = &dNeg;
	//nMinSup = static_cast<int>((nSubPosGlobal + nSubNegGlobal)*minSup);
	//nMinSup = static_cast<int>(nSubPosGlobal*minSup);
	nNode = dPos.getnNode();
}

void StrategyOFG::initStatistics()
{
	stNumMotifExplored = 0;
	stNumMotifGenerated = 0;
}

void StrategyOFG::parseDCES(const ssub_match & option, const ssub_match & minsup, const bool flag)
{
	if(!option.matched) {
		flagDCESConnected = flagDCESBound = flag;
	} else {
		string opt = option.str();
		if(opt.find("c") != string::npos) {
			flagDCESConnected = flag;
		} else { //if(opt.find("b") != string::npos)
			flagDCESBound = flag;
		}
	}
	if(minsup.matched) {
		minSup = stod(minsup.str().substr(1));
	}
}

void StrategyOFG::parseLOG(const ssub_match & param, const bool flag)
{
	flagOutputScore = flag;
	if(flag) {
		pathOutputScore = param.str().substr(1);
	}

}

void StrategyOFG::setSignature()
{
	pdp->initSignature();
	pdn->initSignature();
}

/*
bool StrategyOFG::testEdgeInSub(const int s, const int d, const std::vector<Graph>& graphs) const
{
	int th = static_cast<int>(ceil(graphs.size()*pSnap));
	// return true if #occurence >= th
	for(auto& g : graphs) {
		++stNumGraphChecked;
		if(g.testEdge(s, d))
			if(--th <= 0)
				return true;
	}
	return false;
}

int StrategyOFG::countEdgeInSub(const int s, const int d, const std::vector<Graph>& graphs) const
{
	int cnt = 0;
	for(auto& g : graphs) {
		++stNumGraphChecked;
		if(g.testEdge(s, d))
			++cnt;
	}
	return cnt;
}

bool StrategyOFG::testEdgeXSub(const int s, const int d,
	const std::vector<std::vector<Graph>>& subs, const double minPortion) const
{
	int th = static_cast<int>(ceil(subs.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : subs) {
		++stNumSubjectChecked;
		if(testEdgeInSub(s, d, sub)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

int StrategyOFG::countEdgeXSub(const int s, const int d, const std::vector<std::vector<Graph>>& subs) const
{
	int cnt = 0;
	for(auto& sub : subs) {
		++stNumSubjectChecked;
		if(testEdgeInSub(s, d, sub)) {
			++cnt;
		}
	}
	return cnt;
}

bool StrategyOFG::testMotifInSub(const MotifBuilder& m, const std::vector<Graph>& graphs) const
{
	int th = static_cast<int>(ceil(graphs.size()*pSnap));
	// return true if #occurence >= th
	for(auto& g : graphs) {
		++stNumGraphChecked;
		if(g.testMotif(m))
			if(--th <= 0)
				return true;
	}
	return false;
}

int StrategyOFG::countMotifInSub(const MotifBuilder& m, const std::vector<Graph>& graphs) const
{
	int cnt = 0;
	for(auto& g : graphs) {
		++stNumGraphChecked;
		if(g.testMotif(m))
			++cnt;
	}
	return cnt;
}

bool StrategyOFG::testMotifXSub(const MotifBuilder & m,
	const std::vector<std::vector<Graph>>& subs, const double minPortion) const
{
	int th = static_cast<int>(ceil(subs.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : subs) {
		++stNumSubjectChecked;
		if(testMotifInSub(m, sub)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

int StrategyOFG::countMotifXSub(const MotifBuilder & m, const std::vector<std::vector<Graph>>& subs) const
{
	int cnt = 0;
	for(auto& sub : subs) {
		++stNumSubjectChecked;
		if(testMotifInSub(m, sub))
			++cnt;
	}
	return cnt;

}
*/