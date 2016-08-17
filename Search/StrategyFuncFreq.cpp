#include "stdafx.h"
#include "StrategyFuncFreq.h"
#include "CandidateMethodFactory.h"
#include "Option.h"

using namespace std;

const std::string StrategyFuncFreq::name("funcfreq");
const std::string StrategyFuncFreq::usage(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyFuncFreq::name + " <k> <alpha> <minSup>\n"
	"  <k>: return top-k result"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects");

bool StrategyFuncFreq::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 3, name);
		k = stoi(param[1]);
		alpha = stod(param[2]);
		minSup = stod(param[3]);
//		smin = stoi(param[3]);
//		smax = stoi(param[4]);
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
	nMinSup = static_cast<int>(gPos.size()*minSup);
	nNode = gPos[0][0].nNode;

	return std::vector<Motif>();
}

double StrategyFuncFreq::objectFunction(const double freqPos, const double freqNeg)
{
	return freqPos - alpha*freqNeg;
}

std::vector<Motif> StrategyFuncFreq::method_edge2_dp()
{
	int smin = 2;
	vector<Motif> mps;
	vector<MotifBuilder> open;
	open.emplace_back(MotifBuilder());
/*
	vector<Edge> edges = getEdges(*pugall);
	// maintain: mps: maxized result (s==smax)
	//			open: expandable result (0<=s<=smax-1)
	for(const Edge& e : edges) {
		vector<MotifBuilder> t = _edge2_dp(open, e);
		for(auto& mp : t) {
			int n = mp.getnEdge();
			if(n == smax) {
				double pPos;
				double pNeg;
				if(mp.connected() && objectFunction(pPos, pNeg)) {
					mps.push_back(mp.toMotif());
				}
			} else {
				open.push_back(move(mp));
			}
		}
	}

	for(auto& mp : open) {
		if(mp.getnEdge() >= smin) {
			double pPos;
			double pNeg;
			if(mp.connected() && objectFunction(pPos, pNeg))
				mps.push_back(mp.toMotif());
		}
	}
*/	return mps;
}
