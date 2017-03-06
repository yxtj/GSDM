#include "stdafx.h"
#include "SDSignature.h"
#include <deque>

using namespace std;

SDSignature::SDSignature(const Graph & g)
{
	sd = calA2AShortestDistance(g);
}

SDSignature::SDSignature(const MotifBuilder & m, const int nNode)
	: sd(nNode, std::vector<int>(nNode, 2 * nNode))
{
	//init direct edges
	for(auto& e : m.edges) {
		sd[e.s][e.d] = sd[e.d][e.s] = 1;
	}
	//prepare nodes
	vector<int> nodes;
	int n = m.getnNode();
	nodes.reserve(n);
	for(auto& p : m.nodes) {
		if(p.second > 0)
			nodes.push_back(p.first);
	}
	//for(int i = 0; i < nNode; ++i) {
	//	if(m.containNode(i))
	//		nodes.push_back(i);
	//}
	//update
	for(int k = 0; k < n; ++k) {
		for(int i : nodes) {
			for(int j : nodes) {
				if(sd[i][k] + sd[k][j] < sd[i][j])
					sd[i][j] = sd[i][k] + sd[k][j];
			}
		}
	}
}

void SDSignature::update(const int s, const int d)
{
	if(sd[s][d] <= 1)
		return;
	int n = sd.size();
	// update using SPFA
	sd[s][d] = sd[d][s] = 1;
	deque<pair<int, int>> buf;
	buf.emplace_back(s, d);
	while(!buf.empty()) {
		int f, l;
		tie(f, l) = buf.front();
		buf.pop_front();
		int temp = sd[f][l];
		for(int i = 0; i < n; ++i) {
			if(sd[f][i] > temp + sd[l][i]) {
				sd[f][i] = sd[i][f] = temp + sd[l][i];
				buf.emplace_back(f, i);
			}
			if(sd[l][i] > temp + sd[f][i]) {
				sd[l][i] = sd[i][l] = temp + sd[f][i];
				buf.emplace_back(l, i);
			}
		}
	}
}

std::vector<std::vector<int>> SDSignature::calA2AShortestDistance(const Graph & g)
{
	int n = g.nNode;
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
