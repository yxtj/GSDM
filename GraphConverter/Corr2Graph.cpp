#include "stdafx.h"
#include "Corr2Graph.h"


Corr2Graph::Corr2Graph(const double th)
	:thre(th)
{
}

graph_t Corr2Graph::getGraph(const corr_t& corr)
{
	size_t n = corr.size();
	graph_t res(n);
	for(size_t i = 0; i < n; ++i) {
		for(size_t j = 0; j < n; ++j) {
			if(corr[i][j] >= thre)
				res[i].push_back(j);
		}
	}
	return res;
}

