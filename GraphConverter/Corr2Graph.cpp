#include "stdafx.h"
#include "Corr2Graph.h"

using namespace std;

Corr2Graph::Corr2Graph(const std::vector<std::string>& param)
{
	parseParam(param);
}

void Corr2Graph::parseParam(const std::vector<std::string>& param)
{
	if(param.empty()) {
		throw invalid_argument("no parameter for corr2graph.");
	}
	if(param[0] == "lt") {
		double th = parseNumber(param[1]);
		pred = [=](const double v) {return v < th; };
	} else if(param[0] == "le") {
		double th = parseNumber(param[1]);
		pred = [=](const double v) {return v <= th; };
	} else if(param[0] == "gt") {
		double th = parseNumber(param[1]);
		pred = [=](const double v) {return v > th; };
	} else if(param[0] == "ge") {
		double th = parseNumber(param[1]);
		pred = [=](const double v) {return v >= th; };
	} else if(param[0] == "between") {
		double thLow = parseNumber(param[1]);
		double thUp = parseNumber(param[2]);
		pred = [=](const double v) {return thLow <= v && v < thUp; };
	} else if(param[0] == "outside") {
		double thLow = parseNumber(param[1]);
		double thUp = parseNumber(param[2]);
		pred = [=](const double v) {return v < thLow || thUp <= v; };
	} else {
		throw invalid_argument("unknown parameter for corr2graph: " + param[0]);
	}
}

graph_t Corr2Graph::getGraphVec(const corr_t& corr)
{
	size_t n = corr.size();
	graph_t res(n);
	for(size_t i = 0; i < n; ++i) {
		for(size_t j = 0; j < n; ++j) {
			if(pred(corr[i][j]))
				res[i].push_back(j);
		}
	}
	return res;
}

Graph Corr2Graph::getGraph(const corr_t& corr)
{
	size_t n = corr.size();
	Graph res(n);
	for(size_t i = 0; i < n; ++i) {
		for(size_t j = 0; j < n; ++j) {
			res.matrix[i][j] = pred(corr[i][j]);
		}
	}
	return res;
}


double Corr2Graph::parseNumber(const std::string & v)
{
	if(v[0] == 'n')
		return -stod(v.substr(1));
	return stod(v);
}

