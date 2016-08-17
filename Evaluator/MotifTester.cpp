#include "stdafx.h"
#include "MotifTester.h"

using namespace std;

const std::string MotifTester::name("motifTester");
const std::string MotifTester::usage("the method to test the label of a subject using a motif\n"
	"  freq <min portion>: the motif shows up among at lest <min portion> percent of the snaphots of a subject\n"
	"  prob <min support>: snapshots are merged into an uncertained graph. The motif shows up more likely than a minimum probability");

MotifTester::MotifTester(const std::vector<Graph>& gs)
	: gs(gs)
{
}

bool MotifTester::parse(const std::vector<std::string>& param)
{
	if(param.empty())
		return false;
	const string& method = param[0];
	bool res = true;
	if(method == "freq") {
		threshold = stod(param[1]);
		nMin = static_cast<int>(ceil(gs.size()*threshold));
		fun = &MotifTester::_testFreq;
	} else if(method == "prob") {
		threshold = stod(param[1]);
		gp = GraphProb(gs);
		fun = &MotifTester::_testProb;
	} else {
		cerr << "do not support given motif testing method: " << method << endl;
		res = false;
	}
	return res;
}

bool MotifTester::test(const Motif & m) const
{
	return (this->*fun)(m);
}

bool MotifTester::_testFreq(const Motif & m) const
{
	int cnt = 0;
	for(auto& g : gs) {
		if(g.testMotif(m)) {
			++cnt;
			if(cnt >= nMin)
				break;
		}
	}
	return cnt >= nMin;
}

bool MotifTester::_testProb(const Motif & m) const
{
	return gp.probOfMotif(m) >= threshold;
	//double p = 1.0;
	//for(auto& e : m.edges) {
	//	p *= gp.matrix[e.s][e.d];
	//	if(p < threshold)
	//		break;
	//}
	//return p >= threshold;
}

