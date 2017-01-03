#include "stdafx.h"
#include "StrategyOFG.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "../util/Timer.h"
#include <regex>

using namespace std;

const std::string StrategyOFG::name("ofg");
const std::string StrategyOFG::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyOFG::name + " <k> <smin> <smax> <theta> <obj-fun> <alpha> [sd] [net] [dces] [log]\n"
	"  <k>: [integer] return top-k result"
//	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-no/dces-c/dces-b], default enabled, use dynamic candidate edge set "
	"('c' for connected-condition, 'b' for bound-condition)\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path"
);


bool StrategyOFG::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 6, 10, name);
		k = stoi(param[1]);
		smin = stoi(param[2]);
		smax = stoi(param[3]);
//		minSup = stod(param[4]);
		pSnap = stod(param[4]);
		setObjFun(param[5]);
		// TODO: change to use a separated functio to parse the parameters for certain objective function
		if(objFunID == 1)
			alpha = stod(param[6]);
		flagUseSD = true;
		flagNetworkPrune = true;
		flagDCESConnected = true;
		flagDCESBound = true;
		flagOutputScore = false;
		regex reg("(sd|net|dces(-[cb])?|log(:.+)?)(-no)?");
		for(size_t i = 7; i < param.size(); ++i) {
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
					if(m[2].matched && flag) {
						throw invalid_argument("wrong option for dces");
					} else if(!m[2].matched) {
						flagDCESConnected = flagDCESBound = flag;
					} else {
						string opt = m[2].str();
						if(opt.find("c") != string::npos) {
							flagDCESConnected = true;
							flagDCESBound = false;
						} else { //if(opt.find("b") != string::npos)
							flagDCESConnected = false;
							flagDCESBound = true;
						}
					}
				} else { //if(name.substr(3) == "log")
					flagOutputScore = flag;
					if(flag) {
						string path = m[3].str();
						if(path.size() < 2)
							throw invalid_argument("log path of Strategy " + name + " is not give.");
						else
							pathOutputScore = path.substr(1);
					}
				}
			} else {
				throw invalid_argument("Unknown option for strategy FuncFreqSD: " + param[i]);
			}
		}
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
	setDCESmaintainOrder(false);

	if(flagUseSD) {
		cout << "Generating subject signatures..." << endl;
		Timer timer;
		setSignature();
		cout << "  Signatures generated in " << timer.elapseMS() << " ms" << endl;
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
		<< "    motif explored " << stNumMotifExplored << " , generated " << stNumMotifGenerated << " ; "
		<< "subject counted: " << stNumSubjectChecked << " , graph counted: " << stNumGraphChecked
		<<" , on average: "<< ios::fixed<<(double)stNumGraphChecked/stNumSubjectChecked << " graph/subject ; "
		<< "frequency calculated on positive: " << stNumFreqPos << " , on negative: " << stNumFreqNeg << endl;
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

std::vector<Motif> StrategyOFG::method_edge1_bfs()
{
	cout << "Phase 1 (prepare edges)" << endl;
	//vector<Edge> edges = initialCandidateEdges();
	vector<pair<Edge, double>> edges = getExistedEdges(*pgp);
	cout << "  # of edges: " << edges.size() << endl;
//	vector<pair<MotifBuilder, double>> last;
	vector<MotifBuilder> last;
	last.reserve(3 * edges.size());
	for(const pair<Edge, double>& e : edges) {
		MotifBuilder m;
		m.addEdge(e.first.s, e.first.d);
//		double p = gp.matrix[e.s][e.d];
//		last.emplace_back(move(m), p);
		last.push_back(move(m));
	}
	if(last.empty()) {
		return vector<Motif>();
	}

	cout << "Phase 2 (testing motifs layer by layer)" << endl;
	TopKHolder<Motif, double> holder(k);
	for(int s = 2; s <= smax; ++s) {
		Timer timer;
		size_t numLast, numTotal, numUnique;
		numLast = last.size();
		vector<bool> usedEdges(edges.size(), false);
		map<MotifBuilder, int> t = _edge1_bfs(holder, last, edges, usedEdges);
		tie(last, numTotal) = sortUpNewLayer(t);
		numUnique = last.size();
		int nRmvEdge = (this->*maintainDCESConnected)(edges, usedEdges);
		nRmvEdge += (this->*maintainDCESBound)(edges, holder.lastScore());
		auto _time_ms = timer.elapseMS();
		cout << "  motifs of size " << s - 1 << " : " << _time_ms << " ms, on " << numLast << " motifs."
			<< "\tgenerate new " << numUnique << " / " << numTotal << " motifs (valid/total)"
			<<"\t removed edges by connection condition" << nRmvEdge << endl;
		if(last.empty())
			break;
	}

	cout << "Phase 3 (output)" << endl;
	if(flagOutputScore) {
		ofstream fout(pathOutputScore);
		for(auto& p : holder.data) {
			fout << p.second << "\t" << p.first << "\n";
		}
	}
	return holder.getResultMove();
}

std::map<MotifBuilder, int> StrategyOFG::_edge1_bfs(
	TopKHolder<Motif, double>& holder, const std::vector<MotifBuilder>& last,
	const std::vector<std::pair<Edge, double>>& edges, std::vector<bool>& usedEdge)
{
	int layer = last.front().getnEdge();
	std::map<MotifBuilder, int> newLayer;
	for(const auto& mb : last) {
		// work on a motif
		MotifSign ms(nNode);
		++stNumMotifExplored;
		// TODO: optimize with parent selection and marked SD checking
		int cntPos;
		if(flagUseSD) {
			calMotifSD(ms, mb);
			cntPos = countMotifXSubSD(mb, ms, *pgp, sigPos);
			/*Motif m = mb.toMotif();
			if(cntPos != countMotif(m, *pgp)) {
			cout << "unmatch" << endl << "motif:\n";
			for(auto& e : mb.edges)
			cout << "(" << e.s << "," << e.d << ") ";
			cout << "\nNo SD:\n";
			for(size_t i = 0; i < pgp->size(); ++i)
			cout << i << ":" << testMotif(m, pgp->at(i)) << ", ";
			cout << "\nSD:\n";
			for(size_t i = 0; i < pgp->size(); ++i)
			cout << i << ":" << testMotifInSubSD(m, ms, pgp->at(i), sigPos[i]) << ", ";
			}*/
		} else {
			cntPos = countMotifXSub(mb, *pgp);
		}
		++stNumFreqPos;
		double freqPos = static_cast<double>(cntPos) / pgp->size();
		double scoreUB = freqPos;
		// freqPos is the upperbound of differential & ratio based objective function
		//if(freqPos < minSup || scoreUB <= holder.lastScore())
		if(scoreUB <= holder.lastScore())
			continue;
		if(layer >= smin) {
			int cntNeg;
			if(flagUseSD)
				cntNeg = countMotifXSubSD(mb, ms, *pgn, sigNeg);
			else
				cntNeg = countMotifXSub(mb, *pgn);
			++stNumFreqNeg;
			double freqNeg = static_cast<double>(cntNeg) / pgn->size();
			double score = objFun(freqPos, freqNeg);
			holder.update(mb.toMotif(), score);
			//maintainDCESBound(edges,)
		}
		// generate new motifs
		for(size_t i = 0; i < edges.size(); ++i) {
			const Edge& e = edges[i].first;
			if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
				MotifBuilder t(mb);
				t.addEdge(e.s, e.d);
				++newLayer[t];
				usedEdge[i] = true;
				++stNumMotifGenerated;
			}
		}
	}
	return newLayer;
}




