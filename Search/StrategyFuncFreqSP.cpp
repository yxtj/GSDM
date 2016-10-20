#include "stdafx.h"
#include "StrategyFuncFreqSP.h"
#include "CandidateMethodFactory.h"
#include "Option.h"
#include "Network.h"
#include "../util/Timer.h"

using namespace std;

const std::string StrategyFuncFreqSP::name("funcfreqsp");
const std::string StrategyFuncFreqSP::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFuncFreqSP::name + " <k> <smin> <smax> <minSup> <minSnap> <obj-fun> <alpha> [usd-sd] [log]\n"
	"  <k>: [integer] return top-k result"
	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <minSnap>: [double] the minimum show up probability of a motif among a subject's all snapshots\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  [use-sd]: [0/1] optional, default 1, set to 0 to disable the optimization of shortest distance\n"
	"  [log]: [0/1] optional, default 0, set to 1 to output the score of the top-k result"
);

bool StrategyFuncFreqSP::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 7, 9, name);
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
		try {
			if(param.size() > 8)
				flagUseSD = stod(param[8]) == 1;
		} catch(...) {}
		flagOutputScore = false;
		try {
			if(param.size() > 9)
				flagOutputScore = stod(param[9]) == 1;
		} catch(...) {}
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

	if(flagUseSD) {
		cout << "Generating subject signatures..." << endl;
		chrono::system_clock::time_point _time = chrono::system_clock::now();
		setSignature();
		auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
			chrono::system_clock::now() - _time).count();
		cout << "  Signatures generated in " << _time_ms << " ms" << endl;
	}

	cout << "Enumeratig..." << endl;
	Network net;
	vector<Motif> res;
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
	sigPos.reserve(pgp->size());
	for(auto& sub : *pgp) {
		sigPos.push_back(genSignture(sub, pSnap));
	}
	sigNeg.reserve(pgn->size());
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

bool StrategyFuncFreqSP::testMotifSP(const MotifBuilder & m, const MotifSign& ms,
	const std::vector<Graph>& sub, const Signature& ss) const
{
	if(!checkSPNecessary(m,ms,ss))
		return false;
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

int StrategyFuncFreqSP::countMotifSP(const MotifBuilder & m, const MotifSign & ms,
	const std::vector<std::vector<Graph>>& subs, const std::vector<Signature>& sigs) const
{
	int res = 0;
	for(size_t i = 0; i < subs.size(); ++i) {
		if(testMotifSP(m, ms, subs[i], sigs[i]))
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
	Timer timer;
	_enum1(0, dummy, supPos, supNeg, holder, edges);
	auto _time_ms = timer.elapseMS();
	cout << "  # of result: " << holder.size() << ", last score: " << holder.lastScore()
		<< "\n  time: " << _time_ms << " ms" << endl;

	cout << "Phase 3 (output)" << endl;
	if(flagOutputScore) {
		ofstream fout("../logs/score.txt");
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

	cout << "Phase 2 (testing motifs layer by layer)" << endl;
	TopKHolder<Motif, double> holder(k);
	for(int s = 2; s <= smax; ++s) {
		Timer timer;
		vector<MotifBuilder> t = _edge1_bfs(last, holder, edges);
		sort(t.begin(), t.end());
		auto itend = unique(t.begin(), t.end());
		auto _time_ms = timer.elapseMS();
		cout << "  motifs of size " << s - 1 << " : " << _time_ms << " ms, on "<<last.size()<<" motifs."
			<< "\tgenerate new "<<itend - t.begin() << " / " << t.size() << " motifs (unique/total)" << endl;
		t.erase(itend, t.end());
		if(t.empty())
			break;
		last = move(t);
	}

	cout << "Phase 3 (output)" << endl;
	if(flagOutputScore) {
		ofstream fout("../logs/score.txt");
		for(auto& p : holder.data) {
			fout << p.second << "\t" << p.first << "\n";
		}
	}
	return holder.getResultMove();
}

std::vector<MotifBuilder> StrategyFuncFreqSP::_edge1_bfs(const std::vector<MotifBuilder>& last,
	TopKHolder<Motif, double>& holder, const std::vector<Edge>& edges)
{
	int layer = last.front().getnEdge();
	std::vector<MotifBuilder> newLayer;
	for(const auto& mb : last) {
		// work on a motif
		MotifSign ms(nNode);
		++numMotifExplored;
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
		} else
			cntPos = countMotif(mb.toMotif(), *pgp);
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
			double freqNeg = static_cast<double>(cntNeg) / pgn->size();
			double score = objFun(freqPos, freqNeg);
			holder.update(mb.toMotif(), score);
		}
		// generate new motifs
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



StrategyFuncFreqSP::Signature StrategyFuncFreqSP::genSignture(
	const std::vector<Graph>& gs, const double theta)
{
	int th = static_cast<int>(ceil(theta*gs.size()));
	int n = nNode;
	//vector<sdmatrix_t> buf;
	vector<vector<vector<int>>> buf(n, vector<vector<int>>(n,vector<int>(gs.size())));
	for(size_t k = 0; k < gs.size(); ++k) {
		std::vector<std::vector<int>> m = calA2AShortestDistance(gs[k]);
		//buf.push_back(move(m));
		for(int i = 0; i < n; ++i)
			for(int j = 0; j < n; ++j)
				buf[i][j][k] = m[i][j];
	}
	Signature res(n);
	for(int i = 0; i < n; ++i)
		for(int j = 0; j < n; ++j) {
			nth_element(buf[i][j].begin(), buf[i][j].begin() + th, buf[i][j].end());
			res.sd[i][j] = buf[i][j][th-1];
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
//		sd[i][i] = 0;
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

void StrategyFuncFreqSP::updateMotifSD(MotifSign & ms, const MotifBuilder & mOld, int s, int d)
{
	vector<int> nodes;
	int n = mOld.getnNode();
	nodes.reserve(n);
	for(int i = 0; i < nNode; ++i) {
		if(mOld.containNode(i))
			nodes.push_back(i);
	}
	ms.sd[s][d] = ms.sd[d][s] = 1;
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

void StrategyFuncFreqSP::calMotifSD(MotifSign & ms, const MotifBuilder & m)
{
	//init direct edges
	for(auto& e : m.edges) {
		ms.sd[e.s][e.d] = ms.sd[e.d][e.s] = 1;
	}
	//prepare nodes
	vector<int> nodes;
	int n = m.getnNode();
	nodes.reserve(n);
	for(int i = 0; i < nNode; ++i) {
		if(m.containNode(i))
			nodes.push_back(i);
	}
	//update
	for(int k = 0; k < n; ++k) {
		for(int i : nodes) {
			for(int j : nodes) {
				if(ms.sd[i][k] + ms.sd[k][j] < ms.sd[i][j])
					ms.sd[i][j] = ms.sd[i][k] + ms.sd[k][j];
			}
		}
	}
}

bool StrategyFuncFreqSP::checkSPNecessary(const MotifBuilder& m, const MotifSign & ms, const Signature & ss) const
{
	// all edges should: sdis(e;g) <= sdis(e;m)
	for(const Edge& e : m.edges) {
		if(ms.sd[e.s][e.d] < ss[e.s][e.d])
			return false;
	}
	return true;
}
