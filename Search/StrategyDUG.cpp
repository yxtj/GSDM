#include "stdafx.h"
#include "StrategyDUG.h"

using namespace std;

const std::string StrategyDUG::name("DUG");
const std::string StrategyDUG::usage("Discriminative Uncertain Graph\n"
	"Usage: " + StrategyDUG::name + " <top k> <DS method> <SS method>\n"
	"  <DS method>: Discriminative score method (conf, freq, gtest, hsic).\n"
	"  <SS method>: Statistical summary method (exp, median, mode, phi<X>), where <X> of phi<X> can be any number.");


bool StrategyDUG::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 6, name);
		k = stoi(param[1]);
		dmethod = param[2];
		if(!parseDMethod())
			throw invalid_argument("Strategy "+ StrategyDUG::name 
				+ " cannot parse <DS method>: " + dmethod);
		smethod = param[3];
		if(!parseSMethod())
			throw invalid_argument("Strategy " + StrategyDUG::name
				+ "cannot parse <SS method>: " + smethod);
		minSup = stod(param[4]);
		smin = stoi(param[5]);
		smax = stoi(param[6]);
		if(smax <= 0)
			smax = numeric_limits<int>::max();
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

bool StrategyDUG::parseDMethod()
{
	if(dmethod == "conf") {
		disScoreFun = &StrategyDUG::dsfConfindence;
	}else if(dmethod == "freq") {
		disScoreFun = &StrategyDUG::dsfFreqRatio;
	} else if(dmethod == "gtest") {
		disScoreFun = &StrategyDUG::dsfGtest;
	} else if(dmethod == "hsic") {
		disScoreFun = &StrategyDUG::dsfHSIC;
	} else {
		return false;
	}
	return true;
}

bool StrategyDUG::parseSMethod()
{
	//exp, median, mode, phi<X>
	if(smethod == "exp") {
		statSumFun = bind(&StrategyDUG::ssfExp, this, placeholders::_1, placeholders::_2);
	} else if(smethod == "median") {
		statSumFun = bind(&StrategyDUG::ssfMedian, this, placeholders::_1, placeholders::_2);
	} else if(smethod == "mode") {
		statSumFun = bind(&StrategyDUG::ssfMode, this, placeholders::_1, placeholders::_2);
	} else if(smethod.find("phi") != string::npos && smethod.size() > 3) {
		phi = stod(smethod.substr(3));
		statSumFun = bind(&StrategyDUG::ssfPhiProb, this, placeholders::_1, placeholders::_2);
	} else {
		return false;
	}
	return true;
}


std::vector<Motif> StrategyDUG::search(const Option & opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();

	nNode = gPos.front().front().nNode;
	vector<GraphProb> ugp = getUGfromCGs(gPos);
	vector<GraphProb> ugn = getUGfromCGs(gNeg);
	GraphProb ugall(nNode);
	pugp = &ugp;
	pugn = &ugn;
	pugall = &ugall;
	minSupN = static_cast<int>((gPos.size() + gNeg.size())*minSup);
	// uncertain data init
	ugall.startAccum(nNode);
	for(auto& ug : ugp) {
		ugall.merge(ug);
	}
	for(auto& ug : gNeg) {
		ugall.merge(ug);
	}
	ugall.finishAccum();
	
	// start searching:
	


	vector<Motif> res;
	return res;
}

std::vector<GraphProb> StrategyDUG::getUGfromCGs(const std::vector<std::vector<Graph>>& gs)
{
	std::vector<GraphProb> res;
	res.reserve(gs.size());
	for(auto& vec : gs) {
		res.emplace_back(vec);
	}
	return res;
}

double StrategyDUG::probMotifOnUG(const MotifBuilder & m, const GraphProb & ug)
{
	double res = 1.0;
	for(auto it = m.edges.begin(); res != 0.0 && it != m.edges.end(); ++it) {
		res *= ug.matrix[it->s][it->d];
	}
	return res;
}

std::vector<double> StrategyDUG::disMotifOnUDataset(const Motif & m, std::vector<GraphProb>& ugs)
{
	size_t n = ugs.size();
	// initialize:
	//	Pr(found 0 occurence among 0 graph) = 1.0;
	//	Pr(found x>0 cuucrences among 0 graph) = 0.0
	vector<double> res(n + 1, 0.0);
	res[0] = 1.0;
	// i -> number of the used graphs
	for(size_t i = 1; i <= n; ++i) {
		double p = probMotifOnUG(m, ugs[i - 1]);
		// j -> number of graphs which contains m
		for(size_t j = i; j >= 1; --j) {
			//res[i][j] = (1 - p)*res[i - 1][j] + p*res[i - 1][j - 1];
			res[j] = (1 - p)*res[j] + p*res[j - 1];
		}
		res[0] = (1 - p)*res[0];
	}
	return res;
}

double StrategyDUG::dsfConfindence(int cMotifPos, int cMotifNeg, int cPos, int cNeg)
{
	return static_cast<double>(cMotifPos) / (cMotifPos + cMotifNeg);
}

double StrategyDUG::dsfFreqRatio(int cMotifPos, int cMotifNeg, int cPos, int cNeg)
{
	return abs(log2(static_cast<double>(cMotifPos*cNeg) / (cMotifNeg*cPos)));
}

double StrategyDUG::dsfGtest(int cMotifPos, int cMotifNeg, int cPos, int cNeg)
{
	int difPos = cPos - cMotifPos;
	int difNeg = cNeg - cMotifNeg;
	return 2 * cMotifPos*log(static_cast<double>(cMotifPos*cNeg) / (cMotifNeg*cPos))
		+ 2 * difPos*log(static_cast<double>(cNeg*difPos) / (cPos*difNeg));
}

double StrategyDUG::dsfHSIC(int cMotifPos, int cMotifNeg, int cPos, int cNeg)
{
	double up = cMotifPos*cNeg - cMotifNeg*cPos;
	double down = (cPos + cNeg - 1)*(cPos + cNeg);
	return up*up / (down*down);
}

double StrategyDUG::ssfExp(const std::vector<double>& disPos, const std::vector<double>& disNeg)
{
	double res = 0.0;
	int nPos = disPos.size();
	int nNeg = disNeg.size();
	for(int i = 0; i < nPos; ++i) {
		for(int j = 0; j < nNeg; ++j) {
			res += disPos[i] * disNeg[j] * disScoreFun(i, j, nPos, nNeg);
		}
	}
	return res / nPos / nNeg;
}

double StrategyDUG::ssfMedian(const std::vector<double>& disPos, const std::vector<double>& disNeg)
{
	vector<double> temp;
	temp.reserve(disPos.size()*disNeg.size());
	int nPos = disPos.size();
	int nNeg = disNeg.size();
	for(int i = 0; i < nPos; ++i) {
		for(int j = 0; j < nNeg; ++j) {
			temp.push_back(disScoreFun(i, j, nPos, nNeg));
		}
	}
	auto itMid = temp.begin() + temp.size() / 2;
	partial_sort(temp.begin(), itMid, temp.end());
	if(temp.size() <= 2)
		return temp.front();
	return *(--itMid);
}

double StrategyDUG::ssfMode(const std::vector<double>& disPos, const std::vector<double>& disNeg)
{
	vector<double> temp;
	temp.reserve(disPos.size()*disNeg.size());
	int nPos = disPos.size();
	int nNeg = disNeg.size();
	for(int i = 0; i < nPos; ++i) {
		for(int j = 0; j < nNeg; ++j) {
			temp.push_back(disScoreFun(i, j, nPos, nNeg));
		}
	}
	sort(temp.begin(), temp.end());
	double last = nan("");
	double res;
	int cnt = 0, resCnt = -1;
	for(double v : temp) {
		if(v == last) {
			++cnt;
		} else {
			if(cnt > resCnt) {
				res = last;
				resCnt = cnt;
			}
			last = v;
			cnt = 1;
		}
	}
	if(cnt > resCnt) {
		res = last;
	}
	return res;
}

double StrategyDUG::ssfPhiProb(const std::vector<double>& disPos, const std::vector<double>& disNeg)
{
	double res = 0.0;
	int nPos = disPos.size();
	int nNeg = disNeg.size();
	for(int i = 0; i < nPos; ++i) {
		for(int j = 0; j < nNeg; ++j) {
			if(disScoreFun(i, j, nPos, nNeg) >= phi)
				res += disPos[i] * disNeg[j];
		}
	}
	return res / nPos / nNeg;
}

std::vector<Edge> StrategyDUG::getEdges(const GraphProb & gp)
{
	vector<Edge> edges;
	for(int i = 0; i < nNode; ++i) {
		for(int j = i + 1; j < nNode; ++j)
			if(gp.matrix[i][j] >= minSup)
				edges.emplace_back(i, j);
	}
	return edges;
}

std::vector<Motif> StrategyDUG::method_edge2_dp(const GraphProb& ugall,
	const std::vector<GraphProb>& ugp, const std::vector<GraphProb>& ugn)
{
	vector<Motif> mps;
	vector<pair<MotifBuilder, double>> open;
	open.emplace_back(MotifBuilder(), 1.0);

	vector<Edge> edges = getEdges(ugall);
	// maintain: mps: maxized result (s==smax)
	//			open: expandable result (0<=s<=smax-1)
	for(const Edge& e : edges) {
		vector<pair<MotifBuilder, double>> t = _edge2_dp(open, e);
		for(auto& mp : t) {
			int n = mp.first.getnEdge();
			if(n == smax) {
				mps.push_back(mp.first.toMotif());
			} else {
				open.push_back(move(mp));
			}
		}
	}

	for(auto& mp : open) {
		if(mp.first.getnEdge() >= smin)
			mps.push_back(mp.first.toMotif());
	}
	return mps;
}

vector<pair<MotifBuilder, double>> StrategyDUG::_edge2_dp(
	const std::vector<std::pair<MotifBuilder, double>>& last, const Edge & e)
{
	vector<std::pair<MotifBuilder, double>> res;
	for(const auto& mp : last) {
		if(mp.first.getnEdge() >= smax)
			continue;
		if(mp.first.empty() || mp.first.containNode(e.s) || mp.first.containNode(e.d)) {
			MotifBuilder t(mp.first);
			t.addEdge(e.s, e.d);
			//double p = probMotifOnUG(t, *pugall);
			double p = mp.second*pugall->matrix[e.s][e.d];
			if(p >= minSup) {
				res.push_back(make_pair(move(t), p));
			}
		}
	}
	return res;
}


