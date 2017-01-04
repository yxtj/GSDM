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
	"  <k>: [integer] return top-k result"
//	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-no/dces-c/dces-b], default disabled, use dynamic candidate edge set "
	"('c' for connected-condition, 'b' for bound-condition)\n"
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
		flagDCESConnected = false;
		flagDCESBound = false;
		flagOutputScore = false;
		regex reg("(sd|net|dces(-[cb])?|log(:.+)?)(-no)?");
		for(size_t i = 5; i < param.size(); ++i) {
			//const string& str = param[i];
			smatch m;
			if(regex_match(param[i], m, reg)) {
				bool flag = !m[4].matched;
				string name = m[1].str();
				if(name == "sd") {
					flagUseSD = flag;
				} else if(name == "net") {
					flagNetworkPrune = flag;
				} else if(name.find("dces") != string::npos) {
					parseDCES(m[2], flag);
				} else { //if(name.substr(3) == "log")
					parseLOG(m[3], flag);
				}
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

std::vector<Motif> StrategyOFG::search(const Option & opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
	// initialization
	pgp = &gPos;
	pgn = &gNeg;
	nSubPosGlobal = opt.nPosInd;
	nSubNegGlobal = opt.nNegInd;
	//nMinSup = static_cast<int>((nSubPosGlobal + nSubNegGlobal)*minSup);
	//nMinSup = static_cast<int>(nSubPosGlobal*minSup);
	nNode = gPos[0][0].nNode;
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
	//if(net.getSize() == 1) {
		//res = method_edge1_bfs();
		//res = master(net);
	//} else {
		//if(net.getRank() == 0) {
		//	res = master(net);
		//} else {
		//	numMotifExplored = slave(net);
		//}
	//}
	auto ts = timer.elapseS();
//	cout << "  Rank " << net.getRank() << " finished in " << ts << " seconds\n"
	int oldFlag = cout.setf(ios::fixed);
	auto oldPrec = cout.precision(2);
	cout << "  Finished in " << ts << " seconds\n"
		<< "    motif explored " << stNumMotifExplored << " , generated " << stNumMotifGenerated << "\n"
		<< "    subject counted: " << stNumSubjectChecked << " , graph counted: " << stNumGraphChecked
		<<" , on average: "<< ios::fixed<<(double)stNumGraphChecked/stNumSubjectChecked << " graph/subject\n"
		<< "    frequency calculated on positive: " << stNumFreqPos << " , on negative: " << stNumFreqNeg << endl;
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

void StrategyOFG::initStatistics()
{
	stNumMotifExplored = 0;
	stNumMotifGenerated = 0;
	stNumGraphChecked = 0;
	stNumSubjectChecked = 0;
	stNumFreqPos = 0;
	stNumFreqNeg = 0;
}

void StrategyOFG::parseDCES(const ssub_match & param, const bool flag)
{
	if(param.matched && flag) {
		throw invalid_argument("wrong option for dces");
	} else if(!param.matched) {
		flagDCESConnected = flagDCESBound = flag;
	} else {
		string opt = param.str();
		if(opt.find("c") != string::npos) {
			flagDCESConnected = true;
			flagDCESBound = false;
		} else { //if(opt.find("b") != string::npos)
			flagDCESConnected = false;
			flagDCESBound = true;
		}
	}
}

void StrategyOFG::parseLOG(const ssub_match & param, const bool flag)
{
	flagOutputScore = flag;
	if(flag) {
		string path = param.str();
		if(path.size() < 2)
			throw invalid_argument("log path of Strategy " + name + " is not give.");
		else
			pathOutputScore = path.substr(1);
	}

}

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
