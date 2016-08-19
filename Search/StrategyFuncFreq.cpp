#include "stdafx.h"
#include "StrategyFuncFreq.h"
#include "CandidateMethodFactory.h"
#include "Option.h"

using namespace std;

const std::string StrategyFuncFreq::name("funcfreq");
const std::string StrategyFuncFreq::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFuncFreq::name + " <k> <alpha> <minSup> <minSnap> <smin> <smax>\n"
	"  <k>: return top-k result"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <minSnap>: [double] the minimum show up probability of a motif among a subject's all snapshots");

bool StrategyFuncFreq::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 6, name);
		k = stoi(param[1]);
		alpha = stod(param[2]);
		minSup = stod(param[3]);
		pSnap = stod(param[4]);
		smin = stoi(param[5]);
		smax = stoi(param[6]);
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
	pgp = &gPos;
	pgn = &gNeg;
	nMinSup = static_cast<unsigned>((gPos.size() + gNeg.size())*minSup);
	nNode = gPos[0][0].nNode;

	return method_enum1();
}

double StrategyFuncFreq::objectFunction(const double freqPos, const double freqNeg)
{
	return freqPos - alpha*freqNeg;
}

bool StrategyFuncFreq::checkEdge(const int s, const int d) const
{
	unsigned cnt = 0;
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
	slist supPos, supNeg;
	for(auto& s : *pgp)
		supPos.push(&s);
	for(auto& s : *pgn)
		supNeg.push(&s);
	vector<Edge> edges = getEdges();

	Motif dummy;
	TopKHolder holder(k);
	_enum1(0, dummy, supPos, supNeg, holder, edges);
	vector<Motif> res;
	for(auto& mp : holder.data)
		res.push_back(move(mp.first));
	return res;
}

void StrategyFuncFreq::_enum1(const unsigned p, Motif & curr, slist& supPos, slist& supNeg,
	TopKHolder& res, const std::vector<Edge>& edges)
{
	if(p >= edges.size() || curr.size() == smax) {
		if(curr.size() >= smin && supPos.size() + supNeg.size() >= nMinSup && curr.connected()) {
			double s = objectFunction(static_cast<double>(supPos.size()) / pgp->size(),
				static_cast<double>(supNeg.size()) / pgn->size());
			res.update(move(curr), s);
		}
		return;
	}
	double lowBound = objectFunction(static_cast<double>(supPos.size()) / pgp->size(), 0.0);
	if(!res.updatable(lowBound)) {
		return;
	}

	vector<const subject_t*> rmvPos, rmvNeg;
	removeSupport(supPos, rmvPos, edges[p]);
	removeSupport(supNeg, rmvNeg, edges[p]);
	curr.addEdge(edges[p].s, edges[p].d);
	_enum1(p + 1, curr, supPos, supNeg, res, edges);
	curr.removeEdge(edges[p].s, edges[p].d);
	for(auto s : rmvPos)	supPos.push(s);
	for(auto s : rmvNeg)	supNeg.push(s);

	_enum1(p + 1, curr, supPos, supNeg, res, edges);
}

std::vector<Motif> StrategyFuncFreq::_enum1_nofun(const unsigned p, Motif & curr,
	slist& supPos, slist& supNeg, const std::vector<Edge>& edges)
{
	if(p >= edges.size()) {
		if(curr.size() >= 2 && curr.connected() && supPos.size() + supNeg.size() >= nMinSup) {
			return{ curr };
		}
	}
	vector<Motif> res = _enum1_nofun(p + 1, curr, supPos, supNeg, edges);

	curr.addEdge(edges[p].s, edges[p].d);
	vector<const subject_t*> rmvPos, rmvNeg;
	removeSupport(supPos, rmvPos, edges[p]);
	removeSupport(supNeg, rmvNeg, edges[p]);
	auto t = _enum1_nofun(p + 1, curr, supPos, supNeg, edges);
	curr.removeEdge(edges[p].s, edges[p].d);
	for(auto s : rmvPos)
		supPos.push(s);
	for(auto s : rmvNeg)
		supNeg.push(s);
	move(t.begin(), t.end(), back_inserter(res));
	return res;
}
