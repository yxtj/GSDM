#include "stdafx.h"
#include "StrategyFuncFreqSD.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "Network.h"
#include "../util/Timer.h"
#include <regex>

using namespace std;

const std::string StrategyFuncFreqSD::name("funcfreqsd");
const std::string StrategyFuncFreqSD::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFuncFreqSD::name + " <k> <smin> <smax> <minSup> <minSnap> <obj-fun> <alpha> [sd] [net] [log]\n"
	"  <k>: [integer] return top-k result"
	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <minSnap>: [double] the minimum show up probability of a motif among a subject's all snapshots\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path"
);

bool StrategyFuncFreqSD::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 7, 10, name);
		k = stoi(param[1]);
		smin = stoi(param[2]);
		smax = stoi(param[3]);
		minSup = stod(param[4]);
		pSnap = stod(param[5]);
//		objFunName = param[6];
		setObjFun(param[6]);
		// TODO: change to use a separated functio to parse the parameters for certain objective function
		if(objFunID == 1)
			alpha = stod(param[7]);
		flagUseSD = true;
		flagNetworkPrune = true;
		flagOutputScore = false;
		regex reg("(sd|net|log(:.+)?)(-no)?");
		for(size_t i = 8; i < param.size(); ++i) {
			//const string& str = param[i];
			smatch m;
			if(regex_match(param[i], m, reg)) {
				bool flag = !m[3].matched;
				string name = m[1].str();
				if(name == "sd")
					flagUseSD = flag;
				else if(name == "net")
					flagNetworkPrune = flag;
				else { //if(name.substr(3) == "log")
					flagOutputScore = flag;
					if(flag) {
						string path = m[2].str();
						if(path.size() < 2)
							throw invalid_argument("log path of Strategy " + name + " is not give.");
						else
							pathOutputScore = m[2].str().substr(1);
					}
				}
			} else {
				throw invalid_argument("Unknown option for strategy " + name + ": " + param[i]);
			}
		}
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyFuncFreqSD::search(const Option & opt, 
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
	// initialization
	pgp = &gPos;
	pgn = &gNeg;
	nSubPosGlobal = opt.nPosInd;
	nSubNegGlobal = opt.nNegInd;
	nMinSup = static_cast<int>((nSubPosGlobal + nSubNegGlobal)*minSup);
	//nMinSup = static_cast<int>(nSubPosGlobal*minSup);
	nNode = gPos[0][0].nNode;
	stNumMotifExplored = 0;
	stNumMotifGenerated = 0;
	stNumSubjectChecked = 0;
	stNumGraphChecked = 0;
	stNumFreqPos = 0;
	stNumFreqNeg = 0;

	if(flagUseSD) {
		cout << "Generating subject signatures..." << endl;
		Timer timer;
		setSignature();
		cout << "  Signatures generated in " << timer.elapseMS() << " ms" << endl;
	}

	cout << "Enumeratig..." << endl;
	Network net;
	vector<Motif> res;
	Timer timer;
	if(net.getSize() == 1) {
		res = method_edge1_bfs();
		//res = master(net);
	} else {
		//if(net.getRank() == 0) {
		//	res = master(net);
		//} else {
		//	numMotifExplored = slave(net);
		//}
	}
	auto ts = timer.elapseS();
	MPI_Barrier(MPI_COMM_WORLD);
	auto oldFlag = cout.setf(ios::fixed);
	auto oldPrec = cout.precision(2);
	cout << "  Rank " << net.getRank() << " finished in " << ts << " seconds\n"
		<< "    motif explored " << stNumMotifExplored << " , generated " << stNumMotifGenerated << "\n"
		<< "    subject counted: " << stNumSubjectChecked << " , graph counted: " << stNumGraphChecked
		<< " , on average: " << (double)stNumGraphChecked / stNumSubjectChecked << " graph/subject\n"
		<< "    frequency calculated on positive: " << stNumFreqPos << " , on negative: " << stNumFreqNeg << endl;
	cout.precision(oldPrec);
	cout.setf(oldFlag);
	return res;
}

bool StrategyFuncFreqSD::setObjFun(const std::string & name)
{
	if(name == "diff") {
		//objFun = &StrategyFuncFreqSP::objFun_diffP2N;
		objFun = bind(&StrategyFuncFreqSD::objFun_diffP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 1;
		return true;
	} else if(name == "margin") {
		objFun = bind(&StrategyFuncFreqSD::objFun_marginP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 2;
		return true;
	} else if(name == "ratio") {
		//objFun = &StrategyFuncFreqSP::objFun_ratioP2N;
		objFun = bind(&StrategyFuncFreqSD::objFun_ratioP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 3;
		return true;
	}
	return false;
}

std::vector<Edge> StrategyFuncFreqSD::getEdges()
{
	vector<Edge> res;
	for(int i = 0; i < nNode; ++i) {
		for(int j = i + 1; j < nNode; ++j) {
			if(checkEdge(i, j))
				res.emplace_back(i, j);
		}
	}
	return res;
}

double StrategyFuncFreqSD::objFun_diffP2N(const double freqPos, const double freqNeg)
{
	return freqPos - alpha*freqNeg;
}

double StrategyFuncFreqSD::objFun_marginP2N(const double freqPos, const double freqNeg)
{
	return (1.0 - freqPos) + alpha*freqNeg;
}

double StrategyFuncFreqSD::objFun_ratioP2N(const double freqPos, const double freqNeg)
{
	//return freqNeg != 0.0 ? freqPos / freqNeg : freqPos;
	return freqPos*freqPos / (freqPos + freqNeg);
}

bool StrategyFuncFreqSD::checkEdge(const int s, const int d, const std::vector<Graph>& sub) const
{
	int th = static_cast<int>(ceil(sub.size()*pSnap));
	int cnt = 0;
	for(auto&g : sub) {
		++stNumGraphChecked;
		if(g.testEdge(s, d)) {
			if(++cnt >= th)
				break;
		}
	}
	return cnt >= th;
}

bool StrategyFuncFreqSD::checkEdge(const int s, const int d) const
{
	int cnt = 0;
	for(auto&sub : *pgp) {
		++stNumSubjectChecked;
		if(checkEdge(s, d, sub))
			if(++cnt >= nMinSup)
				break;
	}
	//if(cnt >= nMinSup)
	//	return true;
	//for(auto&sub : *pgn) {
	//	++stNumSubjectChecked;
	//	if(checkEdge(s, d, sub))
	//		if(++cnt >= nMinSup)
	//			break;
	//}
	return cnt >= nMinSup;
}

int StrategyFuncFreqSD::countEdge(const int s, const int d, const std::vector<std::vector<Graph>>& subs) const
{
	int cnt = 0;
	for(auto&sub : subs) {
		++stNumSubjectChecked;
		if(checkEdge(s, d, sub))
			++cnt;
	}
	return cnt;
}

std::pair<int, int> StrategyFuncFreqSD::countEdge(const int s, const int d) const
{
	int nPos = countEdge(s, d, *pgp);
	int nNeg = countEdge(s, d, *pgn);
	return make_pair(nPos, nNeg);
}

bool StrategyFuncFreqSD::testMotif(const Motif & m, const std::vector<Graph>& sub) const
{
	int th = static_cast<int>(ceil(sub.size()*pSnap));
	int cnt = 0;
	for(auto&g : sub) {
		++stNumGraphChecked;
		if(g.testMotif(m)) {
			if(++cnt >= th)
				break;
		}
	}
	return cnt >= th;
}


int StrategyFuncFreqSD::countMotif(const Motif & m, const std::vector<std::vector<Graph>>& subs) const
{
	int res = 0;
	for(auto&sub : subs) {
		++stNumSubjectChecked;
		if(testMotif(m, sub))
			++res;
	}
	return res;
}

std::pair<int, int> StrategyFuncFreqSD::countMotif(const Motif & m) const
{
	int nPos = countMotif(m, *pgp);
	int nNeg = countMotif(m, *pgn);
//	cout << m << "\t(" << nPos << "," << nNeg << ")" << endl;
//	this_thread::sleep_for(chrono::seconds(1));
	return make_pair(nPos, nNeg);
}


void StrategyFuncFreqSD::removeSupport(slist& sup, std::vector<const subject_t*>& rmv, const Edge& e)
{
	auto itLast = sup.before_begin(), it = sup.begin();
	while(it != sup.end()) {
		const subject_t* s = *it;
		if(!checkEdge(e.s, e.d, *s)) {
			rmv.push_back(s);
			it = sup.eraseAfter(itLast);
		} else {
			itLast = it++;
		}
	}
}

std::vector<Motif> StrategyFuncFreqSD::method_edge1_bfs()
{
	cout << "Phase 1 (prepare edges)" << endl;
	vector<Edge> edges = getEdges();
	cout << "  # of edges: " << edges.size() << endl;
	//	vector<pair<MotifBuilder, double>> last;
	vector<MotifBuilder> last;
	last.reserve(3 * edges.size());
	for(const Edge& e : edges) {
		MotifBuilder m;
		m.addEdge(e.s, e.d);
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
		//vector<MotifBuilder> t = _edge1_bfs(last, holder, edges);
		map<MotifBuilder, int> t = _edge1_bfs(last, holder, edges);
		tie(last, numTotal) = sortUpNewLayer(t);
		numUnique = last.size();
		auto _time_ms = timer.elapseMS();
		cout << "  level " << s - 1 << " : " << _time_ms << " ms, on " << numLast << " motifs."
			<< "\tgenerate: " << numUnique << " / " << numTotal << "."
			<< "\tk-th score: " << holder.lastScore() << endl;
		if(last.empty())
			break;
//		last = move(t);
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

//std::vector<MotifBuilder> StrategyFuncFreqSD::_edge1_bfs(const std::vector<MotifBuilder>& last,
//	TopKHolder<Motif, double>& holder, const std::vector<Edge>& edges)
//{
//	int layer = last.front().getnEdge();
//	std::vector<MotifBuilder> newLayer;
//	for(const auto& mb : last) {
//		// work on a motif
//		MotifSign ms(nNode);
//		++stNumMotifExplored;
//		// TODO: optimize with parent selection and marked SD checking
//		int cntPos;
//		if(flagUseSD) {
//			calMotifSD(ms, mb);
//			cntPos = countMotifSP(mb, ms, *pgp, sigPos);
//			/*Motif m = mb.toMotif();
//			if(cntPos != countMotif(m, *pgp)) {
//				cout << "unmatch" << endl << "motif:\n";
//				for(auto& e : mb.edges)
//					cout << "(" << e.s << "," << e.d << ") ";
//				cout << "\nNo SD:\n";
//				for(size_t i = 0; i < pgp->size(); ++i)
//					cout << i << ":" << testMotif(m, pgp->at(i)) << ", ";
//				cout << "\nSD:\n";
//				for(size_t i = 0; i < pgp->size(); ++i)
//					cout << i << ":" << testMotifSP(m, ms, pgp->at(i), sigPos[i]) << ", ";
//			}*/
//		} else {
//			cntPos = countMotif(mb.toMotif(), *pgp);
//		}
//		++stNumFreqPos;
//		double freqPos = static_cast<double>(cntPos) / pgp->size();
//		double scoreUB = freqPos;
//		// freqPos is the upperbound of differential & ratio based objective function
//		if(freqPos < minSup || scoreUB <= holder.lastScore())
//			continue;
//		if(layer >= smin) {
//			int cntNeg;
//			if(flagUseSD)
//				cntNeg = countMotifSP(mb, ms, *pgn, sigNeg);
//			else
//				cntNeg = countMotif(mb.toMotif(), *pgn);
//			++stNumFreqNeg;
//			double freqNeg = static_cast<double>(cntNeg) / pgn->size();
//			double score = objFun(freqPos, freqNeg);
//			holder.update(mb.toMotif(), score);
//		}
//		// generate new motifs
//		for(const Edge&e : edges) {
//			if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
//				MotifBuilder t(mb);
//				t.addEdge(e.s, e.d);
//				newLayer.push_back(move(t));
//				++stNumMotifGenerated;
//			}
//		}
//	}
//	return newLayer;
//}

std::map<MotifBuilder, int> StrategyFuncFreqSD::_edge1_bfs(const std::vector<MotifBuilder>& last,
	TopKHolder<Motif, double>& holder, const std::vector<Edge>& edges)
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
			cntPos = countMotifSP(mb, ms, *pgp, sigPos);
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
			cout << i << ":" << testMotifSP(m, ms, pgp->at(i), sigPos[i]) << ", ";
			}*/
		} else {
			cntPos = countMotif(mb.toMotif(), *pgp);
		}
		++stNumFreqPos;
		double freqPos = static_cast<double>(cntPos) / pgp->size();
		double scoreUB = freqPos;
		// freqPos is the upperbound of differential & ratio based objective function
		if(freqPos < minSup || scoreUB <= holder.lastScore())
			continue;
		if(layer >= smin) {
			int cntNeg;
			if(flagUseSD)
				cntNeg = countMotifSP(mb, ms, *pgn, sigNeg);
			else
				cntNeg = countMotif(mb.toMotif(), *pgn);
			++stNumFreqNeg;
			double freqNeg = static_cast<double>(cntNeg) / pgn->size();
			double score = objFun(freqPos, freqNeg);
			holder.update(mb.toMotif(), score);
		}
		// generate new motifs
		for(const Edge&e : edges) {
			if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
				MotifBuilder t(mb);
				t.addEdge(e.s, e.d);
				//newLayer.push_back(move(t));
				++newLayer[t];
				++stNumMotifGenerated;
			}
		}
	}
	return newLayer;
}
