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
		supPos.push_front(&s);
	for(auto& s : *pgn)
		supNeg.push_front(&s);
	vector<Edge> edges = master_gather_edges(net);
	cout << "  # of edges: " << edges.size() << endl;

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

int StrategyFuncFreq::slave(Network& net)
{
	cout << "slave on rank " << net.getRank() << endl;
	slave_edge_counting(net);
	int countMotif = slave_motif_counting(net);
	return countMotif;
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

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& param) {
	for(auto& p : param)
		os << p << " ";
	return os;
}

std::vector<Edge> StrategyFuncFreq::master_gather_edges(Network& net)
{
	vector<vector<int>> cnt;
	cnt.reserve(nNode);
	// local count
//	ofstream fout("edge-count-"+to_string(net.getSize())+"-"+to_string(net.getRank())+".txt");
	for(int i = 0; i < nNode; ++i) {
		vector<int> temp;
		temp.reserve(nNode - i - 1);
		for(int j = i + 1; j < nNode; ++j) {
			auto p = countEdge(i, j);
			temp.push_back(p.first + p.second);
		}
//		fout << temp << "\n";
		cnt.push_back(move(temp));
	}
//	fout.close();
	// gather remote count
	cout << "gather edges:" << endl;
	int size = net.getSize();
	vector<int> recvCnt(size);
	int nLeft = size - 1;
	while(nLeft != 0) {
		vector<int> ci;
		int source;
		net.readVecInt(ci, source);
//		cout << "  receive from " << source << " with length " << ci.size() << "\n" << ci << endl;
		int row = nNode - 1 - ci.size();
		for(size_t j = 0; j < ci.size(); ++j)
			cnt[row][j] += ci[j];
		if(++recvCnt[source] == nNode - 1)
			--nLeft;
	}
	// generate valid edges
	vector<Edge> res;
	int _nMinSup = static_cast<int>(nMinSup);
//	fout.open("edge-count-" + to_string(net.getSize()) + ".txt");
	for(int i = 0; i < nNode; ++i) {
		for(int j = 0; j < nNode - 1 - i; ++j) {
			if(cnt[i][j] >= _nMinSup)
				res.emplace_back(i, j + i + 1);
		}
//		fout << cnt[i] << "\n";
	}
	return res;
}

void StrategyFuncFreq::slave_edge_counting(Network& net)
{
	// last row has zero entry, do not send
//	ofstream fout("edge-count-" + to_string(net.getSize()) + "-" + to_string(net.getRank()) + ".txt");
	for(int i = 0; i < nNode - 1; ++i) {
		vector<int> cnt;
		cnt.reserve(nNode - i - 1);
		for(int j = i + 1; j < nNode; ++j) {
			auto p = countEdge(i, j);
			cnt.push_back(p.first + p.second);
		}
//		cout << "  send from " << net.getRank() << " with length " << cnt.size() << "\n" << cnt << endl;
		net.sendVecInt(0, cnt);
//		fout << cnt << "\n";
		//this_thread::sleep_for(chrono::seconds(1));
	}
}

int StrategyFuncFreq::slave_motif_counting(Network& net) {
	int count = 0;
	Motif m;
	int source;
	while(net.readMotif(m, source)) {
//		cout << "counting motif:\t\t\t" << m << endl;
//		this_thread::sleep_for(chrono::duration<int>(1));
		pair<int, int> c = countMotif(m);
		net.sendCount(source, c);
		++count;
//		cout << "waiting" << endl;
	}
	return count;
}

void StrategyFuncFreq::_enum1_dis1(const unsigned p, Motif & curr, slist & supPos, slist & supNeg, 
	TopKHolder<Motif, double>& res, const std::vector<Edge>& edges, Network& net)
{
	if(p >= edges.size() || curr.size() == smax) {
		if(curr.getnEdge() >= smin && curr.connected()) {
			int nPos, nNeg;
			tie(nPos, nNeg) = master_gather_count(net, curr);
			auto t = countMotif(curr);
			nPos += t.first;
			nNeg += t.second;
			//nPos += supPos.size();
			//nNeg += supNeg.size();
			if(nPos < nMinSup)
			//if(nPos + nNeg < nMinSup)
				return;
			double s = (this->*objFun)(static_cast<double>(nPos) / nSubPosGlobal,
				static_cast<double>(nNeg) / nSubNegGlobal);
			++numMotifExplored;
			res.update(move(curr), s);
		}
		return;
	}
	// pre-prune based on the # of supporting graph of a sub-motif
	{
		//auto clocal = countMotif(curr);
		//auto cremote = master_gather_count(net, curr);
		//double lowBound = objectFunction(static_cast<double>(clocal.first + cremote.first) / pgp->size(), 0.0);
		//if(!res.updatable(lowBound)) {
		//	return;
		//}
	}

	_enum1_dis1(p + 1, curr, supPos, supNeg, res, edges, net);

	vector<const subject_t*> rmvPos, rmvNeg;
	removeSupport(supPos, rmvPos, edges[p]);
	removeSupport(supNeg, rmvNeg, edges[p]);
	curr.addEdge(edges[p].s, edges[p].d);
	_enum1_dis1(p + 1, curr, supPos, supNeg, res, edges, net);
	curr.removeEdge(edges[p].s, edges[p].d);
	for(auto s : rmvPos)
		supPos.push_front(s);
	for(auto s : rmvNeg)
		supNeg.push_front(s);
}
