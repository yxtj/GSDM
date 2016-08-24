#include "stdafx.h"
#include "StrategyFuncFreq.h"
#include "Option.h"
#include "Network.h"

using namespace std;


std::vector<Motif> StrategyFuncFreq::master(Network& net)
{
	cout << "Phase 1 (meta-data prepare)" << endl;
	slist supPos, supNeg;
	for(auto& s : *pgp)
		supPos.push(&s);
	for(auto& s : *pgn)
		supNeg.push(&s);
	vector<Edge> edges = getEdges();

	cout << "Phase 2 (calculate)" << endl;
	Motif dummy;
	TopKHolder<Motif, double> holder(k);
	chrono::system_clock::time_point _time = chrono::system_clock::now();
	_enum1_dis1(0, dummy, supPos, supNeg, holder, edges, net);
	cout << "  sending finish signal." << endl;
	for(int i = 0; i < net.getSize(); ++i) {
		if(i == net.getRank())
			continue;
		net.sendEnd(i);
	}
	auto _time_ms = chrono::duration_cast<chrono::milliseconds>(
		chrono::system_clock::now() - _time).count();
	cout << "  # of result: " << holder.size() << ", last score: " << holder.lastScore()
		<< "\n  time: " << _time_ms << " ms" << endl;

	cout << "Phase 3 (output)" << endl;
	return holder.getResultMove();
}

void outputFoundMotif(ostream& os, const Motif& m);

int StrategyFuncFreq::slaver(Network& net)
{
	int count = 0;
	Motif m;
	int source;
//	int type;
	while(net.readMotif(m, source)) {
/*		if(type == Network::TAG_COUNT_PAIR) {
			pair<int, int> c = countMotif(m);
			net.sendCount(source, c);
		} else if(type == Network::TAG_COUNT_POS) {
			int c = countMotif(m, *pgp);
			net.sendCountPos(source, c);
		} else if(type == Network::TAG_COUNT_NEG) {
			int c = countMotif(m, *pgn);
			net.sendCountNeg(source, c);
		}*/
//		cout << "counting motif:\t\t\t" << m << endl;
//		this_thread::sleep_for(chrono::duration<int>(1));
		pair<int, int> c = countMotif(m);
		net.sendCount(source, c);
		++count;
//		cout << "waiting" << endl;
	}
	return count;
}

std::pair<int, int> StrategyFuncFreq::master_gather_count(Network& net, const Motif & m)
{
//	cout << "gather counts for motif:\t" << m << endl;
	for(int i = 0; i < net.getSize(); ++i) {
		if(i == net.getRank())
			continue;
		net.sendMotif(i, m);
	}
//	cout << "send" << endl;
	int nPos = 0, nNeg = 0;
	vector<bool> flag(net.getSize(), false);
	int nLeft = net.getSize() - 1;
	flag[net.getRank()] = true;
	pair<int, int> t;
	int s;
	while(nLeft != 0 && net.readCount(t, s)) {
		if(flag[s] == false) {
			nPos += t.first;
			nNeg += t.second;
			flag[s] = true;
			nLeft--;
		} else {
			cerr << "error in reading count from " << s << endl;
		}
	}
//	cout << "received" << endl;
	return make_pair(nPos, nNeg);
}

int StrategyFuncFreq::master_gather_count_pos(Network & net, const Motif & m)
{
	// TODO: not valid
	for(int i = 0; i < net.getSize(); ++i) {
		if(i == net.getRank())
			continue;
		net.sendMotif(i, m);
	}
	int nPos = 0;
	for(int i = 0; i < net.getSize(); ++i) {
		if(i == net.getRank())
			continue;
		pair<int, int> t;
		net.readCount(t, i);
		nPos += t.first;
	}
	return nPos;
}

void StrategyFuncFreq::_enum1_dis1(const unsigned p, Motif & curr, slist & supPos, slist & supNeg, 
	TopKHolder<Motif, double>& res, const std::vector<Edge>& edges, Network& net)
{
	if(p >= edges.size() || curr.size() == smax) {
		if(curr.getnEdge() >= smin && curr.connected()) {
			int nPos, nNeg;
			tie(nPos, nNeg) = master_gather_count(net, curr);
			auto t = countMotif(curr);
			//nPos += supPos.size();
			//nNeg += supNeg.size();
			nPos += t.first;
			nNeg += t.second;
			if(nPos + nNeg < nMinSup)
				return;
			double s = objectFunction(static_cast<double>(nPos) / pgp->size(),
				static_cast<double>(nNeg) / pgn->size());
			++numMotifExplored;
			res.update(move(curr), s);
		}
		return;
	}
	//double lowBound = objectFunction(static_cast<double>(supPos.size()) / pgp->size(), 0.0);
	//if(!res.updatable(lowBound)) {
	//	return;
	//}

	_enum1_dis1(p + 1, curr, supPos, supNeg, res, edges, net);

	vector<const subject_t*> rmvPos, rmvNeg;
	removeSupport(supPos, rmvPos, edges[p]);
	removeSupport(supNeg, rmvNeg, edges[p]);
	curr.addEdge(edges[p].s, edges[p].d);
	_enum1_dis1(p + 1, curr, supPos, supNeg, res, edges, net);
	curr.removeEdge(edges[p].s, edges[p].d);
	for(auto s : rmvPos)
		supPos.push(s);
	for(auto s : rmvNeg)
		supNeg.push(s);
}
