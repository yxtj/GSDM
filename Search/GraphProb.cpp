#include "stdafx.h"
#include "GraphProb.h"

using namespace std;

GraphProb::GraphProb(const int n)
	: nNode(n), nEdge(0), nSample(0), matrix(nNode, vector<double>(nNode, 0.0))
{
}

GraphProb::GraphProb(const std::vector<Graph>& gs)
{
	mergeWith(gs);
}

void GraphProb::mergeWith(const std::vector<Graph>& gs)
{
	nSample = gs.size();
	if(nSample == 0) {
		nNode = 0;
		nEdge = 0;
		return;
	}
	if(nNode != gs.front().nNode) {
		nNode = gs.front().nNode;
		initMatrix();
	}
	// accumulate
	for(const Graph& g : gs) {
		for(int i = 0; i < nNode; ++i) {
			const vector<bool>& line = g.matrix[i];
			for(int j = 0; j < nNode; ++j) {
				if(line[j])
					matrix[i][j] += 1;
			}
		}
	}
	// normalize
	nEdge = 0;
	for(auto& line : matrix) {
		for(auto& v : line)
			if(v != 0.0) {
				v /= nSample;
				++nEdge;
			}
	}
}

bool GraphProb::merge(const GraphProb & g)
{
	if(g.nNode != nNode)
		return false;
	size_t n = nSample + g.nSample;
	for(int i = 0; i < nNode; ++i) {
		for(int j = 0; j < nNode; ++j) {
			matrix[i][j] = (matrix[i][j] * nSample + g.matrix[i][j] * g.nSample) / n;
		}
	}
	return true;
}

void GraphProb::startAccum(const int nNode)
{
	if(this->nNode != nNode) {
		this->nNode = nNode;
		initMatrix();
	}
	nEdge = 0;
	nSample = 0;
}

bool GraphProb::iterAccum(const Graph & g)
{
	if(nNode != g.nNode)
		return false;
	for(int i = 0; i < nNode; ++i) {
		const vector<bool>& line = g.matrix[i];
		for(int j = 0; j < nNode; ++j) {
			if(line[j])
				matrix[i][j] += 1.0;
		}
	}
	++nSample;
	return true;
}

void GraphProb::finishAccum()
{
	int cnt = 0;
	for(auto& line : matrix) {
		for(auto& v : line)
			if(v != 0.0) {
				v /= nSample;
				++cnt;
			}
	}
	nEdge = cnt;
}

void GraphProb::initMatrix()
{
	matrix.clear();
	matrix.resize(nNode, vector<double>(nNode, 0.0));
}
