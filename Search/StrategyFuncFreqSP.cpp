#include "stdafx.h"
#include "StrategyFuncFreqSP.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "Network.h"

using namespace std;

const std::string StrategyFuncFreqSP::name("funcfreqsp");
const std::string StrategyFuncFreqSP::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFuncFreqSP::name + " <k> <smin> <smax> <minSup> <minSnap> <obj-fun> <alpha>\n"
	"  <k>: return top-k result"
	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <minSnap>: [double] the minimum show up probability of a motif among a subject's all snapshots\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
);

bool StrategyFuncFreqSP::parse(const std::vector<std::string>& param)
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

std::vector<Motif> StrategyFuncFreqSP::search(const Option & opt, 
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

	setSignature();

	Network net;
	vector<Motif> res;
	if(net.getSize() == 1) {
		res = method_enum1();
		//res = master(net);
	} else {
		//if(net.getRank() == 0) {
		//	res = master(net);
		//} else {
		//	numMotifExplored = slave(net);
		//}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "  Rank " << net.getRank() << " has counted " << numMotifExplored << " motifs." << endl;
	return res;
}

bool StrategyFuncFreqSP::setObjFun(const std::string & name)
{
	if(name == "diff") {
		//objFun = &StrategyFuncFreqSP::objFun_diffP2N;
		objFun = bind(&StrategyFuncFreqSP::objFun_diffP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 1;
		return true;
	} else if(name == "margin") {
		objFun = bind(&StrategyFuncFreqSP::objFun_marginP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 2;
		return true;
	} else if(name == "ratio") {
		//objFun = &StrategyFuncFreqSP::objFun_ratioP2N;
		objFun = bind(&StrategyFuncFreqSP::objFun_ratioP2N, this, placeholders::_1, placeholders::_2);
		objFunID = 3;
		return true;
	}
	return false;
}

std::vector<Edge> StrategyFuncFreqSP::getEdges()
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

void StrategyFuncFreqSP::setSignature()
{
	for(auto& sub : *pgp) {
		sigPos.push_back(genSignture(sub, pSnap));
	}
	for(auto& sub : *pgn) {
		sigNeg.push_back(genSignture(sub, pSnap));
	}
}

double StrategyFuncFreqSP::objFun_diffP2N(const double freqPos, const double freqNeg)
{
	return freqPos - alpha*freqNeg;
}

double StrategyFuncFreqSP::objFun_marginP2N(const double freqPos, const double freqNeg)
{
	return (1.0 - freqPos) + alpha*freqNeg;
}

double StrategyFuncFreqSP::objFun_ratioP2N(const double freqPos, const double freqNeg)
{
	//return freqNeg != 0.0 ? freqPos / freqNeg : freqPos;
	return freqPos*freqPos / (freqPos + freqNeg);
}

bool StrategyFuncFreqSP::checkEdge(const int s, const int d, const std::vector<Graph>& sub) const
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

bool StrategyFuncFreqSP::checkEdge(const int s, const int d) const
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

int StrategyFuncFreqSP::countEdge(const int s, const int d, const std::vector<std::vector<Graph>>& subs) const
{
	int cnt = 0;
	for(auto&sub : subs) {
		if(checkEdge(s, d, sub))
			++cnt;
	}
	return cnt;
}

std::pair<int, int> StrategyFuncFreqSP::countEdge(const int s, const int d) const
{
	int nPos = countEdge(s, d, *pgp);
	int nNeg = countEdge(s, d, *pgn);
	return make_pair(nPos, nNeg);
}

bool StrategyFuncFreqSP::testMotif(const Motif & m, const std::vector<Graph>& sub) const
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

int StrategyFuncFreqSP::countMotif(const Motif & m, const std::vector<std::vector<Graph>>& subs) const
{
	int res = 0;
	for(auto&sub : subs) {
		if(testMotif(m, sub))
			++res;
	}
	return res;
}

std::pair<int, int> StrategyFuncFreqSP::countMotif(const Motif & m) const
{
	int nPos = countMotif(m, *pgp);
	int nNeg = countMotif(m, *pgn);
//	cout << m << "\t(" << nPos << "," << nNeg << ")" << endl;
//	this_thread::sleep_for(chrono::seconds(1));
	return make_pair(nPos, nNeg);
}


void StrategyFuncFreqSP::removeSupport(slist& sup, std::vector<const subject_t*>& rmv, const Edge& e)
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

std::vector<Motif> StrategyFuncFreqSP::method_enum1()
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

void StrategyFuncFreqSP::_enum1(const unsigned p, Motif & curr, slist& supPos, slist& supNeg,
	TopKHolder<Motif, double>& res, const std::vector<Edge>& edges)
{
	if(p >= edges.size() || curr.size() == smax) {
		if(curr.getnEdge() >= smin && supPos.size() + supNeg.size() >= nMinSup && curr.connected()) {
			//if(curr.getnEdge() >= smin && supPos.size() >= nMinSup && curr.connected()) {
			double s = objFun(static_cast<double>(supPos.size()) / pgp->size(),
				static_cast<double>(supNeg.size()) / pgn->size());
			++numMotifExplored;
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

std::vector<Motif> StrategyFuncFreqSP::method_edge1_bfs()
{
	vector<Motif> res;
	vector<Edge> edges = getEdges();
//	vector<pair<MotifBuilder, double>> last;
	vector<MotifBuilder> last;
	res.reserve(3 * edges.size());
	last.reserve(3 * edges.size());
	for(const Edge& e : edges) {
		MotifBuilder m;
		m.addEdge(e.s, e.d);
//		double p = gp.matrix[e.s][e.d];
//		last.emplace_back(move(m), p);
		last.push_back(move(m));
	}
	TopKHolder<Motif, double> holder(k);

	for(int s = 2; s <= smax; ++s) {
		vector<MotifBuilder> t = _edge1_bfs(last, holder, edges);
		sort(t.begin(), t.end());
		auto itend = unique(t.begin(), t.end());
		cout << t.end() - itend << " / " << t.size() << " redundant motifs of size " << s << endl;
		t.erase(itend, t.end());
		if(t.empty())
			break;
		if(s - 1 >= smin) {
			for(auto& mbp : last)
				res.push_back(mbp.toMotif());
		}
		last = move(t);
	}
	for(auto& mbp : last)
		res.push_back(mbp.toMotif());
	return res;
}

std::vector<MotifBuilder> StrategyFuncFreqSP::_edge1_bfs(const std::vector<MotifBuilder>& last,
	TopKHolder<Motif, double>& res, const std::vector<Edge>& edges)
{
	std::vector<MotifBuilder> newLayer;
	for(const auto& mb : last) {
		//		workOnMotif(m.toMotif());
		Motif m = mb.toMotif();
		double freqPos = static_cast<double>(countMotif(m, *pgp)) / pgp->size();


		for(const Edge&e : edges) {
			if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
				MotifBuilder t(mb);
				t.addEdge(e.s, e.d);
				newLayer.push_back(move(t));
			}
		}
	}
	return newLayer;
}



StrategyFuncFreqSP::signature StrategyFuncFreqSP::genSignture(
	const std::vector<Graph>& gs, const double theta)
{
	int th = static_cast<int>(ceil(theta*gs.size()));
	int n = nNode;
	typedef std::vector<std::vector<int>> sdmatrix_t;
	vector<sdmatrix_t> buf(th);
	for(size_t k = 0; k < gs.size(); ++k) {
		sdmatrix_t m = calA2AShortestDistance(gs[k]);
		for(int i = 0; i < n; ++i)
			for(int j = 0; j < n; ++j)
				buf[i][j][k] = m[i][j];
	}
	signature res(n);
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j) {
			nth_element(buf[i][j].begin(), buf[i][j].begin() + th, buf[i][j].end());
			res.sd[i][j] = buf[i][j][th];
		}
	return res;
}

std::vector<std::vector<int>> StrategyFuncFreqSP::calA2AShortestDistance(const Graph & g)
{
	int n = g.getnNode();
	std::vector<std::vector<int>> sd(n, vector<int>(n, 2 * n));
	for(int i = 0; i < n; ++i) {
		for(int j = 0; j < n; ++j) {
			if(g.matrix[i][j])
				sd[i][j] = 1;
		}
		sd[i][i] = 0;
	}
	for(int k = 0; k < n; ++k) {
		for(int i = 0; i < n; ++i)
			for(int j = 0; j < n; ++j) {
				if(sd[i][k] + sd[k][j] < sd[i][j])
					sd[i][j] = sd[i][k] + sd[k][j];
			}
	}
	return sd;
}

void StrategyFuncFreqSP::updateSP(motifSign & ms, const MotifBuilder & mOld, int s, int d)
{
	ms.sd[s][d] = ms.sd[d][s] = 1;
	vector<int> nodes;
	int n = mOld.getnNode();
	nodes.reserve(n);
	for(int i = 0; i < nNode; ++i) {
		if(mOld.containNode(i))
			nodes.push_back(i);
	}
	// TODO: optimize with SPFA-like method
	for(int k = 0; k < n; ++k) {
		for(int i : nodes) {
			for(int j : nodes) {
				if(ms.sd[i][k] + ms.sd[k][j] < ms.sd[i][j])
					ms.sd[i][j] = ms.sd[i][k] + ms.sd[k][j];
			}
		}
	}
}

bool StrategyFuncFreqSP::checkSPNecessary(const MotifBuilder& m, const motifSign & ms, const signature & ss)
{
	// all edges should: sdis(e;g) <= sdis(e;m)
	for(const Edge& e : m.edges) {
		if(ms.sd[e.s][e.d] < ss[e.s][e.d])
			return false;
	}
	return true;
}
