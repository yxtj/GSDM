#include "stdafx.h"
#include "StrategyFuncFreq.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "Network.h"

using namespace std;

const std::string StrategyFuncFreq::name("funcfreq");
const std::string StrategyFuncFreq::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFuncFreq::name + " <k> <smin> <smax> <minSup> <minSnap> <obj-fun> <alpha>\n"
	"  <k>: return top-k result"
	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <minSnap>: [double] the minimum show up probability of a motif among a subject's all snapshots\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
);

bool StrategyFuncFreq::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 7, name);
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
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyFuncFreq::search(const Option & opt, 
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
	numMotifExplored = 0;

	Network net;
	vector<Motif> res;
	if(net.getSize() == 1) {
		res = method_enum1();
		//res = master(net);
	} else {
		if(net.getRank() == 0) {
			res = master(net);
		} else {
			numMotifExplored = slave(net);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "  Rank " << net.getRank() << " has counted " << numMotifExplored << " motifs." << endl;
	return res;
}

bool StrategyFuncFreq::setObjFun(const std::string & name)
{
	if(name == "diff") {
		//objFun = &StrategyFuncFreq::objFun_diffP2N;
		objFun = bind(&StrategyFuncFreq::objFun_diffP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 1;
		return true;
	} else if(name == "margin") {
		objFun = bind(&StrategyFuncFreq::objFun_marginP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 2;
		return true;
	} else if(name == "ratio") {
		//objFun = &StrategyFuncFreq::objFun_ratioP2N;
		objFun = bind(&StrategyFuncFreq::objFun_ratioP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 3;
		return true;
	}
	return false;
}

std::vector<Edge> StrategyFuncFreq::getEdges()
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

std::vector<Motif> StrategyFuncFreq::method_enum1()
{
	cout << "Phase 1 (meta-data prepare)" << endl;
	slist supPos, supNeg;
	for(auto& s : *pgp)
		supPos.pushFront(&s);
	for(auto& s : *pgn)
		supNeg.pushFront(&s);
	vector<Edge> edges = getEdges();
	cout << "  # of edges: " << edges.size() << endl;

	cout << "Phase 2 (calculate)" << endl;
	Motif dummy;
	TopKHolder<Motif, double> holder(k);
	chrono::system_clock::time_point _time = chrono::system_clock::now();
	_enum1(0, dummy, supPos, supNeg, holder, edges);
	auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now() - _time).count();
	cout << "  # of result: " << holder.size() << ", last score: " << holder.lastScore()
		<< "\n  time: " << _time_ms << " ms" << endl;

	cout << "Phase 3 (output)" << endl;
	{
		ofstream fout("score.txt");
		for(auto& p : holder.data) {
			fout << p.second << "\t" << p.first << "\n";
		}
	}
	vector<Motif> res = holder.getResultMove();
	return res;
}

double StrategyFuncFreq::objFun_diffP2N(const double freqPos, const double freqNeg)
{
	return freqPos - alpha*freqNeg;
}

double StrategyFuncFreq::objFun_marginP2N(const double freqPos, const double freqNeg)
{
	return (1.0 - freqPos) + alpha*freqNeg;
}

double StrategyFuncFreq::objFun_ratioP2N(const double freqPos, const double freqNeg)
{
	//return freqNeg != 0.0 ? freqPos / freqNeg : freqPos;
	return freqPos*freqPos / (freqPos + freqNeg);
}

bool StrategyFuncFreq::checkEdge(const int s, const int d, const std::vector<Graph>& sub) const
{
	int th = static_cast<int>(ceil(sub.size()*pSnap));
	int cnt = 0;
	for(auto&g : sub) {
		if(g.testEdge(s, d)) {
			if(++cnt >= th)
				break;
		}
	}
	return cnt >= th;
}

bool StrategyFuncFreq::checkEdge(const int s, const int d) const
{
	int cnt = 0;
	for(auto&sub : *pgp) {
		if(checkEdge(s, d, sub))
			if(++cnt >= nMinSup)
				break;
	}
	if(cnt >= nMinSup)
		return true;
	for(auto&sub : *pgn) {
		if(checkEdge(s, d, sub))
			if(++cnt >= nMinSup)
				break;
	}
	return cnt >= nMinSup;
}

int StrategyFuncFreq::countEdge(const int s, const int d, const std::vector<std::vector<Graph>>& subs) const
{
	int cnt = 0;
	for(auto&sub : subs) {
		if(checkEdge(s, d, sub))
			++cnt;
	}
	return cnt;
}

std::pair<int, int> StrategyFuncFreq::countEdge(const int s, const int d) const
{
	int nPos = countEdge(s, d, *pgp);
	int nNeg = countEdge(s, d, *pgn);
	return make_pair(nPos, nNeg);
}

bool StrategyFuncFreq::testMotif(const Motif & m, const std::vector<Graph>& sub) const
{
	int th = static_cast<int>(ceil(sub.size()*pSnap));
	int cnt = 0;
	for(auto&g : sub) {
		if(g.testMotif(m)) {
			if(++cnt >= th)
				break;
		}
	}
	return cnt >= th;
}

int StrategyFuncFreq::countMotif(const Motif & m, const std::vector<std::vector<Graph>>& subs) const
{
	int res = 0;
	for(auto&sub : subs) {
		if(testMotif(m, sub))
			++res;
	}
	return res;
}

std::pair<int, int> StrategyFuncFreq::countMotif(const Motif & m) const
{
	int nPos = countMotif(m, *pgp);
	int nNeg = countMotif(m, *pgn);
//	cout << m << "\t(" << nPos << "," << nNeg << ")" << endl;
//	this_thread::sleep_for(chrono::seconds(1));
	return make_pair(nPos, nNeg);
}


void StrategyFuncFreq::removeSupport(slist& sup, std::vector<const subject_t*>& rmv, const Edge& e)
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

void StrategyFuncFreq::_enum1(const unsigned p, Motif & curr, slist& supPos, slist& supNeg,
	TopKHolder<Motif, double>& res, const std::vector<Edge>& edges)
{
	if(p >= edges.size() || curr.size() == smax) {
		if(curr.getnEdge() >= smin && supPos.size() + supNeg.size() >= nMinSup && curr.connected()) {
		//if(curr.getnEdge() >= smin && supPos.size() >= nMinSup && curr.connected()) {
			double s = objFun(static_cast<double>(supPos.size()) / pgp->size(),
				static_cast<double>(supNeg.size()) / pgn->size());
			++numMotifExplored;
			//res.update(move(curr), s);
			res.update(curr, s);
		}
		return;
	}
	if(objFunID == 1) {
		double upBound = objFun(static_cast<double>(supPos.size()) / pgp->size(), 0.0);
		if(!res.updatable(upBound)) {
			return;
		}
	}

	_enum1(p + 1, curr, supPos, supNeg, res, edges);

	vector<const subject_t*> rmvPos, rmvNeg;
	removeSupport(supPos, rmvPos, edges[p]);
	removeSupport(supNeg, rmvNeg, edges[p]);
	curr.addEdge(edges[p].s, edges[p].d);
	_enum1(p + 1, curr, supPos, supNeg, res, edges);
	curr.removeEdge(edges[p].s, edges[p].d);
	for(auto s : rmvPos)
		supPos.pushFront(s);
	for(auto s : rmvNeg)
		supNeg.pushFront(s);
}
